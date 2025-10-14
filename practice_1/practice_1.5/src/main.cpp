#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <array>

std::mutex mtx;

void printString(std::stop_token st, const std::string& str)
{
    while (!st.stop_requested())
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Id: " << std::this_thread::get_id() << " string: " << str << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    const std::array<std::string, 10> strings = {
        "The sun slowly dipped below the horizon, painting the sky in shades of orange and pink.",
        "The cat stretched lazily on the windowsill and closed its eyes.",
        "A light drizzle began outside, creating a soft patter on the leaves.",
        "Children laughed as they chased a ball across the green lawn.",
        "The library was quiet, broken only by the rustle of turning pages.",
        "The old house on the hill looked abandoned but still retained its grandeur.",
        "The smell of fresh bread wafted from the little bakery on the corner.",
        "The train quietly pulled away, leaving a long trail of smoke behind.",
        "The girl on the bench was reading a book and occasionally jotting notes in her notebook.",
        "The wind carried the scent of blooming lilacs through the park."
    };

    std::vector<std::jthread> threads;
    threads.reserve(strings.size());

    std::stop_source ss;

    for (size_t i = 0; i < strings.size(); i++)
    {
        // threads.emplace_back(&printString, ss.get_token(), std::cref(strings[i]));
        threads.emplace_back(&printString, std::cref(strings[i]));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cin.get();
    // ss.request_stop();

    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Stop request" << std::endl;
    }

    return 0;
}
