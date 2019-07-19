//
// Created by yuan on 7/4/19.
//

#include <iostream>
#include "ThreadJob.h"


void ThreadJob::wait()
{
    if (!threadStarted_) {
        std::cerr << "ThreadJob must be started before wait on\n";
        exit(-1);
    }
    if (!thread_.joinable()) {
        std::cerr << "thread not joinable!\n";
        exit(-1);
    }
    thread_.join();
}
