/*
    Course: Parallel Programming
    Assignment: 4.1
    Completed by: Bereza Kirill
*/

// Задание корутины 1.
// Дано: несколько файлов, в которых в форматированном виде хранятся
// целые значения (разделенные пробелом). Для упрощения задачи в каждом
// файле задаем одинаковое количество значений.Но лучше учесть, что количество может быть разное!
// 
// Требуется посредством корутин вычислять и выводить на экран(или в файл) среднее соответствующих (i - тых значений)


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <coroutine>
#include <optional>
#include <filesystem>


int generateInteger(int minVal = 1, int maxVal = 1000)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(gen);
}

void writeToFile(const std::string& filename, int numberCount)
{
    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        std::cerr << "Can't open file for write: " << filename << std::endl;
        return;
    }

    for (size_t i = 0; i < numberCount; ++i)
    {
        outFile << generateInteger();
        if (i + 1 < numberCount)
            outFile << " ";
    }
    outFile << std::endl;
    outFile.close();
}


std::vector<std::string> generateFiles(int numberFiles)
{
    const std::string dir = "numbers";
    std::filesystem::create_directories(dir);
    std::vector<std::string> filenames;
    for (int i = 0; i < numberFiles; ++i)
    {
        std::string filename = dir + "/file_" + std::to_string(i + 1) + ".txt";

        int numberCount = generateInteger();
        writeToFile(filename, numberCount);
        filenames.push_back(filename);

        std::cout << "Created file: " << filename << " with " << numberCount << " numbers" << std::endl;
    }
    return filenames;
}

template <typename T>
struct Generator
{
    struct promise_type
    {
        std::optional<T> current_value;

        Generator get_return_object() { return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) noexcept {
            current_value = value;
            return {};
        }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type handle;

    explicit Generator(handle_type h) : handle(h) {}
    Generator(const Generator&) = delete;
    Generator(Generator&& other) noexcept : handle(other.handle) { other.handle = {}; }
    ~Generator() { if (handle) handle.destroy(); }

    std::optional<T> next()
    {
        if (!handle || handle.done())
            return std::nullopt;
        handle.resume();
        if (handle.done())
            return std::nullopt;
        return handle.promise().current_value;
    }
};

Generator<int> readFromFile(const std::string& filename)
{
    std::ifstream in(filename);
    int value;
    while (in >> value)
        co_yield value;
}

int main()
{
    const int numberFiles = 1000;
    auto filenames = generateFiles(numberFiles);

    std::vector<Generator<int>> generators;
    generators.reserve(numberFiles);
    for (const auto& filename : filenames)
        generators.push_back(readFromFile(filename));

    std::ofstream result("result.txt");
    if (!result.is_open())
    {
        std::cerr << "Cannot open result file\n";
        return 1;
    }

    int index = 0;
    for (;;)
    {
        float sum = 0;
        int count = 0;
        for (auto& gen : generators)
        {
            if (auto value = gen.next())
            {
                sum += value.value();
                ++count;
            }
        }
        if (count == 0)
            break;
        float avg = sum / count;
        result << "Index: " << index++ << "\tavg : " << avg << '\n';
    }

    return 0;
}
