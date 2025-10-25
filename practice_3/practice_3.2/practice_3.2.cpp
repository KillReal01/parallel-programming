/*
    Course: Parallel Programming
    Assignment: 3.2
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <thread>
#include <future>
#include <string>
#include <stdexcept>


int main()
{
    auto res = std::async(std::launch::async, []() {
        std::string str;
        while (true)
        {
            std::cout << "Enter value: ";
            std::cin >> str;
            size_t index;
            int value = std::stoi(str, &index);
            if (index != str.size())
                throw std::invalid_argument("not an integer");
        }
     });

    try
    {
        res.get();
    }
    catch (const std::invalid_argument& ex)
    {
        std::cout << "invalid_argument: " << ex.what() << std::endl;
    }
    catch (const std::out_of_range& ex)
    {
        std::cout << "out_of_range: " << ex.what() << std::endl;
    }

    return 0;
}

