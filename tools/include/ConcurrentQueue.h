//
// Created by yuan on 7/4/19.
//
#pragma once

#ifndef TOOLS_CONCURRENTQUEUE_H
#define TOOLS_CONCURRENTQUEUE_H

#include <map>
#include <vector>
#include <deque>
#include <queue>
#include <mutex>    // linker "pthread"
#include <thread>
#include <chrono>
#include <cstddef>

/* implements a thread-safe queue supporting
 * single item push and pops as well as batch push
 * element type T must support default constructor
 * */
template <typename T, typename Container = std::deque<T>>
class ConcurrentQueue {
    std::size_t totalPushedItems_ = 0;
    std::size_t totalPoppedItems_ = 0;

    std::queue<T> queue_;
    std::mutex mutex_;
    bool quitSignal_ = false;

public:
    explicit ConcurrentQueue(const Container& cont = Container()) : queue_(cont) { }
    explicit ConcurrentQueue(Container&& cont) : queue_(cont) { }
    ConcurrentQueue(const std::queue<T>&& other) : queue_(other) { }
    template <class Alloc>
    explicit ConcurrentQueue(const Alloc& alloc) : queue_(alloc) { }
    template <class Alloc>
    ConcurrentQueue(const Container& cont, const Alloc& alloc):
        queue_(cont, alloc) { }
    template <class Alloc>
    ConcurrentQueue(Container&& cont, const Alloc& alloc):
        queue_(cont, alloc) { }
    template <class Alloc>
    ConcurrentQueue(const std::queue<T>&& other, const Alloc& alloc):
        queue_(other, alloc) { }

    /* disable copy constructor */
    ConcurrentQueue(const ConcurrentQueue&) = delete;

    void setQuitSignal()
    {
        quitSignal_ = true;
    }

    /* push a single item to the queue */
    void push(const T& item)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        queue_.push(item);
        totalPushedItems_++;
    }

    /* push a list of items to the queue, batch push*/
    void push(const std::vector<T>& items)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for (auto item : items)
            queue_.push(item);
        totalPushedItems_ += items.size();
    }

    /* pops a single item from the queue,
     * first item is the popped item
     * second item is set to true if the queue is
     * no longer being pushed/produced by the producer*/
    std::pair<T, bool> pop()
    {
        using namespace std;

        mutex_.lock();
        while (queue_.empty()) {
            mutex_.unlock();
            if (quitSignal_)
                return pair<T, bool>{T(), true};

            this_thread::sleep_for(chrono::milliseconds(2));
            mutex_.lock();
        }


        pair<T, bool> ret{queue_.front(), false};
        queue_.pop();
        totalPoppedItems_++;

        mutex_.unlock();

        return ret;
    }

    std::size_t size()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        return queue_.size();
    }

    std::size_t totalPoppedItems()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        return totalPoppedItems_;
    }

    std::size_t totalPushedItems()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        return totalPushedItems_;
    }

    bool isQuit() const
    {
        return quitSignal_;
    }
};


#endif //TOOLS_CONCURRENTQUEUE_H
