//
// Created by yuan on 8/28/19.
//

#pragma once
#ifndef SWIGTEST_CONCURRENTBLOCKINGSTATICQUEUE_H
#define SWIGTEST_CONCURRENTBLOCKINGSTATICQUEUE_H

#include "StaticQueue.h"
#include "Semaphore.h"
#include <mutex>
#include <atomic>

/**
 * @brief bounded producer consumer queue
 * @details uses semaphores to implement a blocking PC queue
 *          differs from the self-spinning ConcurrentStaticQueue
 * @tparam T type of the element to be stored
 */
template <typename T>
class ConcurrentBlockingStaticQueue {
protected:
    // queue
    StaticQueue<T> staticQueue_;

    // synchronization members
    Semaphore produced_, consumed_;
    std::mutex m_;

    // quit signal
    std::atomic_bool quit_ = false;

    // maximum number of consumers
    int maxConsumers_;

public:
    /**
     * @brief constructs a ConcurrentStaticQueue object
     * @param maxSize sets queue's buffer size which is sizeof(T) * maxSize in bytes
     * @param writeAhead sets maximum number of elements can be produced (pushed) without consumers running
     */
    ConcurrentBlockingStaticQueue(size_t maxSize, int writeAhead, int maxConsumers) :
            staticQueue_(maxSize), consumed_(writeAhead), maxConsumers_(maxConsumers)
    {
        /* writeAhead limits produced, therefore, if this condition is satisfied
         * the queue will never exceed full */
        if (writeAhead <= 0)
            PERROR("writeAhead must be positive integer");
        if ((size_t)writeAhead > maxSize)
            PERROR("writeAhead cannot be larger than maximum queue size");
    }

    void push(const T& item)
    {
        // wait for empty semaphores
        consumed_.wait();

        // lock, produce, and unlock
        m_.lock();
        staticQueue_.push(item);
        m_.unlock();

        // release newly produced semaphore
        produced_.release();
    }

    void emplace_push(T&& item)
    {
        // wait for empty semaphores
        consumed_.wait();

        // lock, produce, and unlock
        m_.lock();
        staticQueue_.emplace_push(std::forward<T>(item));
        m_.unlock();

        // release newly produced semaphore
        produced_.release();
    }


    /**
     * @brief pops a single element from queue (consumes an item)
     * @return {item, quit}
     *      if quit is true,
     *          then it means the queue is empty and no longer being produced (pushed)
     *          and item is set to default value of T, using T's default constructor T()
     *      else
     *          it means an item is being successfully consumed (popped)
     */
    std::pair<T, bool> pop()
    {
        // wait for produced data
        produced_.wait();

        // lock, consume, and unlock
        m_.lock();

        // if queue is empty and quit is true, exit
        if (quit_ && staticQueue_.empty()) {
            // unlock mutex first
            m_.unlock();
            // quit
            return std::pair{T(), true};
        }

        std::pair<T, bool> ret{staticQueue_.pop(), false};
        m_.unlock();

        consumed_.release();

        return ret;
    }

    /**
     * @brief sets queue into "quit" state
     * @details sets quit_ flag to true and releases maxConsumer amount of semaphores to full.
     *          this means the queue is no longer being pushed/produced upon, no new elements will be added.
     *          Consumer threads will exit when the queue is empty. Releasing maxConsumer number of produced semaphores
     *          to make sure that all consumer threads can proceed to the exit routine.
     */
    void setQuitSignal()
    {
        quit_ = true;

        produced_.release(maxConsumers_);
    }

    /**
     * @brief resets queue back to initial state, with quit flag being false
     */
    void reset()
    {
        quit_ = false;
    }
};


template <typename T>
using CBSQueue = ConcurrentBlockingStaticQueue<T>;


#endif //SWIGTEST_CONCURRENTBLOCKINGSTATICQUEUE_H
