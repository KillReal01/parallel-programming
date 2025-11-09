/*
    Course: Parallel Programming
    Assignment: coroutine 2
    Completed by: Bereza Kirill
*/

// Задание корутины 2.
// В текущей директории (для простоты) находятся известное количество с известной спецификацией.txt - файлов.
// Файлы содержат только (для простоты) целые через естественный разделитель.
// 
// Запускаем соответствующее количество корутин - producer-ов. Каждый producer работает в отдельном потоке. Каждый "достает" очередное целое
// значение из заданного файла и приостанавливается.
// 
// Один consumer - main. Получив ото всех producer-ов по одному значению, находит среднее(выводит его) и инициирует получение следующих значений
// Хотелось бы предусмотреть, что в файлах может быть разное количество значений.
//
// PS Я использовала для синхронизации std::barrier(мне показалось, что это удобно) и два семафора.Но вы можете предложить любое другое решение = > обсудим.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <coroutine>
#include <optional>
#include <filesystem>
#include <thread>
#include <barrier>
#include <array>


int generateInteger(int minVal = 1, int maxVal = 1000)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(gen);
}

void writeToFile(const std::string& file, int numberCount)
{
    std::ofstream outFile(file);
    if (!outFile.is_open())
    {
        std::cerr << "Can't open file for write: " << file << std::endl;
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
    std::vector<std::string> files;
    for (int i = 0; i < numberFiles; ++i)
    {
        std::string file = dir + "/file_" + std::to_string(i + 1) + ".txt";

        int numberCount = generateInteger();
        writeToFile(file, numberCount);
        files.push_back(file);

        std::cout << "Created file: " << file << " with " << numberCount << " numbers" << std::endl;
    }
    return files;
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

Generator<int> readFromFile(const std::string& file)
{
    std::ifstream in(file);
    int value;
    while (in >> value)
        co_yield value;
}

void producerTask(const std::string& file, std::optional<int>& current, std::barrier<>& barrier)
{
    auto generator = readFromFile(file);
    while (true)
    {
        auto val = generator.next();
        current = val;
        barrier.arrive_and_wait();
        if (!val)
            break;
        barrier.arrive_and_wait();
    }
    barrier.arrive_and_drop();
}

int main()
{
    std::ofstream result("result.txt");
    if (!result.is_open())
    {
        std::cerr << "Cannot open result file\n";
        return 1;
    }

    const int numberFiles = 1000;
    auto files = generateFiles(numberFiles);

    std::array<std::optional<int>, numberFiles> shared;
    std::barrier barrier(numberFiles + 1);

    std::vector<std::thread> producers;
    producers.reserve(numberFiles);

    for (int i = 0; i < numberFiles; ++i)
        producers.emplace_back(producerTask, std::cref(files[i]), std::ref(shared[i]), std::ref(barrier));

    bool running = true;
    int index = 0;
    while (running)
    {
        barrier.arrive_and_wait();
        int sum = 0, count = 0;
        running = false;

        for (auto& value : shared)
        {
            if (value)
            {
                sum += *value;
                ++count;
                running = true;
            }
        }

        if (count)
            result << "Index: " << index++ << "\tavg: " << (sum / (double)count) << '\n';

        barrier.arrive_and_wait();
    }

    for (auto& t : producers)
        t.join();

    return 0;
}
