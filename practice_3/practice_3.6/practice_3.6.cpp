/*
    Course: Parallel Programming
    Assignment: 3.6
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <cctype>


class Queue
{
public:
    explicit Queue(size_t size = 10)
        : _buffer(size), _size(size), _head_index(0), _tail_index(0), timeout(10)
    { }

    ~Queue()
    {
        _cv_produce.notify_all();
        _cv_consume.notify_all();
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void push(char c)
    {
        {
            std::unique_lock lock(_mtx);
            auto status = _cv_produce.wait_for(lock, timeout, [this]() {
                size_t fact_size = (_tail_index >= _head_index)
                    ? _tail_index - _head_index
                    : _size - _head_index + _tail_index;
                return fact_size < _size;
            });

            if (status)
            {
                _buffer[_tail_index] = c;
                _tail_index = (_tail_index + 1) % _size;
            }
            else
                return;
        }
        _cv_consume.notify_one();
    }

    char* pop()
    {
        char ch;
        {
            std::unique_lock lock(_mtx);
            auto status = _cv_consume.wait_for(lock, timeout, [this]() { return _head_index != _tail_index; });
            if (status)
            {
                ch = _buffer[_head_index];
                _head_index = (_head_index + 1) % _size;
            }
            else
                return nullptr;
        }
        _cv_produce.notify_one();
        return &ch;
    }

private:
    std::vector<char> _buffer;
    size_t _size;
    size_t _head_index;
    size_t _tail_index;

    std::mutex _mtx;
    std::condition_variable _cv_produce;
    std::condition_variable _cv_consume;
    // end of work

    std::chrono::milliseconds timeout;
};


void producer(Queue& queue, char c)
{
    while (true)
    {
        queue.push(c);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void consumer(Queue& queue)
{
    while (true)
    {
        char* ch = queue.pop();
        if (ch)
            std::cout << char(std::tolower(*ch));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

int main()
{
    Queue q;

    std::thread th1(producer, std::ref(q), 'A');
    std::thread th2(consumer, std::ref(q));

    th1.join();
    th2.join();

    return 0;
}

