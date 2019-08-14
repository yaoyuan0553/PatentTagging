//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLIOREADERTHREAD_H
#define TOOLS_XMLIOREADERTHREAD_H


#include <pugixml.hpp>

#include "ThreadModelInterface.h"
#include "ConcurrentQueue.h"

class XmlIOReaderThread :
        public IOThreadInterface<CQueue<std::string>,
                CQueue<pugi::xml_document*>> {

    std::vector<pugi::xml_document*> batchOutput_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    XmlIOReaderThread(InputType& filenameQueue,
            OutputType& xmlDocQueue, int batchSize = 128) :
            InputOutputThreadInterface(filenameQueue,
                    xmlDocQueue, batchSize)
    {
        batchOutput_.reserve(batchSize_);
    }
};


#endif //TOOLS_XMLIOREADERTHREAD_H
