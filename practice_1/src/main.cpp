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


std::vector<fs::path> getInputFiles(const fs::path& path)
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

void printTime(std::chrono::nanoseconds time)
{
    using namespace std::chrono;

    auto h  = duration_cast<hours>(time);
    auto m  = duration_cast<minutes>(time % hours(1));
    auto s  = duration_cast<seconds>(time % minutes(1));
    auto ms = duration_cast<milliseconds>(time % seconds(1));
    auto us = duration_cast<microseconds>(time % milliseconds(1));

    std::cout << "Duration: "
              << std::setfill('0')
              << std::setw(2) << h.count() << "h:"
              << std::setw(2) << m.count() << "m:"
              << std::setw(2) << s.count() << "s."
              << std::setw(3) << ms.count() << "ms."
              << std::setw(3) << us.count() << "us"
              << std::endl;
}

int main()
{
    fs::path inputDir = "input_text";
    fs::path outputDir = "output_text";

    if (!fs::exists(outputDir))
        fs::create_directory(outputDir);

    auto files = getInputFiles(inputDir);

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

    printTime(duration);

    return 0;
}
