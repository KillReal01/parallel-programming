#include <iostream>
#include <thread>
#include <filesystem>
#include <vector>
#include <string>

// mkdir build && cd build
// cmake ..
// cmake --build .
// .\Debug\practice-1.1.exe


int main()
{
    std::vector<std::string> files;
    std::string path = "input_text";

    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file())
            files.push_back(entry.path().string());
    }

    // вывод для проверки
    for (const auto& f : files)
        std::cout << f << '\n';
    return 0;
}