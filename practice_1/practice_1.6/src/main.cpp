#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <stop_token>
#include <mutex>

std::mutex mtx;

std::vector<int> generateVector(size_t size)
{
    std::vector<int> v;
    v.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        auto val = std::rand() % 100'000'000;
        v.push_back(val);
    }
    return v;
}


template<typename InIt, typename T>
void findBlock(std::stop_source ss, InIt first, InIt last, const T value)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Id: " << std::this_thread::get_id() << " started" << std::endl;
    }

    auto st = ss.get_token();

    for (; first != last && !st.stop_requested(); ++first)
    {
        if (*first == value)
        {
            ss.request_stop();
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Id: " << std::this_thread::get_id() << " found result!\n";
            }
            return;
        }
    }
    if (st.stop_requested())
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Id: " << std::this_thread::get_id() << " stopped" << std::endl;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Id: " << std::this_thread::get_id() << " finished" << std::endl;
    }
}

int main()
{
    const int key = -1;
    const size_t vector_size = 100'000'000;

    std::cout << "Generating vector...\n";
    std::vector<int> vector = generateVector(vector_size);
    std::cout << "Vector is ready\n";
    // vector[0] = key;
    // vector[vector_size / 2 - 2] = key;
    vector[vector_size - 1] = key;

    uint32_t maxThreads = std::max(1u, std::thread::hardware_concurrency());
    const size_t partSize = vector.size() / maxThreads;

    std::stop_source ss;
    std::vector<std::thread> threads;
    threads.reserve(maxThreads);

    auto first = vector.begin();
    for (uint32_t i = 0; i < maxThreads; ++i)
    {
        auto last = (i + 1 == maxThreads) ? vector.end() : std::next(first, std::min(partSize, static_cast<size_t>(vector.end() - first)));
        threads.emplace_back(findBlock<std::vector<int>::iterator, int>, ss, first, last, key);
        first = last;
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (auto& t : threads)
        t.join();

    if (ss.get_token().stop_requested())
        std::cout << "Value " << key << " found\n";
    else
        std::cout << "Value " << key << " not found\n";

    std::cin.get();
    return 0;
}
