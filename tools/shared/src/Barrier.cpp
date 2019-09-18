//
// Created by yuan on 9/5/19.
//

#include "Barrier.h"

#include "Utility.h"

Barrier::Barrier(unsigned count) noexcept
{
    // initialize POSIX barrier
    if (int err = pthread_barrier_init(&barrier_, nullptr, count); err)
        PERROR("pthread_barrier_init() failed with %s", strerror(err));
}

Barrier::~Barrier() noexcept
{
    if (int err = pthread_barrier_destroy(&barrier_); err)
        PERROR("pthread_barrier_destroy() failed with %s", strerror(err));
}

void Barrier::wait()
{
    if (int err = pthread_barrier_wait(&barrier_); err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
        PERROR("pthread_barrier_wait() failed with %s", strerror(err));
}

