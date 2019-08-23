//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_FILEREADERTHREAD_H
#define TOOLS_FILEREADERTHREAD_H


#include <pugixml.hpp>

#include "ThreadModelInterface.h"
#include "CQueue.h"
#include "XmlFile.h"


class FileReaderThread :
        public IOThreadInterface<ConcurrentQueue<std::string>,
                CQueue<XmlFile>> {
    std::vector<XmlFile> batchOutput_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    FileReaderThread(InputType& filenameQueue,
            OutputType& xmlFileQueue, int batchSize = 128) :
            InputOutputThreadInterface(filenameQueue, xmlFileQueue, batchSize)
    {
        batchOutput_.reserve(batchSize_);
    }
};


#endif //TOOLS_FILEREADERTHREAD_H
