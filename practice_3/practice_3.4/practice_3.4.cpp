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

    std::cout << "Start generate...\n";
    for (std::size_t i = 0; i < size; ++i)
        vector.push_back(dist(gen));

    std::cout << "Vector generated\n";
    p.set_value(vector);
}

int sumValues(std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int sum = std::accumulate(vector.begin(), vector.end(), 0);
    return sum;
}

int minValue(std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int min = *std::min_element(vector.begin(), vector.end());
    return min;
}

int maxValue(std::shared_future<std::vector<int>> future)
{
    auto vector = future.get();
    int max = *std::max_element(vector.begin(), vector.end());
    return max;
}

int main()
{
    std::promise<std::vector<int>> pr;
    auto shared_future = pr.get_future().share();

    size_t vector_size = 100'000'000;

    std::thread t(&createVector, std::move(pr), vector_size);

    std::future<int> f_sum = std::async(std::launch::async, [shared_future]() {
        return sumValues(shared_future);
        });

    std::future<int> f_min = std::async(std::launch::async, [shared_future]() {
        return minValue(shared_future);
        });

    std::future<int> f_max = std::async(std::launch::async, [shared_future]() {
        return maxValue(shared_future);
        });

    t.join();
    std::cout << "Sum: " << f_sum.get() << std::endl;
    std::cout << "Min: " << f_min.get() << std::endl;
    std::cout << "Max: " << f_max.get() << std::endl;

	return 0;
}

