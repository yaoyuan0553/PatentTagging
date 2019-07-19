//
// Created by yuan on 7/4/19.
//

#pragma once
#ifndef TOOLS_FILEDISTRIBUTOR_H
#define TOOLS_FILEDISTRIBUTOR_H

#include <vector>
#include <string>
#include <thread>
#include <fstream>

#include "ConcurrentQueue.h"
#include "PatentTagCollector.h"
#include "ThreadJob.h"

class FileDistributor : public ThreadJob {
    int batchSize_;
    std::string pathFilename_;

    ConcurrentQueue<std::string> filenameQueue_;

    void internalRun();

public:

    explicit FileDistributor(std::string pathFilename, int batchSize = 128):
        pathFilename_(std::move(pathFilename)), batchSize_(batchSize) { }

    void run();

    ConcurrentQueue<std::string>& filenameQueue() { return filenameQueue_; }
};


#endif //TOOLS_FILEDISTRIBUTOR_H
