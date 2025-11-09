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
#include <array>


// Условие не совсем ясно, понял как:
// Найти минимальный элемент на каждой позиции среди всех векторов и вывести массив этих минимумов.


int main()
{
    std::vector<std::vector<int>> vv
    {
        {1, -1, 2, 5, 0, -3, 7},
        std::vector<int>(20, 2),
        {-5, 3, 4, 33, 22}
    };

    const int count = 5;
    std::array<std::atomic<int>, count> min_values;

    for (int i = 0; i < count; ++i)
        min_values[i].store(vv.front()[i]);

    std::barrier barrier(vv.size(), [&]() noexcept {
        std::cout << "Minimums: ";
        for (int i = 0; i < count; ++i)
            std::cout << min_values[i] << " ";
        std::cout << std::endl;
        });

    auto worker = [&](const std::vector<int>& v) {
        for (int i = 0; i < count && i < v.size(); ++i)
        {
            int current = min_values[i].load(std::memory_order_relaxed);
            while (v[i] < current && !min_values[i].compare_exchange_weak(current, v[i],
                                                                          std::memory_order_release,
                                                                          std::memory_order_relaxed));
        }
        barrier.arrive_and_wait();
    };

    std::vector<std::thread> threads;
    for (auto& vec : vv)
        threads.emplace_back(worker, std::ref(vec));

    for (auto& t : threads)
        t.join();

    return 0;
}
