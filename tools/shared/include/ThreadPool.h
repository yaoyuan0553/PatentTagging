//
// Created by yuan on 7/22/19.
//

#pragma once
#ifndef TOOLS_THREADPOOL_H
#define TOOLS_THREADPOOL_H

#include <vector>
#include <array>
#include <type_traits>

#include "ThreadJob.h"
#include "ConcurrentQueue.h"


struct ThreadPoolInterface {
    virtual void runAll() = 0;
    virtual void waitAll() = 0;
};

struct ThreadPoolCollection : public ThreadPoolInterface { };


class ThreadPool : public ThreadPoolInterface, private std::vector<ThreadInterface*> {
public:
/*
    */
/* initialize threads with a callback function *//*

    template <typename ThreadInitFn>
    ThreadPool(int n, ThreadInitFn&& threadInitFn)
    {
        static_assert(std::is_invocable_v<ThreadInitFn, decltype(threadPool_)&>,
                "ThreadInitFn expects argument type of (vector<ThreadInterface>&)");
        threadPool_.reserve(n);
        threadInitFn(threadPool_);
    }
*/
    ThreadPool() = default;

    virtual ~ThreadPool()
    {
        for (ThreadInterface* t : *this)
            delete t;
    }

    ThreadPool(const ThreadPool&) = default;

    ThreadPool(ThreadPool&& threadPool) = default;

/*
    void add(ThreadInterface* t)
    {
        push_back(t);
    }
*/
    template <class ThreadInterfaceSubclass, typename... Args>
    void add(Args&&... args)
    {
        static_assert(std::is_base_of_v<ThreadInterface, ThreadInterfaceSubclass>,
                "First argument of the template must be a subclass of ThreadInterface");
        push_back(new ThreadInterfaceSubclass(std::forward<Args>(args)...));
    }

    template <int N, class ThreadInterfaceSubclass, typename... Args>
    void add(Args&&... args)
    {
        static_assert(std::is_base_of_v<ThreadInterface, ThreadInterfaceSubclass>,
                      "First argument of the template must be a subclass of ThreadInterface");
        for (int i = 0; i < N; i++)
            add<ThreadInterfaceSubclass, Args...>(std::forward<Args>(args)...);
//            push_back(new ThreadInterfaceSubclass(std::forward<Args>(args)...));
    }
    template <class ThreadInterfaceSubclass, typename... Args>
    void add(int N, Args&&... args)
    {
        static_assert(std::is_base_of_v<ThreadInterface, ThreadInterfaceSubclass>,
                      "First argument of the template must be a subclass of ThreadInterface");
        for (int i = 0; i < N; i++)
            add<ThreadInterfaceSubclass, Args...>(std::forward<Args>(args)...);
//        push_back(new ThreadInterfaceSubclass(std::forward<Args>(args)...));
    }

    void runAll() override
    {
        for (ThreadInterface* t : *this)
            t->run();
    }
    void waitAll() override
    {
        for (ThreadInterface* t : *this)
            t->wait();
    }
};


template <typename DataPool = void>
class PCThreadPool : public ThreadPoolCollection {
protected:
    DataPool& dataPool_;
public:
    ThreadPool producers;
    ThreadPool consumers;

    explicit PCThreadPool(DataPool& dataPool) : dataPool_(dataPool) { }

    PCThreadPool(ThreadPool&& producers, ThreadPool&& consumers, DataPool& dataPool) :
        producers(std::move(producers)), consumers(std::move(consumers)),
        dataPool_(dataPool) { }

    PCThreadPool(PCThreadPool&&) noexcept = default;

    void runAll() override
    {
        producers.runAll();
        consumers.runAll();
    }

    void waitAll() override
    {
        producers.waitAll();
        consumers.waitAll();
    }
};

template <>
class PCThreadPool<> : public ThreadPoolCollection {
public:
    ThreadPool producers;
    ThreadPool consumers;

    PCThreadPool() = default;

    PCThreadPool(ThreadPool&& producers, ThreadPool&& consumers) :
            producers(std::move(producers)), consumers(std::move(consumers)) { }

    PCThreadPool(PCThreadPool&&) noexcept = default;

    void runAll() override
    {
        producers.runAll();
        consumers.runAll();
    }

    void waitAll() override
    {
        producers.waitAll();
        consumers.waitAll();
    }
};




#endif //TOOLS_THREADPOOL_H
