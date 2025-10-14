#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>


#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
    #include <windows.h>
#elif defined(__linux__)
    #define OS_LINUX
    #include <cstdlib>
#else
    #error "Unsupported operating system"
#endif


void playNote(int frequency, int duration_ms, int count = 1)
{
#ifdef OS_LINUX
    std::string cmd = "play -n synth " + std::to_string(duration_ms / 1000.0) + " sine " + std::to_string(frequency) + " >/dev/null 2>&1";
    for (int i = 0; i < count; ++ i)
        std::system(cmd.c_str());
#elif defined(OS_WINDOWS)
    for (int i = 0; i < count; ++ i)
        Beep(frequency, duration_ms);
#endif
}

void playAllNotes()
{
    std::vector<std::thread> threads;

    threads.emplace_back(&playNote, 261, 500, 10); // ДО
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 293, 600, 10); // РЕ
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 329, 500, 8); // МИ
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 349, 500, 6); // ФА
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 392, 500, 7); // СОЛЬ
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 440, 800, 5); // ЛЯ
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    threads.emplace_back(&playNote, 493, 400, 8); // СИ

    for (auto& th : threads)
        th.detach();
}

void playGamma()
{
    std::vector<int> notes = {261, 293, 329, 349, 392, 440, 493, 523};
    int duration = 400; 

    for (int freq : notes)
    {
        std::thread th(&playNote, freq, duration, 1);
        th.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void playAccord()
{
    std::vector<int> chord = {261, 329, 392};
    int duration = 1000;

    std::vector<std::thread> threads;
    threads.reserve(chord.size());

    for (int freq : chord)
        threads.emplace_back(&playNote, freq, duration, 1);

    for (auto& th : threads)
        th.detach();
}

int main()
{
    // 1
    std::cout << "Playing all notes...\n";
    playAllNotes();
    std::this_thread::sleep_for(std::chrono::seconds(7));
    std::cout << "Notes finished\n";

    // 1.a
    std::cout << "Playing gamma...\n";
    playGamma();
    std::cout << "Gamma finished.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 1.b
    std::cout << "Playing accord DO-MAJOR...\n";
    playAccord();
    std::cout << "Accord finished.\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
