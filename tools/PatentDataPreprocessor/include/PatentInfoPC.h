//
// Created by yuan on 7/26/19.
//

#pragma once
#ifndef TOOLS_PATENTINFOPC_H
#define TOOLS_PATENTINFOPC_H

#include <string>
#include <string_view>

#include <ThreadPool.h>
#include <StatsThread.h>

#include "ConcurrentQueue.h"
#include "XmlFileReader.h"


class PatentInfoPC : public PCThreadPool<> {
    ConcurrentQueue<std::string> filenameQueue_;
    ConcurrentQueue<std::string> outputInfoQueue_;
    ConcurrentQueue<std::string> splitAbstractQueue_;

    XmlFileReader xmlFileReader_;
    StatsThread<std::string, true>* statsThread_;

    std::string infoOutputFilename_;
    std::string splitAbstractOutputFilename_;

    int nProducers_;

    void initThreads();

public:

    explicit PatentInfoPC(const std::string& pathFilename,
            std::string_view infoOutputFilename,
            std::string_view splitAbstractOutputFilename, int nProducers) :
        xmlFileReader_(pathFilename, filenameQueue_),
        infoOutputFilename_(infoOutputFilename),
        splitAbstractOutputFilename_(splitAbstractOutputFilename), nProducers_(nProducers)
    {
        initThreads();
    }

    ~PatentInfoPC()
    {
        delete statsThread_;
    }

    void runAll() override;

    void waitAll() override;
};




#endif //TOOLS_PATENTINFOPC_H
