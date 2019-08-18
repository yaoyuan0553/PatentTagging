//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLREADERTHREAD_H
#define TOOLS_XMLREADERTHREAD_H

#include <vector>

#include <pugixml.hpp>

#include "ThreadModelInterface.h"
#include "CQueue.h"

class XmlReaderThread :
        public OutputThreadInterface<CQueue<pugi::xml_document*>> {

    std::string pathFilename_;

    std::vector<pugi::xml_document*> batchOutput_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    XmlReaderThread(std::string_view pathFilename, OutputType& xmlDocQueue, int batchSize = 128) :
        OutputThreadInterface(xmlDocQueue, batchSize),
        pathFilename_(pathFilename)
    {
        batchOutput_.reserve(batchSize_);
    }
};


#endif //TOOLS_XMLREADERTHREAD_H
