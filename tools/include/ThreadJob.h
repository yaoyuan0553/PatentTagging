//
// Created by yuan on 7/20/19.
//

#ifndef TOOLS_THREADJOB_H
#define TOOLS_THREADJOB_H

#endif //TOOLS_THREADJOB_H

#include <thread>
#include <iostream>
#include <utility>
#include <type_traits>


template<typename T>
auto return_ref_if_ref(T&& t)
{
    if constexpr (std::is_reference_v<T>)
        return std::ref(t);
    else
        return std::forward<T>(t);
}

/* A joinable class wrapper for std::thread
 * this base class should be subclassed
 * forbidding any construction without subclasses
 * */
template <typename... RunArgs>
class ThreadJob {
protected:
    std::thread thread_;
    bool threadStarted_ = false;

    /* abstract method, must be implemented by subclasses
     * main execution routine for the current thread
     * upon starting of the thread, this function will execute*/
    virtual void internalRun(RunArgs...) = 0;

    ThreadJob() = default;

public:
    /* virtual run function with default values, can be override in subclasses.
     * calling this function spawns thread and starts execution */
    virtual void run(RunArgs&&... runArgs)
    {
        thread_ = std::thread(&ThreadJob::internalRun, this,
                (typename std::conditional<std::is_reference_v<RunArgs>,
                        std::reference_wrapper<RunArgs>, RunArgs>)runArgs...);
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
