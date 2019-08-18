//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_XMLBUFFERTAGTEXTPRINTERPROCESSORTHREAD_H
#define TOOLS_XMLBUFFERTAGTEXTPRINTERPROCESSORTHREAD_H

#include "ThreadModelInterface.h"
#include "CQueue.h"
#include "XpathQueryTextFormatter.h"

class XmlBufferTagTextPrinterProcessorThread :
        public InputOutputThreadInterface<CQueue<std::pair<char*, size_t>>,
                CQueue<std::string*>> {

    std::vector<std::string*> batchOutput_;

    XpathQueryTextFormatterDict xpathQueryTextFormatterDict_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    explicit XmlBufferTagTextPrinterProcessorThread(
            CQueue<std::pair<char*, size_t>>& inputQueue,
            CQueue<std::string*>& outputQueue,
            const XpathQueryTextFormatterDict& xpathQueryTextFormatterDict,
            int batchSize = 128) :
            InputOutputThreadInterface(inputQueue, outputQueue, batchSize),
            xpathQueryTextFormatterDict_(xpathQueryTextFormatterDict)
    {
        batchOutput_.reserve(batchSize_);
    }
};


#endif //TOOLS_XMLBUFFERTAGTEXTPRINTERPROCESSORTHREAD_H
