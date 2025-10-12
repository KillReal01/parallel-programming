#include <iostream>
#include <thread>
#include <filesystem>
#include <vector>
#include <string>


int main()
{
    std::vector<std::string> files;
    std::string path = "input_text";

    for (const auto& entry : std::filemsystem::directory_iterator(path)) {
        if (entry.is_regular_file())
            files.push_back(entry.path().string());
    }

    // вывод для проверки
    for (const auto& f : files)
        std::cout << f << '\n';
    return 0;
}