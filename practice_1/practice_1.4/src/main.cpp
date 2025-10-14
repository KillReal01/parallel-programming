#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cxxopts.hpp>


std::vector<int> generateVector(size_t size)
{
    std::vector<int> vector;
    vector.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        vector.push_back(std::rand() % 100 - 50);
    }
    return vector;
}

void printVector(const std::vector<int>& vector)
{
    for (const auto& item : vector)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

std::string formatDuration(std::chrono::nanoseconds time)
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

template<typename InIt, typename OutIt, typename T>
void handleBlock(InIt first, InIt last, OutIt dest)
{
    std::transform(first, last, dest, [](T x){ return std::abs(x); });
}

int main(int argc, char** argv)
{
    int32_t maxThreads = 1;
    try
    {
        cxxopts::Options options(argv[0], "File processing application");
        options.add_options()
            ("maxThreads", "Maximum number of threads", cxxopts::value<int>()->default_value("1"))
            ("h,help", "Show help message");

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        maxThreads = result["maxThreads"].as<int>();
        if (maxThreads < 1)
        {
            std::cerr << "Error: maxThreads must be >= 1\n";
            return 1;
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }

    size_t vector_size = 100'000'000;
    std::cout << "Generating vector...\n";
    std::vector<int> vector = generateVector(vector_size);
    std::cout << "Vector is ready\n";

    std::vector<int> h_vector(vector.size());

    auto start = std::chrono::steady_clock::now();

    if (maxThreads <= 1)
    {
        handleBlock<std::vector<int>::iterator, std::vector<int>::iterator, int>(vector.begin(), vector.end(), h_vector.begin());
    }
    else
    {
        int32_t numThreads = maxThreads - 1;
        std::vector<std::thread> threads;
        threads.reserve(numThreads);

        auto partSize = vector.size() / maxThreads;
        auto first = vector.begin();
        auto dest = h_vector.begin();

        for (int32_t i = 0; i < numThreads; ++i)
        {
            auto last = first;
            std::advance(last, partSize);
            threads.emplace_back(&handleBlock<std::vector<int>::iterator, std::vector<int>::iterator, int>, first, last, dest);

            // offset
            first = last;
            std::advance(dest, partSize);
        }

        // handle remainder
        handleBlock<std::vector<int>::iterator, std::vector<int>::iterator, int>(first, vector.end(), dest);

        for (auto& th : threads)
            th.join();
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Handle duration (steady): " << formatDuration(end - start) << std::endl;

    return 0;
}
