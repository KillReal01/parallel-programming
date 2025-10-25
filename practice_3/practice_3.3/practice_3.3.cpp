#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>
#include <stop_token>


std::mutex mtx;
std::condition_variable cv;
bool ping_turn = true;


void ping(std::stop_token st)
{
    while (!st.stop_requested())
    {
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [] { return ping_turn; });
            std::cout << "ping\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::lock_guard lock(mtx);
            ping_turn = false;
        }
        cv.notify_one();
    }
}

void pong(std::stop_token st)
{
    while (!st.stop_requested())
    {
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [] { return !ping_turn; });
            std::cout << "pong\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::lock_guard lock(mtx);
            ping_turn = true;
        }
        cv.notify_one();
    }
}

int main()
{
    std::cout << "Starting ping-pong...\n";
    std::stop_source ss;

    std::jthread t1(ping, ss.get_token());
    std::jthread t2(pong, ss.get_token());

    std::cin.get();
    ss.request_stop();
    std::cout << "Finished ping-pong\n";

    return 0;
}
