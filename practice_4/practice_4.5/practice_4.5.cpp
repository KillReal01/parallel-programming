/*
    Course: Parallel Programming
    Assignment: 4.5
    Completed by: Bereza Kirill
*/
       

#include <iostream>
#include <atomic>
#include <mutex>
#include <shared_mutex>
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

    void unlock() noexcept
    {
        _flag.clear(std::memory_order_release);
        _flag.notify_one();
    }

    bool try_lock() noexcept
    {
        return !_flag.test_and_set(std::memory_order_acquire);
    }

private:
    std::atomic_flag _flag;
};


template<typename T>
class T_Mutex
{
public:
    explicit T_Mutex(T value)
        : m_value(std::move(value))
    {
        std::lock_guard<std::shared_mutex> lock(m_mtx);
        ++s_counter;
    }

    ~T_Mutex()
    {
        std::lock_guard<std::shared_mutex> lock(m_mtx);
        --s_counter;
    }

    static size_t get_count()
    {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return s_counter;
    }

private:
    inline static size_t s_counter{ 0 };
    inline static std::shared_mutex m_mtx;

    T m_value;
};

template<typename T>
class T_Spin
{
public:
    explicit T_Spin(T value)
        : m_value(std::move(value))
    {
        std::lock_guard<spinlock> lock(m_spin);
        ++s_counter;
    }

    ~T_Spin()
    {
        std::lock_guard<spinlock> lock(m_spin);
        --s_counter;
    }

    static size_t get_count()
    {
        std::lock_guard<spinlock> lock(m_spin);
        return s_counter;
    }

private:
    inline static size_t s_counter{ 0 };
    inline static spinlock m_spin;

    T m_value;
};

template<typename T>
class T_Atomic
{
public:
    explicit T_Atomic(T value)
        : m_value(value)
    { s_counter.fetch_add(1, std::memory_order_relaxed); }

    ~T_Atomic() { s_counter.fetch_sub(1, std::memory_order_relaxed); }
    static size_t get_count() { return s_counter.load(std::memory_order_relaxed); }

private:
    inline static std::atomic<size_t> s_counter{ 0 };
    T m_value;
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

template<typename T>
void worker(size_t n)
{
    for (size_t i = 0; i < n; ++i)
        T obj(5);
}

template<typename A>
void test_class(const std::string& name, uint32_t threads_count, size_t objs_per_thread)
{
    std::cout << "Testing " << name << "...\n";

    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threads_count; ++i)
        threads.emplace_back(worker<A>, objs_per_thread);

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Final count: " << A::get_count() << "\n";
    std::cout << "Time taken: " << formatDuration(end - start) << " ms\n\n";
}

int main()
{
    uint32_t threads_count = std::max(1u, std::thread::hardware_concurrency());
    size_t objs_per_thread = 1'000'000;

    test_class<T_Mutex<int>>("T_Mutex", threads_count, objs_per_thread);
    test_class<T_Spin<int>>("T_Spin", threads_count, objs_per_thread);
    test_class<T_Atomic<int>>("T_Atomic", threads_count, objs_per_thread);

    return 0;
}