#include <string>
#include <thread>
#include <vector>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
    #include <windows.h>
#elif defined(__linux__)
    #define OS_LINUX
    #include <cstdlib>
#else
    #error "Unsupported operating system"
#endif


void playNote(int frequency, int duration_ms, int times)
{
#ifdef OS_LINUX
    std::string cmd = "play -n synth " + std::to_string(duration_ms / 1000.0) +
                      " sine " + std::to_string(frequency) + " >/dev/null 2>&1";
    for (int i = 0; i < times; ++i)
        std::system(cmd.c_str());
#elif defined(OS_WINDOWS)
    for (int i = 0; i < times; ++i)
        Beep(frequency, duration_ms);
#endif
}


int main()
{
    std::vector<std::thread> threads;

    threads.emplace_back(playNote, 261, 500, 3); // ДО
    threads.emplace_back(playNote, 293, 500, 4); // РЕ
    threads.emplace_back(playNote, 329, 500, 5); // МИ
    threads.emplace_back(playNote, 349, 500, 6); // ФА
    threads.emplace_back(playNote, 392, 500, 7); // СОЛЬ
    threads.emplace_back(playNote, 440, 500, 8); // ЛЯ
    threads.emplace_back(playNote, 493, 500, 9); // СИ

    for (auto& th : threads)
        th.detach(); // потоки работают независимо

    std::this_thread::sleep_for(std::chrono::seconds(6)); // ждём окончания всех сигналов

    return 0;
}
