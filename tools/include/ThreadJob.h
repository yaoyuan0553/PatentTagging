//
// Created by yuan on 7/20/19.
//
#pragma once

#ifndef TOOLS_THREADJOB_H
#define TOOLS_THREADJOB_H

#endif //TOOLS_THREADJOB_H

#include <thread>
#include <iostream>
#include <utility>
#include <type_traits>
#include <functional>
#include <tuple>

template <typename T>
struct optional_ref_wrapper {
    static constexpr bool value = false;
    using type = T;
    type operator()(T val) {
        std::cout << "non-ref\n";
        return val;
    }

};

template <typename T>
struct optional_ref_wrapper<T&> {
    static constexpr bool value = true;
    using type = std::reference_wrapper<T>;
    type operator()(T& val) {
        std::cout << "l-ref\n";
        return std::ref(val);
    }
};

template <typename T>
struct optional_ref_wrapper<T&&> {
    static constexpr bool value = true;
    using type = T&&;
    type operator()(T&& val) {
        std::cout << "r-ref\n";
        return std::forward<T>(val);
    }
};

template <typename T>
using optional_ref_wrapper_t = typename optional_ref_wrapper<T>::type;


/* A joinable class wrapper for std::thread
 * this base class should be subclassed
 * forbidding any construction without subclasses
 * */
template <typename... RunArgs>
class ThreadJob {
    std::tuple<optional_ref_wrapper_t<RunArgs>...> argTuple_;
/*    void initThread(RunArgs&&... runArgs)
    {
        thread_ = std::thread(&ThreadJob::internalRun, this, runArgs...);
    }*/
protected:
    std::thread thread_;
    bool threadStarted_ = false;

    /* abstract method, must be implemented by subclasses
     * main execution routine for the current thread
     * upon starting of the thread, this function will execute*/
    virtual void internalRun(RunArgs...) = 0;

    explicit ThreadJob(RunArgs&&... runArgs) : argTuple_{optional_ref_wrapper<RunArgs>()(runArgs)...}
    {
//        initThread = [this, runArgs...] () mutable {
//            this->thread_ = std::thread(&ThreadJob::internalRun, this, optional_ref_wrapper<RunArgs>()(std::forward<RunArgs>(runArgs))...);
//        };
    }

public:
    /* virtual run function with default values, can be override in subclasses.
     * calling this function spawns thread and starts execution */
/*    virtual void run(RunArgs&&... runArgs)
    {
        thread_ = std::thread(&ThreadJob::internalRun, this, optional_ref_wrapper<RunArgs>()(runArgs)...);
        threadStarted_ = true;
    }*/
    ThreadJob(ThreadJob&&) = default;

    virtual void run()
    {
//        initThread();
//        std::apply(&ThreadJob::initThread, std::tuple_cat(std::make_tuple(this), argTuple_));
        thread_ = std::make_from_tuple<std::thread>(std::tuple_cat(
                std::make_tuple(&ThreadJob::internalRun, this), argTuple_));
        threadStarted_ = true;
    }

    /* virtual run function with default values, can be override in subclasses.
     * blocks the caller until current thread finishes
     * WARNING: this function will throw if called before thread starts */
    virtual void wait()
    {
        if (!threadStarted_) {
            std::cerr << "Thread must be started (run(...)) before call on wait()\n";
            exit(-1);
        }
        thread_.join();
    }
};
