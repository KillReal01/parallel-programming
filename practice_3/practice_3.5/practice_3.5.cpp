/*
    Course: Parallel Programming
    Assignment: 3.5
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <stop_token>
#include <functional>
#include <numeric>
#include <iomanip>
#include <sstream>
#include <syncstream>


class ThreadPool
{
public:
    using task_type = std::packaged_task<int64_t()>;

    explicit ThreadPool(size_t worker_number = std::max(1u, std::thread::hardware_concurrency()))
    {
        _workers.reserve(worker_number);
        for (size_t i = 0; i < worker_number; ++i)
            _workers.emplace_back(&ThreadPool::thread_loop, this, _ss.get_token());
    }

    ~ThreadPool()
    {
        stop();
        for (auto& th : _workers)
            if (th.joinable())
                th.join();
    }

    std::future<int64_t> append_task(task_type task)
    {
        std::future<int64_t> future = task.get_future();
        {
            std::lock_guard lock(_mtx);
            _tasks.push(std::move(task));
        }
        _cv.notify_one();
        return future;
    }

    void stop()
    {
        _ss.request_stop();
        _cv.notify_all();
    }

private:
    void thread_loop(std::stop_token st)
    {
        std::osyncstream(std::cout) << "[ThreadPool] Id: " << std::this_thread::get_id() << "\tstarted\n";

        thread_local int number_tasks = 0;
        while (true)
        {
            task_type task;
            {
                std::unique_lock lock(_mtx);
                _cv.wait(lock, [this, &st]() { return !_tasks.empty() || st.stop_requested(); });

                if (_tasks.empty() && st.stop_requested())
                {
                    std::osyncstream(std::cout) << "[ThreadPool] Id: " << std::this_thread::get_id() << "\tfinished " << number_tasks << " tasks\n";
                    return;
                }

                task = std::move(_tasks.front());
                _tasks.pop();
            }
            task();
            number_tasks++;
        }
    }

    std::vector<std::thread> _workers;
    std::queue<task_type> _tasks;
    std::stop_source _ss;
    std::mutex _mtx;
    std::condition_variable _cv;
};

std::string formatDuration(const std::chrono::nanoseconds& time)
{
    using namespace std::chrono;

    hours h = duration_cast<hours>(time);
    minutes m = duration_cast<minutes>(time % hours(1));
    seconds s = duration_cast<seconds>(time % minutes(1));
    milliseconds ms = duration_cast<milliseconds>(time % seconds(1));
    microseconds mks = duration_cast<microseconds>(time % milliseconds(1));
    nanoseconds ns = duration_cast<nanoseconds>(time % microseconds(1));

    std::stringstream ss;
    ss << std::setfill('0')
        << std::setw(2) << h.count() << "h:"
        << std::setw(2) << m.count() << "m:"
        << std::setw(2) << s.count() << "s."
        << std::setw(3) << ms.count() << "ms."
        << std::setw(3) << mks.count() << "mks."
        << std::setw(3) << ns.count() << "ns";

    return ss.str();
}

int64_t sumArray(size_t size)
{
    std::osyncstream(std::cout) << "Id: " << std::this_thread::get_id() << " start sumArray\n";

    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    int64_t sum = 0;
    for (auto v : data)
        sum += v;

    std::osyncstream(std::cout) << "Id: " << std::this_thread::get_id() << " finished sumArray\n";
    return sum;
}

int64_t matrixSum(size_t N)
{
    std::osyncstream(std::cout) << "Id: " << std::this_thread::get_id() << " start matrixSum\n";

    std::vector<std::vector<int>> A(N, std::vector<int>(N, 1));
    std::vector<std::vector<int>> B(N, std::vector<int>(N, 2));
    std::vector<std::vector<int>> C(N, std::vector<int>(N, 0));
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            for (size_t k = 0; k < N; ++k)
                C[i][j] += A[i][k] * B[k][j];
    int64_t sum = 0;
    for (const auto& row : C)
        for (auto v : row)
            sum += v;

    std::osyncstream(std::cout) << "Id: " << std::this_thread::get_id() << " finished matrixSum\n";
    return sum;
}

int64_t fibNumber(int N)
{
    if (N <= 2) return 1;
    return fibNumber(N - 1) + fibNumber(N - 2);
}

void startThreadPool(int task_number, const std::vector<std::function<int64_t()>>& pool_tasks)
{
    std::vector<std::future<int64_t>> futures;
    futures.reserve(task_number);

    auto start = std::chrono::steady_clock::now();
    {
        ThreadPool pool;
        for (int i = 0; i < task_number; ++i)
        {
            std::packaged_task<int64_t()> task(pool_tasks[i % pool_tasks.size()]);
            futures.push_back(pool.append_task(std::move(task)));
        }
        for (auto& f : futures)
            f.get();
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "ThreadPool time: " << formatDuration(end - start) << std::endl;
}

void startParallel(int task_number, const std::vector<std::function<int64_t()>>& pool_tasks)
{
    std::vector<std::future<int64_t>> futures;
    futures.reserve(task_number);
    std::vector<std::thread> threads;
    threads.reserve(task_number);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < task_number; ++i)
    {
        std::packaged_task<int64_t()> task(pool_tasks[i % pool_tasks.size()]);
        futures.push_back(task.get_future());
        threads.emplace_back(std::move(task));
    }
    for (auto& t : threads)
        t.join();
    for (auto& f : futures)
        f.get();

    auto end = std::chrono::steady_clock::now();
    std::cout << "Individual threads time: " << formatDuration(end - start) << std::endl;
}

int main()
{
    int task_number = 200;
    std::vector<std::function<int64_t()>> pool_tasks
    {
        []() { return sumArray(10'000'000); },
        []() { return matrixSum(1000); },
        []() { return fibNumber(40); }
    };

    // ThreadPool
    startThreadPool(task_number, pool_tasks);

    // Individual threads
    startParallel(task_number, pool_tasks);

    return 0;
}

// С небольшим отрывом победу одержал ThreadPool

//[ThreadPool] Id: 7300   started
//[ThreadPool] Id : 17972  started
//[ThreadPool] Id : 21024  started
//[ThreadPool] Id : 11468  started
//[ThreadPool] Id : 1644   started
//[ThreadPool] Id : 21084  started
//[ThreadPool] Id : 18332  started
//[ThreadPool] Id : 21128  started
//[ThreadPool] Id : 3784   started
//[ThreadPool] Id : 21024  finished 18 tasks
//[ThreadPool] Id : 21044  finished 15 tasks
//[ThreadPool] Id : 18332  finished 16 tasks
//[ThreadPool] Id : 21128  finished 15 tasks
//[ThreadPool] Id : 19212  finished 15 tasks
//[ThreadPool] Id : 3784   finished 21 tasks
//[ThreadPool] Id : 17972  finished 13 tasks
//[ThreadPool] Id : 21084  finished 21 tasks
//[ThreadPool] Id : 7300   finished 21 tasks
//[ThreadPool] Id : 11468  finished 11 tasks
//[ThreadPool] Id : 1644   finished 17 tasks
//[ThreadPool] Id : 15112  finished 17 tasks
//ThreadPool time : 00h : 00m : 32s.884ms.092mks.900ns
//Individual threads time : 00h : 00m : 35s.313ms.520mks.400ns
