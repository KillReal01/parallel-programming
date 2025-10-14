#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    int delay = 100;
    int delta = 10;

    for (char ch = 'a'; ch <= 'z'; ++ch)
    {
        std::cout << ch << std::flush;
        delay += delta;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay)); 
    }
    std::cout << std::endl;
    return 0;
}
