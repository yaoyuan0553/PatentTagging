//
// Created by yuan on 7/4/19.
//

#include "Semaphore.h"

/*
 * code snippet source:
 * https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
 * by Maxim Egorushkin
 */
void Semaphore::release(unsigned int count)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    count_ += count;
    if (count == 1)
        condition_.notify_one();
    else
        condition_.notify_all();
}

void Semaphore::wait()
{
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    while(!count_) // Handle spurious wake-ups.
        condition_.wait(lock);
    --count_;
}

bool Semaphore::try_wait()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    if(count_) {
        --count_;
        return true;
    }
    return false;
}
