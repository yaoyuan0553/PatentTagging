//
// Created by yuan on 7/4/19.
//

#ifndef TOOLS_SEMAPHORE_H
#define TOOLS_SEMAPHORE_H

#include <thread>       // linker "pthread" needed
#include <mutex>
#include <condition_variable>

/*
 * code snippet source:
 * https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
 * by Maxim Egorushkin
 */
class Semaphore {
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    Semaphore(unsigned long count) : count_(count) { }

    void release(unsigned int count = 1);
    void wait();
    bool try_wait();
};


#endif //TOOLS_SEMAPHORE_H
