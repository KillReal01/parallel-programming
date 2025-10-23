/*
    Course: Parallel Programming
    Assignment: 2.4
    Completed by: Bereza Kirill
*/


#include <string>
#include <iostream>
#include <thread>
#include <vector>
// #include <syncstream>


void printString(int threadNumber)
{
    thread_local std::string s("hello from ");
    s += std::to_string(threadNumber);

    // std::osyncstream stream(std::cout);
    // stream << "Address: " << &s << " string: " << s << std::endl;
    std::cout << "Address: " << &s << " string: " << s << std::endl;
}

int main()
{
    const int num_threads = 4;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(&printString, i + 1);

    for (auto& t : threads)
        t.join();

    std::cin.get();
    return 0;
}
