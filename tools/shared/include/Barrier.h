//
// Created by yuan on 9/5/19.
//

#pragma once
#ifndef SWIGTEST_BARRIER_H
#define SWIGTEST_BARRIER_H

#include <pthread.h>


class Barrier {
    pthread_barrier_t barrier_;
public:
    /**
     * @brief initialize barrier with count
     * @param count count of the barrier
     */
    explicit Barrier(unsigned count) noexcept;

    ~Barrier() noexcept;

    Barrier(const Barrier&) = delete;
    Barrier(Barrier&&) = delete;

    /*! @brief blocks calling thread on barrier until count reaches 0 */
    void wait();
};


#endif //SWIGTEST_BARRIER_H
