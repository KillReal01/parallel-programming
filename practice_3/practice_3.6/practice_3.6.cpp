/*
    Course: Parallel Programming
    Assignment: 3.6
    Completed by: Bereza Kirill
*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <chrono>
#include <cctype>


class Queue
{
public:
    explicit Queue(size_t size = 10)
        : _buffer(size), _size(size), _head(0), _tail(0), _current_size(0), _stop(false)
    { }

    ~Queue()
    {
        stop();
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&& other) noexcept
        : _buffer(std::move(other._buffer)),
        _size(other._size),
        _head(other._head),
        _tail(other._tail),
        _current_size(other._current_size),
        _stop(other._stop.load())
    {
        other._head = other._tail = other._current_size = 0;
        other._stop = true;
    }

    Queue& operator=(Queue&& other) noexcept
    {
        if (this != &other)
        {
            std::scoped_lock lock(_mtx, other._mtx);

            _buffer = std::move(other._buffer);
            _head = other._head;
            _tail = other._tail;
            _current_size = other._current_size;
            _stop = other._stop.load();

            other._head = other._tail = other._current_size = 0;
            other._stop = true;
        }
        return *this;
    }

    void stop()
    {
        {
            std::lock_guard lock(_mtx);
            _stop = true;
        }
        _cv_produce.notify_all();
        _cv_consume.notify_all();
    }

    bool push(char c)
    {
        {
            std::unique_lock lock(_mtx);
            bool status = _cv_produce.wait_for(lock, timeout, [this] {return _current_size < _size || _stop; });

            if (_stop || !status)
                return false;

            _buffer[_tail] = c;
            _tail = (_tail + 1) % _size;
            ++_current_size;
        }
        _cv_consume.notify_one();
        return true;
    }

    std::optional<char> pop()
    {
        char c;
        {
            std::unique_lock lock(_mtx);
            bool status = _cv_consume.wait_for(lock, timeout, [this] { return _current_size > 0 || _stop; });

            if (_stop || !status)
                return std::nullopt;

            c = _buffer[_head];
            _head = (_head + 1) % _size;
            --_current_size;
        }
        _cv_produce.notify_one();
        return c;
    }

private:
    std::vector<char> _buffer;
    const size_t _size;
    size_t _head;
    size_t _tail;
    size_t _current_size;

    std::mutex _mtx;
    std::condition_variable _cv_produce;
    std::condition_variable _cv_consume;

    std::atomic<bool> _stop;
    const std::chrono::milliseconds timeout{ 500 };
};


void producer(Queue& q, char c)
{
    while (q.push(c))
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void consumer(Queue& q)
{
    while (true)
    {
        auto ch = q.pop();
        if (!ch.has_value())
            break;
        std::cout << char(std::tolower(*ch)) << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main()
{
    Queue q(5);

    std::thread t1(producer, std::ref(q), 'A');
    std::thread t2(producer, std::ref(q), 'B');
    std::thread t3(consumer, std::ref(q));
    std::thread t4(consumer, std::ref(q));

    std::this_thread::sleep_for(std::chrono::seconds(3));
    q.stop();

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::cout << "\nStopped\n";
    return 0;
}
