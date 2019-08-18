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

#include "MetaFunctionHelpers.h"


/* An interface class defining abstract methods
 * all child classes should support */
struct ThreadInterface {
    /* run current thread function in main */
    virtual void runOnMain() = 0;
    /* starts the current thread */
    virtual void run() = 0;
    /* blocks the caller until this thread finishes */
    virtual void wait() = 0;

    virtual ~ThreadInterface() = default;
};


/* A joinable class wrapper for std::thread
 * this base class should be subclassed
 * forbidding any construction without subclasses
 * */
template <typename... RunArgs>
class ThreadJob : public ThreadInterface {
    std::tuple<optional_ref_wrapper_t<RunArgs>...> argTuple_;
protected:
    std::thread thread_;
    bool threadStarted_ = false;

    /* abstract method, must be implemented by subclasses
     * main execution routine for the current thread
     * upon starting of the thread, this function will execute*/
    virtual void internalRun(RunArgs...) = 0;

    explicit ThreadJob(RunArgs&&... runArgs) :
        argTuple_{optional_ref_wrapper<RunArgs>()(std::forward<RunArgs>(runArgs))...} { }

public:
    /* virtual run function with default values, can be override in subclasses.
     * calling this function spawns thread and starts execution */
/*    virtual void run(RunArgs&&... runArgs)
    {
        thread_ = std::thread(&ThreadJob::internalRun, this, optional_ref_wrapper<RunArgs>()(runArgs)...);
        threadStarted_ = true;
    }*/
    ThreadJob(ThreadJob&&) noexcept = default;

    ~ThreadJob() override = default;

    void runOnMain() override
    {
        std::apply(&ThreadJob::internalRun, std::tuple_cat(
                std::make_tuple(this), argTuple_));
    }

    void run() override
    {
        thread_ = std::make_from_tuple<std::thread>(std::tuple_cat(
                std::make_tuple(&ThreadJob::internalRun, this), argTuple_));
        threadStarted_ = true;
    }

    /* virtual run function with default values, can be override in subclasses.
     * blocks the caller until current thread finishes
     * WARNING: this function will throw if called before thread starts */
    void wait() override
    {
        if (!threadStarted_) {
            std::cerr << "Thread must be started (run(...)) before call on wait()\n";
            exit(-1);
        }
        thread_.join();
    }
};
