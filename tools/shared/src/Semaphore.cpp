//
// Created by yuan on 8/28/19.
//

#include "Semaphore.h"

Semaphore::Semaphore(unsigned int n) noexcept
{
    // initialize POSIX semaphore
    if (sem_init(&sem_, 0, n))
        PSYS_FATAL("sem_init()");
}

Semaphore::~Semaphore() noexcept
{
    if (sem_destroy(&sem_))
        PSYS_FATAL("sem_destroy()");
}

void Semaphore::wait()
{
    if (sem_wait(&sem_))
        PSYS_FATAL("sem_wait()");
}

void Semaphore::release(int n)
{
    for (int i = 0; i < n; i++)
        if (sem_post(&sem_))
            PSYS_FATAL("sem_post()");
}

