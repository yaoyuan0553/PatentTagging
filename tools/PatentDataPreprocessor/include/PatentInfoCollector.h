//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_PATENTINFOCOLLECTOR_H
#define TOOLS_PATENTINFOCOLLECTOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <type_traits>

#include <pugixml.hpp>

#include "ThreadJob.h"
#include "ConcurrentQueue.h"
#include "PatentTagAbstractWalker.h"


class PatentInfoCollector :public ThreadJob<ConcurrentQueue<std::string>&, ConcurrentQueue<std::string>&> {

    PatentTagAbstractWalker walker_;


    std::vector<std::string> errorFiles_;

    int batchSize_;
    std::vector<std::string> batchInfo_;

    void internalRun(ConcurrentQueue<std::string>& filenameQueue,
            ConcurrentQueue<std::string>& outputInfoQueue) override;

public:
    explicit PatentInfoCollector(ConcurrentQueue<std::string>& filenameQueue,
            ConcurrentQueue<std::string>& outputInfoQueue, int batchSize = 128) :
        ThreadJob(filenameQueue, outputInfoQueue), batchSize_(batchSize)
        { batchInfo_.reserve(batchSize_); }
};


#endif //TOOLS_PATENTINFOCOLLECTOR_H
