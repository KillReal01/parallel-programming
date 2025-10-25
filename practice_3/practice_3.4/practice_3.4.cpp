/*
    Course: Parallel Programming
    Assignment: 3.4
    Completed by: Bereza Kirill
*/


#include <iostream>
#include <thread>
#include <future>
#include <random>
#include <cstddef>
#include <vector>
#include <numeric>
#include <algorithm>


void createVector(std::promise<std::vector<int>> p, size_t size)
{
    std::vector<int> vector;
	vector.reserve(size);

    std::random_device rd;         
    std::mt19937 gen(rd());       
    std::uniform_int_distribution<int> dist(0, 100);

    for (std::size_t i = 0; i < size; ++i)
        vector.push_back(dist(gen));

    p.set_value(vector);
}

void sumValues(std::promise<int> p, std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int sum = std::accumulate(vector.begin(), vector.end(), 0);
    p.set_value(sum);
}

void minValue(std::promise<int> p, std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int min = *std::min_element(vector.begin(), vector.end());
    p.set_value(min);
}

void maxValue(std::promise<int> p, std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int max = *std::max_element(vector.begin(), vector.end());
    p.set_value(max);
}

int main()
{
    std::promise<std::vector<int>> pr_create_vector;
    std::promise<int> pr_sum;
    std::promise<int> pr_min;
    std::promise<int> pr_max;

    auto shared_future = pr_create_vector.get_future().share();
    std::future<int> f_sum = pr_sum.get_future();
    std::future<int> f_min = pr_min.get_future();
    std::future<int> f_max = pr_max.get_future();

    size_t vector_size = 100'000'000;

    std::vector<std::jthread> threads;
    threads.emplace_back(&createVector, std::move(pr_create_vector), vector_size);
    threads.emplace_back(&sumValues, std::move(pr_sum), shared_future);
    threads.emplace_back(&minValue, std::move(pr_min), shared_future);
    threads.emplace_back(&maxValue, std::move(pr_max), shared_future);

    std::cout << "Sum: " << f_sum.get() << std::endl;
    std::cout << "Min: " << f_min.get() << std::endl;
    std::cout << "Max: " << f_max.get() << std::endl;

	return 0;
}

