/*
    Course: Parallel Programming
    Assignment: 4.3
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <barrier>
#include <climits>
#include <algorithm>


constexpr int window_size = 5;


int main()
{
    std::vector<std::vector<int>> vv
    {
        {1, -1, 2, 5, 0, -3, 7},
        std::vector<int>(20, 2),
        {-5, 3, 4, 33, 22}
    };

    auto it = std::ranges::max_element(vv, {}, [](const auto &v){ return v.size(); });
    size_t max_len = (it != vv.end()) ? it->size() : 0;

    size_t windows_count = max_len / window_size + ((max_len % window_size) ? 1 : 0);

    std::atomic<int> current_min{INT_MAX};

    std::barrier barrier(vv.size(), [&]() noexcept {
        std::cout << "Minimum of current window: " << current_min.load() << std::endl;
        current_min.store(INT_MAX, std::memory_order_relaxed);
    });

    auto worker = [&](const std::vector<int>& v) {
        for (size_t w = 0; w < windows_count; ++w)
        {
            size_t start = w * window_size;
            size_t end = std::min(start + window_size, v.size());
            if (start >= v.size())
            {
                barrier.arrive_and_wait();
                continue;
            }

            int window_min = *std::min_element(v.begin() + start, v.begin() + end);
            int prev = current_min.load(std::memory_order_relaxed);

            while (window_min < prev &&
                   !current_min.compare_exchange_weak(prev, window_min,
                                                      std::memory_order_release,
                                                      std::memory_order_relaxed));

            barrier.arrive_and_wait();
        }
    };

    std::vector<std::thread> threads;
    for (auto& vec : vv)
        threads.emplace_back(worker, std::ref(vec));

    for (auto& t : threads)
        t.join();

    return 0;
}
