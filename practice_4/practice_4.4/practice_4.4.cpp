/*
    Course: Parallel Programming
    Assignment: 4.4
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <atomic>
#include <shared_mutex>
#include <vector>
#include <thread>
#include <chrono>


class spinlock
{
public:
    spinlock() = default;
    ~spinlock() = default;

    void lock() noexcept
    {
        while (_flag.test_and_set(std::memory_order_acquire))
            _flag.wait(true, std::memory_order_relaxed);
    }

    void lock_shared() noexcept
    {
        this->lock(); // заглушка
    }

    void unlock() noexcept
    {
        _flag.clear(std::memory_order_release);
        _flag.notify_one();
    }

    void unlock_shared() noexcept
    {
        this->unlock(); // заглушка
    }

    bool try_lock() noexcept
    {
        return !_flag.test_and_set(std::memory_order_acquire);
    }

private:
    std::atomic_flag _flag;
};

template<typename T, typename Mutex>
class threadsafe_stack
{
    std::vector<T> _data;
    mutable Mutex _mtx;

public:

    threadsafe_stack() = default;
    ~threadsafe_stack() = default;

    threadsafe_stack(const threadsafe_stack& other)
    {
        std::shared_lock<Mutex> lock(other._mtx);
        _data = other._data;
    }

    threadsafe_stack& operator=(const threadsafe_stack& other)
    {
        if (this != &other)
        {
            std::scoped_lock lock(_mtx, other._mtx);
            _data = other._data;
        }
        return *this;
    }

    threadsafe_stack(threadsafe_stack&& other) noexcept
    {
        std::lock_guard<Mutex> lock(other._mtx);
        _data = std::move(other._data);
    }

    threadsafe_stack& operator=(threadsafe_stack&& other) noexcept
    {
        if (this != &other)
        {
            std::scoped_lock lock(_mtx, other._mtx);
            _data = std::move(other._data);
        }
        return *this;
    }

    bool empty() const
    {
        std::shared_lock<Mutex> lock(_mtx);
        return _data.empty();
    }

    void push(T value)
    {
        std::lock_guard<Mutex> lock(_mtx);
        _data.push_back(std::move(value));
    }

    void pop(T& value)
    {
        std::lock_guard<Mutex> lock(_mtx);
        if (_data.empty())
            throw std::runtime_error("stack is empty");
        value = _data.back();
        _data.pop_back();
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<Mutex> lock(_mtx);
        if (_data.empty())
            return nullptr;
        auto res = std::make_shared<T>(_data.back());
        _data.pop_back();
        return res;
    }

    T top() const
    {
        std::shared_lock<Mutex> lock(_mtx);
        if (_data.empty())
            throw std::runtime_error("stack is empty");
        return _data.back();
    }

    size_t size() const
    {
        std::shared_lock<Mutex> lock(_mtx);
        return _data.size();
    }
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

template<typename Stack>
void stress_test(Stack& stack, int num_threads, int ops_per_thread, const std::string& name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto worker = [&stack, ops_per_thread]() {
        for (int i = 0; i < ops_per_thread; ++i)
        {
            if (i % 2)
            {
                stack.push(12345);
            }
            else
            {
                try { stack.pop(); }
                catch (...) {}
            }
        }
        };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(worker);

    for (auto& t : threads)
        t.join();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "[" << name << "] Time taken : " << formatDuration(end - start) << std::endl;
}

int main()
{
    threadsafe_stack<int, std::shared_mutex> stack_shared_mtx;
    threadsafe_stack<int, spinlock> stack_spinlock;

    const auto num_threads = std::max(1u, std::thread::hardware_concurrency());
    const int ops_per_thread = 1'000'000;

    std::cout << "Testing std::shared_mutex...\n";
    stress_test(stack_shared_mtx, num_threads, ops_per_thread, "shared_mutex");

    std::cout << "\nTesting spinlock...\n";
    stress_test(stack_spinlock, num_threads, ops_per_thread, "spinlock");

    return 0;
}

// Наблюдаем солидный выигрыш!
//
//Testing std::shared_mutex...
//[shared_mutex] Time taken : 00h : 00m : 24s.341ms.166mks.400ns
//
//Testing spinlock...
//[spinlock] Time taken :     00h : 00m : 16s.607ms.609mks.000ns