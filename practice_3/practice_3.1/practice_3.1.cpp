#include <random>
#include <iterator>
#include <type_traits>
#include <vector>
#include <deque>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <numeric>
#include <future>


template<typename Container, typename T = typename Container::value_type>
void fill_random(Container& c, T min = T(0), T max = T(100))
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    if constexpr (std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> dist(min, max);
        for (auto& v : c)
            v = dist(gen);
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        std::uniform_real_distribution<T> dist(min, max);
        for (auto& v : c)
            v = dist(gen);
    }
}

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

template<typename Container>
void sequantial(const Container& c)
{
    auto start = std::chrono::steady_clock::now();
    auto sum = std::accumulate(c.begin(), c.end(), 0);
    auto finish = std::chrono::steady_clock::now();

    float avg = static_cast<float>(sum) / c.size();

    std::cout << "[sequantial] Sum: " << sum << '\n'
              << "[sequantial] Avg: " << avg << '\n'
              << "[sequantial] Duration: " << formatDuration(finish - start) << '\n';
}

template<typename Container, typename T = typename Container::value_type>
void parallel(const Container& c)
{
    const auto threadNum = std::max(1u, std::thread::hardware_concurrency());
    std::cout << "[parallel] Number threads: " << threadNum << '\n';

    const size_t part_size = c.size() / threadNum;

    std::vector<std::future<T>> futures;
    futures.reserve(threadNum);

    auto start = std::chrono::steady_clock::now();

    auto begin = c.begin();
    for (auto i = 0; i < threadNum; ++i)
    {
        auto end = (i == threadNum - 1) ? c.end() : std::next(begin, part_size);

        futures.emplace_back(std::async(std::launch::async, [begin, end]() {
            return std::accumulate(begin, end, T(0));
            }));

        begin = end;
    }

    T sum = 0;
    for (auto& f : futures)
        sum += f.get();

    auto finish = std::chrono::steady_clock::now();

    float avg = static_cast<float>(sum) / c.size();

    std::cout << "[parallel] Sum: " << sum << '\n'
              << "[parallel] Avg: " << avg << '\n'
              << "[parallel] Duration: " << formatDuration(finish - start) << '\n';
}

int main()
{
    std::cout << "Generating values...\n";
    size_t vector_size = 100'000'000;

    std::vector<int> vi(vector_size);
    vi.reserve(vector_size);

    fill_random(vi, 1, 5);
    std::cout << "Generated\n";

    // Последовательная обработка
    sequantial(vi);

    // Параллельная обработка
    parallel(vi);

    return 0;
}
