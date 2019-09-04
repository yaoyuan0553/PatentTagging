//
// Created by yuan on 8/28/19.
//

#pragma once
#ifndef SWIGTEST_SEMAPHORE_H
#define SWIGTEST_SEMAPHORE_H

#include <semaphore.h>

#include "Utility.h"

class Semaphore {
    sem_t sem_{};
public:
    /* initialize semaphore by count n (default 0) */
    explicit Semaphore(unsigned int n = 0) noexcept;
    ~Semaphore() noexcept;

    /* disable copy or move */
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&& other) = delete;

    /* wait for semaphore */
    void wait();

    /* release semaphore by count n*/
    void release(int n = 1);
};


#endif //SWIGTEST_SEMAPHORE_H
