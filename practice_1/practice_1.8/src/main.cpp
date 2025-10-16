#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>


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

void reduce(std::stop_source ss, int& limit, int timeout_ms, int value)
{
    auto st = ss.get_token();
    int local_sum = 0;

    std::stop_callback sc {st, [&local_sum, timeout_ms, value](){
        std::cout << "Id: " << std::this_thread::get_id() << ";\tsum: " << local_sum << ";\tvalue: " << value << ";\ttimeout: " << timeout_ms << "\n";
    }};

    while(!st.stop_requested())
    {
        {
            std::lock_guard<std::mutex> lock(mtx); 
            if (limit - value < 0)
                ss.request_stop();
            else
            {
                limit -= value;
                local_sum += value;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    }
}

int main()
{
    std::srand(std::time(0));
    int limit = 10'000;
  
    uint32_t maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::jthread> threads;
    threads.reserve(maxThreads);

    std::cout << "Start reducing, maxThreads: " << maxThreads << std::endl;
    auto start = std::chrono::steady_clock::now();
    for (auto i = 0; i < maxThreads; i++)
    {
        auto delay = rand() % 20 + 1;
        auto value = rand() % 10 + 1;
        threads.emplace_back(&reduce, std::stop_source{}, std::ref(limit), delay, value);
    }
    
    for (auto& th : threads)
        th.join();

    auto end = std::chrono::steady_clock::now();
    std::cout << "Empty...\n";
    std::cout << "Handle duration (steady): " << formatDuration(end - start) << std::endl;

    std::cin.get();
    return 0;
}
