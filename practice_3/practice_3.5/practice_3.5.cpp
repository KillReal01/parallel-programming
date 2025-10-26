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
#include <syncstream>
#include <functional>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <cmath>


class ThreadPool
{
public:
    using task_type = std::packaged_task<int64_t()>;
    // using task_type = std::function<void()>;

    ThreadPool(size_t worker_number = std::max(1u, std::thread::hardware_concurrency()))
    {
        for (size_t i = 0; i < worker_number; i++)
            _workers.emplace_back(&ThreadPool::thread_loop, this, _ss.get_token());
    }

    ~ThreadPool()
    {
        this->stop();
        for (auto& th : _workers)
        {
            if (th.joinable())
                th.join();
        }
    }

    void append_task(task_type task)
    {
        {
            std::lock_guard lock(_mtx);
            _tasks.push(std::move(task));
        }
        _cv.notify_one();
    }

    void stop()
    {
        _ss.request_stop();
        _cv.notify_all();
    }

private:
    void thread_loop(std::stop_token st)
    {
        thread_local int task_count = 0;
        std::osyncstream os(std::cout);

        // os << "Id: " << std::this_thread::get_id() << " started" << std::endl;
        while (true)
        {
            task_type task;
            {
                std::unique_lock lock(_mtx);
                _cv.wait(lock, [this]() { return !_tasks.empty() || _ss.stop_requested(); });

                if (_tasks.empty() && _ss.stop_requested())
                    break;

                task = std::move(_tasks.front());
                _tasks.pop();
            }
            task();
            task_count++;
        }
        // os << "Id: " << std::this_thread::get_id() << " finished " << task_count << " tasks" << std::endl;
    }

private:
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

//void task1()
//{
//    constexpr size_t size = 100'000'000;
//    std::vector<int> data(size);
//    std::iota(data.begin(), data.end(), 1);
//    long long sum = 0;
//    for (auto v : data)
//        sum += v;
//}
//
//void task2()
//{
//    constexpr size_t N = 1000;
//    std::vector<std::vector<double>> A(N, std::vector<double>(N, 1.0));
//    std::vector<std::vector<double>> B(N, std::vector<double>(N, 2.0));
//    std::vector<std::vector<double>> C(N, std::vector<double>(N, 0.0));
//
//    for (size_t i = 0; i < N; ++i)
//        for (size_t j = 0; j < N; ++j)
//            for (size_t k = 0; k < N; ++k)
//                C[i][j] += A[i][k] * B[k][j];
//}
//
//unsigned long long fib(int n)
//{
//    if (n <= 2) return 1;
//    return fib(n - 1) + fib(n - 2);
//}
//
//void task3()
//{
//    volatile auto r = fib(35);
//}

int64_t sumArray()
{
    constexpr size_t size = 100'000'000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);

    int64_t sum = 0;
    for (auto v : data)
        sum += v;

    return sum;
}

int64_t matrixDeterminantSum()
{
    constexpr size_t N = 100;
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

    return sum;
}

int64_t fibNumber(int n = 35)
{
    if (n <= 2) return 1;
    return fibNumber(n - 1) + fibNumber(n - 2);
}

int main()
{
    int task_number = 100;
    std::vector<std::function<int64_t()>> pool_tasks{ sumArray, matrixDeterminantSum, fibNumber };

    std::vector<std::function<void()>> tasks;
    tasks.reserve(task_number);

    for (int i = 0; i < task_number; i++)
        tasks.emplace_back(pool_tasks[i % pool_tasks.size()]);

    // Thread pool
    auto start = std::chrono::steady_clock::now();
    {
        ThreadPool pool;
        for (const auto& task : tasks)
            pool.append_task(task);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "Thread pool time: " << formatDuration(end - start) << std::endl;

    // Individual threads
    start = std::chrono::steady_clock::now();
    {
        std::vector<std::thread> threads;
        for (const auto& task : tasks)
            threads.emplace_back(task);
        for (auto& t : threads)
            t.join();
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Individual threads time: " << formatDuration(end - start) << std::endl;

    return 0;
}

// На тяжелых операциях победу одержал ThreadPool
// Thread pool time : 00h : 00m : 21s.486ms.771mks.700ns
// Individual threads time : 00h : 00m : 25s.343ms.817mks.800ns
