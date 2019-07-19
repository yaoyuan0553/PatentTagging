//
// Created by yuan on 7/4/19.
//

#pragma once
#ifndef TOOLS_THREADJOB_H
#define TOOLS_THREADJOB_H

#include <thread>


class ThreadJob {
protected:

    std::thread thread_;
    bool threadStarted_ = false;

    ThreadJob() = default;

public:

    ThreadJob(const ThreadJob&) = delete;

    virtual void wait();
};


#endif //TOOLS_THREADJOB_H
