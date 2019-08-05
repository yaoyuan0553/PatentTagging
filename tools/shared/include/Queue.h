//
// Created by yuan on 8/4/19.
//

#pragma once
#ifndef TOOLS_QUEUE_H
#define TOOLS_QUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>


/* thread-safe implementation of std::queue */
template <typename T>
class ConcurrentQueue : private std::queue<T> {
    using Base = std::queue<T>;
protected:
    /* mutex protecting data access */
    std::mutex mutex_;
    /* flag for signaling if the queue is no longer being pushed (no new data) */
    std::atomic_bool quitSignal_ = false;
public:
    ConcurrentQueue() = default;

    /* allocate memory */
    void reserve(size_t newSize)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        Base::reserve(newSize);
    }

    T& front()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        return Base::front();
    }

    const T& front() const
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        return Base::front();
    }

    virtual void push(const T& item)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        Base::push(item);
    }

    virtual void push(T&& item)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        Base::push(item);
    }

    virtual void push(const std::vector<T>& items)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for (const T& item : items)
            Base::push(item);
    }

    virtual std::pair<T, bool> pop()
    {
        mutex_.lock();
        while (Base::empty()) {
            mutex_.unlock();
            if (quitSignal_.load())
                return std::pair<T, bool>{T(), true};
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            mutex_.lock();
        }

        std::pair<T, bool> ret{Base::front(), false};
        Base::pop();
        mutex_.unlock();

        return ret;
    }
};


/* shorter name for ConcurrentQueue */
template <typename T>
using CQueue = ConcurrentQueue<T>;


#endif //TOOLS_QUEUE_H
