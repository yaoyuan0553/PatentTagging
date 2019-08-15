//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTPRINTERPROCESSORXPATHTHREAD_H
#define TOOLS_XMLTAGTEXTPRINTERPROCESSORXPATHTHREAD_H

#include <vector>

#include <pugixml.hpp>

#include "ThreadJob.h"
#include "ThreadModelInterface.h"
#include "XpathQueryTextFormatter.h"
#include "CQueue.h"

class XmlTagTextPrinterProcessorXpathThread :
    public InputOutputThreadInterface<CQueue<pugi::xml_document*>, CQueue<std::string*>> {

    std::vector<std::string*> batchOutput_;

    XpathQueryTextFormatterDict xpathQueryTextFormatterDict_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    explicit XmlTagTextPrinterProcessorXpathThread(
            CQueue<pugi::xml_document*>& inputQueue,
            CQueue<std::string*>& outputQueue,
            const XpathQueryTextFormatterDict& xpathQueryTextFormatterDict,
            int batchSize = 128) :
            InputOutputThreadInterface(inputQueue, outputQueue, batchSize),
            xpathQueryTextFormatterDict_(xpathQueryTextFormatterDict)
    {
        batchOutput_.reserve(batchSize_);
    }

};

#endif //TOOLS_XMLTAGTEXTPRINTERPROCESSORXPATHTHREAD_H
