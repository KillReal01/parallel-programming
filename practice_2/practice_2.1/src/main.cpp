/*
    Course: Parallel Programming
    Assignment: 2.1
    Completed by: Bereza Kirill
*/


#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <exception>


template<typename T>
class threadsafe_stack
{
    std::vector<T> _data;
    mutable std::shared_mutex _mtx;

public:

    threadsafe_stack() = default;
    ~threadsafe_stack() = default;

    threadsafe_stack(const threadsafe_stack& other)
    {
        std::shared_lock<std::shared_mutex> lock(other._mtx);
        _data = other._data;
    }

    threadsafe_stack& operator=(const threadsafe_stack& other)
    {
        {
            std::scoped_lock lock(_mtx, other._mtx);
            if (this != &other)
            {
                _data = other._data;
            }
        }
        return *this;
    }

    threadsafe_stack(threadsafe_stack&& other) noexcept
    {
        std::lock_guard<std::shared_mutex> lock(other._mtx);
        _data = std::move(other._data);
    }

    threadsafe_stack& operator=(threadsafe_stack&& other) noexcept
    {
        {
            std::scoped_lock lock(_mtx, other._mtx);
            if (this != &other)
            {
                _data = std::move(other._data);
            }
        }
        return *this;
    }

    bool empty() const
    {
        std::shared_lock<std::shared_mutex> lock(_mtx);
        return _data.empty();
    }

    void push(T value)
    {
        std::lock_guard<std::shared_mutex> lock(_mtx);
        _data.push_back(std::move(value));
    }

    void pop(T& value)
    {
        std::lock_guard<std::shared_mutex> lock(_mtx);
        if (_data.empty())
            throw std::runtime_error("stack is empty");
        value = _data.back();
        _data.pop_back();
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::shared_mutex> lock(_mtx);
        if (_data.empty())
            return nullptr;
        auto res = std::make_shared<T>(_data.back());
        _data.pop_back();
        return res;
    }

    T top() const
    {
        std::shared_lock<std::shared_mutex> lock(_mtx);
        if (_data.empty())
            throw std::runtime_error("stack is empty");
        return _data.back();
    }

    size_t size() const
    {
        std::shared_lock lock(_mtx);
        return _data.size();
    }
};

template<typename T>
void produceValue(std::stop_token st, threadsafe_stack<T>& stack, T value, int delayMs)
{
    std::stop_callback sc {st, [id = std::this_thread::get_id()](){
        std::cout << "Producer Id: " << id << " finished\n";
    }};

    while (!st.stop_requested())
    {
        stack.push(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
}

template<typename T>
void consumeValue(std::stop_token st, threadsafe_stack<T>& stack, std::exception_ptr& exception)
{
    while (!st.stop_requested())
    {
        T value;
        try
        {
            stack.pop(value);
        }
        catch(...)
        {
            exception = std::current_exception();
            break;
        }
        
        std::cout << "Consumer gets: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    threadsafe_stack<int32_t> stack;
    int32_t producerNum = 3;
    std::vector<std::jthread> producers;
    producers.reserve(producerNum);

    std::cout << "Producers starting\n";
    for (int32_t i = 0; i < producerNum; i++)
    {
        int delayMs = 10 * (rand() % 100 + 1);
        producers.emplace_back(&produceValue<int32_t>, std::ref(stack), i, delayMs);
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::exception_ptr exception;
    std::cout << "Consumer starting\n";
    std::jthread th(&consumeValue<int32_t>, std::ref(stack), std::ref(exception));

    th.join();
    std::cout << "Consumer finished\n";

    for (auto& prodecer : producers)
        prodecer.request_stop();

    if (exception)
    {
        try { std::rethrow_exception(exception); }
        catch(const std::exception& e) { std::cout << "Consumer got exception in main: " << e.what() << std::endl; }
    }

    std::cin.get();
    return 0;
}
