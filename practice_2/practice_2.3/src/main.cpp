/*
    Course: Parallel Programming
    Assignment: 2.3
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

// 1. multi-threading
class ThreadSafeSingletonMT
{
public:
    ~ThreadSafeSingletonMT() = default;

    ThreadSafeSingletonMT(const ThreadSafeSingletonMT&) = delete;
    ThreadSafeSingletonMT& operator=(const ThreadSafeSingletonMT&) = delete;

    static ThreadSafeSingletonMT& get()
    {
        static ThreadSafeSingletonMT singleton;
        return singleton;
    }

    void show_id() const { std::cout << "MT Singleton address: " << this << std::endl; }

private:
    ThreadSafeSingletonMT() = default;
};

// 2. thread local
class ThreadSafeSingletonTL
{
public:
    ~ThreadSafeSingletonTL() = default;

    ThreadSafeSingletonTL(const ThreadSafeSingletonTL&) = delete;
    ThreadSafeSingletonTL& operator=(const ThreadSafeSingletonTL&) = delete;

    static ThreadSafeSingletonTL& get()
    {
        thread_local ThreadSafeSingletonTL singleton;
        return singleton;
    }

    void show_id() const { std::cout << "TL Singleton address: " << this << std::endl; }

private:
    ThreadSafeSingletonTL() = default;
};


void testSingletons(int thread_num)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Thread " << thread_num << ":\n";
    ThreadSafeSingletonMT::get().show_id();
    ThreadSafeSingletonTL::get().show_id();
}

int main()
{
    const int num_threads = 3;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(&testSingletons, i + 1);

    for (auto& t : threads)
        t.join();

    std::cin.get();
    return 0;
}
