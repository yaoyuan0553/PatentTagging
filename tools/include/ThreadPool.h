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

struct ThreadPoolUnit : ThreadPoolInterface {
    virtual void add(ThreadInterface*) = 0;
};

struct ThreadPoolCollection : public ThreadPoolInterface { };


class ThreadPool : public ThreadPoolUnit, private std::vector<ThreadInterface*> {
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

    ~ThreadPool() = default;

    ThreadPool(const ThreadPool&) = default;

    ThreadPool(ThreadPool&& threadPool) = default;

    void add(ThreadInterface* t) override
    {
        push_back(t);
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
    ThreadPool producers_;
    ThreadPool consumers_;
public:
    explicit PCThreadPool(DataPool& dataPool) : dataPool_(dataPool) { }

    PCThreadPool(ThreadPool&& producers, ThreadPool&& consumers, DataPool& dataPool) :
        producers_(std::move(producers)), consumers_(std::move(consumers)),
        dataPool_(dataPool) { }

    PCThreadPool(PCThreadPool&&) noexcept = default;

    void addProducer(ThreadInterface* t)
    {
        producers_.add(t);
    }

    void addConsumer(ThreadInterface* t)
    {
        consumers_.add(t);
    }

    void runAll() override
    {
        producers_.runAll();
        consumers_.runAll();
    }

    void waitAll() override
    {
        producers_.waitAll();
        consumers_.waitAll();
    }
};

template <>
class PCThreadPool<> : public ThreadPoolCollection {
protected:
    ThreadPool producers_;
    ThreadPool consumers_;
public:
    PCThreadPool() = default;

    PCThreadPool(ThreadPool&& producers, ThreadPool&& consumers) :
            producers_(std::move(producers)), consumers_(std::move(consumers)) { }

    PCThreadPool(PCThreadPool&&) noexcept = default;

    void addProducer(ThreadInterface* t)
    {
        producers_.add(t);
    }

    void addConsumer(ThreadInterface* t)
    {
        consumers_.add(t);
    }

    void runAll() override
    {
        producers_.runAll();
        consumers_.runAll();
    }

    void waitAll() override
    {
        producers_.waitAll();
        consumers_.waitAll();
    }
};



#endif //TOOLS_THREADPOOL_H
