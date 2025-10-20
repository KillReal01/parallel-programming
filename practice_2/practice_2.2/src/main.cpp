/*
    Course: Parallel Programming
    Assignment: 2.2
    Completed by: Bereza Kirill
*/


#include <iostream>
#include <thread>
#include <chrono>
#include <semaphore>
#include <stop_token>


std::binary_semaphore sem_ping(1);
std::binary_semaphore sem_pong(0);


void ping(std::stop_token st)
{
    while (!st.stop_requested())
    {
        sem_ping.acquire();
        std::cout << "ping\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sem_pong.release();
    }
}

void pong(std::stop_token st)
{
    while (!st.stop_requested())
    {
        sem_pong.acquire();
        std::cout << "pong\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sem_ping.release();
    }
}

int main()
{
    std::stop_source ss;
    std::cout << "Press any key to stop the ping-pong...\n";

    std::jthread th1(&ping, ss.get_token());
    std::jthread th2(&pong, ss.get_token());

    std::cin.get();
    ss.request_stop();
    std::cout << "The ping-pong stopped\n";

    return 0;
}
