//
// Created by yuan on 8/15/19.
//

/*****************************************
Concurrent Static Queue - Yuan Yao 11/13/2016
Inherited class of StaticQueue. This is a
thread-safe implementation of the StaticQueue.
push() & pop() are guarded by a critical section
with sleep-spinning model to avoid kernel
calls unlike using semaphores.

 modified on 8/15/2019
*****************************************/

#pragma once
#ifndef TOOLS_CONCURRENTSTATICQUEUE_H
#define TOOLS_CONCURRENTSTATICQUEUE_H

#include "StaticQueue.h"

#include <mutex>
#include <atomic>
#include <vector>
#include <thread>


#define DEFAULT_STATIC_QUEUE_SIZE (1U << 14)


template <typename T>
class ConcurrentStaticQueue {

    StaticQueue<T> staticQueue_;

    std::mutex mutex_;
    std::atomic_bool quitSignal_ = false;

    // stats variables
    std::atomic_size_t totalPushedItems_ = 0;
    std::atomic_size_t totalPoppedItems_ = 0;

public:
    explicit ConcurrentStaticQueue(size_t size = DEFAULT_STATIC_QUEUE_SIZE) :
            staticQueue_(size) { }

    void setQuitSignal()
    {
        quitSignal_.store(true);
    }

    void push(const T& item)
    {
        using namespace std;

        // sleep-spinning
        mutex_.lock();
        while (staticQueue_.full()) {
            mutex_.unlock();
            this_thread::sleep_for(chrono::milliseconds(2));
            mutex_.lock();
        }
        staticQueue_.push(item);
        mutex_.unlock();

        totalPushedItems_++;
    }

    void push(const std::vector<T>& items)
    {
        using namespace std;

        size_t size = items.size();

        mutex_.lock();
        while (staticQueue_.size() + size > staticQueue_.capacity()) {
            mutex_.unlock();
            this_thread::sleep_for(chrono::milliseconds(2));
            mutex_.lock();
        }
        for (auto item : items)
            staticQueue_.push(item);
        mutex_.unlock();

        totalPushedItems_ += size;
    }

    /* pops a single item from the queue,
     * first item is the popped item
     * second item is set to true if the queue is
     * no longer being pushed/produced by the producer*/
    std::pair<T, bool> pop()
    {
        using namespace std;

        mutex_.lock();
        while (staticQueue_.empty()) {
            mutex_.unlock();
            if (quitSignal_.load())
                return pair<T, bool>{T(), true};

            this_thread::sleep_for(chrono::milliseconds(2));
            mutex_.lock();
        }

        pair<T, bool> ret{staticQueue_.pop(), false};

        mutex_.unlock();

        totalPoppedItems_++;

        return ret;
    }

    /* for size checking */
    std::size_t size()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        return staticQueue_.size();
    }

    std::size_t totalPoppedItems()
    {
        return totalPoppedItems_;
    }

    std::size_t totalPushedItems()
    {
        return totalPushedItems_;
    }

    bool isQuit() const
    {
        return quitSignal_.load();
    }
};


#endif //TOOLS_CONCURRENTSTATICQUEUE_H
