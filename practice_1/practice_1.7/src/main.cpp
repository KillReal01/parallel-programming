#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>


std::mutex mtx;

std::string formatDuration(const std::chrono::nanoseconds& time)
{
    using namespace std::chrono;

    hours h  = duration_cast<hours>(time);
    minutes m  = duration_cast<minutes>(time % hours(1));
    seconds s  = duration_cast<seconds>(time % minutes(1));
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

void fillVector(std::stop_source ss, std::vector<int>& vector, int& sum, int limit, int value)
{
    auto st = ss.get_token();
    while(!st.stop_requested())
    {   
        std::lock_guard<std::mutex> lock(mtx);
        vector.push_back(value);
        sum += value;

        if (sum > limit)
            ss.request_stop();    
    }
}

int main()
{
    int limit = 100'000'000;
    int sum = 0;
    std::vector<int> vector;

    uint32_t maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::jthread> threads;
    threads.reserve(maxThreads);

    std::stop_source ss;

    std::cout << "Filling vector...\n";
    auto start = std::chrono::steady_clock::now();
    for (auto i = 0; i < maxThreads; i++)
        threads.emplace_back(&fillVector, ss, std::ref(vector), std::ref(sum), limit, (i + 1));
    
    for (auto& th : threads)
        th.join();

    auto end = std::chrono::steady_clock::now();
    std::cout << "Vector is full\n";
    std::cout << "Handle duration (steady): " << formatDuration(end - start) << std::endl;

    std::cin.get();

    return 0;
}
