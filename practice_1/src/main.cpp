#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <thread>
#include <chrono>
#include <iomanip>


namespace fs = std::filesystem;


std::vector<fs::path> getInputFilenames(const fs::path& path)
{
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file())
            files.push_back(entry.path());
    }
    return files;
}

bool processFile(const fs::path& inputFile, const fs::path& outputFile)
{
    std::ifstream in(inputFile);
    if (!in.is_open())
    {
        std::cerr << "Error opening input file: " << inputFile << "\n";
        return false;
    }

    std::ofstream out(outputFile);
    if (!out.is_open())
    {
        std::cerr << "Error opening output file: " << outputFile << "\n";
        return false;
    }

    std::string line;
    while (std::getline(in, line))
    {
        std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return std::toupper(c); });
        out << line << "\n";
    }

    return true;
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

void task_1()
{
    fs::path inputDir = "input_text";
    fs::path outputDir = "output_text";

    if (!fs::exists(outputDir))
        fs::create_directory(outputDir);

    auto files = getInputFilenames(inputDir);

    auto start = std::chrono::steady_clock::now();

    for (const auto& f : files)
    {
        fs::path outFile = outputDir / ("_" + f.filename().string());
        std::thread th(&processFile, f, outFile);
        th.join();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;

    std::cout << "Duration (steady): " << formatDuration(duration) << std::endl;
}

void task_2()
{
    fs::path inputDir = "input_text";
    fs::path outputDir = "output_text";

    if (!fs::exists(outputDir))
        fs::create_directory(outputDir);

    auto files = getInputFilenames(inputDir);

    std::vector<std::thread> threads;
    threads.reserve(files.size());

    auto start = std::chrono::steady_clock::now();

    for (const auto& f : files)
    {
        fs::path outFile = outputDir / ("_" + f.filename().string());
        threads.emplace_back(&processFile, f, outFile);
    }

    for (auto& th : threads)
        th.join();

    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;

    std::cout << "Duration (steady): " << formatDuration(duration) << std::endl;
}

void task_3()
{
    fs::path inputDir = "input_text";
    fs::path outputDir = "output_text";

    if (!fs::exists(outputDir))
        fs::create_directory(outputDir);

    auto files = getInputFilenames(inputDir);

    uint32_t maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> threads;
    threads.reserve(maxThreads);

    auto start = std::chrono::steady_clock::now();

    size_t idx = 0;
    while (idx < files.size())
    {
        threads.clear();
        for (size_t t = 0; t < maxThreads && idx < files.size(); ++t, ++idx)
        {
            fs::path outFile = outputDir / ("_" + files[idx].filename().string());
            threads.emplace_back(&processFile, files[idx], outFile);
        }

        for (auto& th : threads)
            th.join();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;

    std::cout << "Duration (steady): " << formatDuration(duration) << std::endl;
}

int main()
{
    std::cout << "Task 1.a\n";
    task_1();

    std::cout << "Task 1.b\n";
    task_2();

    std::cout << "Task 1.c\n";
    task_3();

    return 0;
}
