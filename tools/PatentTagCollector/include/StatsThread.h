//
// Created by yuan on 7/5/19.
//

#pragma once
#ifndef TOOLS_STATSTHREAD_H
#define TOOLS_STATSTHREAD_H

#include <string>
#include "ThreadJob.h"
#include "ConcurrentQueue.h"
#include "FileDistributor.h"


class StatsThread : public ThreadJob {
    void internalRun(ConcurrentQueue<std::string>& dataQueue);

public:
    void run(ConcurrentQueue<std::string>& dataQueue);
};


#endif //TOOLS_STATSTHREAD_H
