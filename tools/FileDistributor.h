//
// Created by yuan on 7/4/19.
//

#pragma once
#ifndef TOOLS_FILEDISTRIBUTOR_H
#define TOOLS_FILEDISTRIBUTOR_H

#include <vector>
#include <string>
#include <thread>

#include "ConcurrentQueue.h"
#include "PatentTagCollector.h"
#include "ThreadJob.h"

class FileDistributor : public ThreadJob {
    int batchSize;

    ConcurrentQueue<std::string> filenameQueue_;

    void internalRun();

public:

    explicit FileDistributor(int batchSize = 128):
        batchSize(batchSize) { }

    void run();

    ConcurrentQueue<std::string>& filenameQueue() { return filenameQueue_; }
};


#endif //TOOLS_FILEDISTRIBUTOR_H
