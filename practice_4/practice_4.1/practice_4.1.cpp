/*
    Course: Parallel Programming
    Assignment: 4.1
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <atomic>
#include <vector>
#include <thread>


void increment_value(std::atomic<int>& value, int delta)
{
    value.fetch_add(delta, std::memory_order_relaxed);
}

void increment_value_pointer(std::atomic<int>* value, int delta)
{
    value->fetch_add(delta, std::memory_order_relaxed);
}

void print(const std::vector<std::atomic<int>>& vec)
{
    for (const auto& value : vec)
        std::cout << value.load() << " ";
    std::cout << std::endl;
}

void task_1()
{
    const int vector_size = 10;
    std::vector<std::atomic<int>> vec(vector_size);
    for (auto i = 0; i < vec.size(); ++i)
        vec[i].store(i);

    std::cout << "[Task 1] Vector before: ";
    print(vec);

    std::vector<std::thread> workers;
    workers.reserve(vec.size());

    for (auto& value : vec)
        workers.emplace_back(increment_value, std::ref(value), 1);

    for (auto& th : workers)
        th.join();

    std::cout << "[Task 1] Vector after: ";
    print(vec);
}

void task_2()
{
    const int vector_size = 10;
    std::vector<std::atomic<int>> vec(vector_size);
    for (auto i = 0; i < vec.size(); ++i)
        vec[i].store(i);

    std::cout << "[Task 2] Vector before: ";
    print(vec);

    std::vector<std::thread> workers;
    workers.reserve(vec.size());

    for (auto& value : vec)
        workers.emplace_back(increment_value_pointer, &value, 1);

    for (auto& th : workers)
        th.join();

    std::cout << "[Task 2] Vector after: ";
    print(vec);
}

int main()
{
    // 1.a
    task_1();
    // 1.b
    task_2();
    return 0;
}
