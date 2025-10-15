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
    size_t partSize = vector_size / maxThreads;
    size_t remainder = vector_size % maxThreads;
    uint32_t totalThreads = maxThreads + (remainder ? 1 : 0);

    std::stop_source ss;
    std::vector<std::thread> threads;
    threads.reserve(totalThreads);

    auto first = vector.begin();
    for (uint32_t i = 0; i < totalThreads; ++i)
    {
        size_t currentSize = (i < maxThreads) ? partSize : remainder;
        if (currentSize == 0)
            break;

        auto last = std::next(first, currentSize);
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
