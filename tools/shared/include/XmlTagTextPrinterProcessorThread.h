//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTPRINTERPROCESSORTHREAD_H
#define TOOLS_XMLTAGTEXTPRINTERPROCESSORTHREAD_H

#include <vector>

#include <pugixml.hpp>

#include "ThreadJob.h"
#include "ThreadModelInterface.h"
#include "CQueue.h"
#include "XmlTagTextWalker.h"
#include "FormatFunctors.h"


class XmlTagTextPrinterProcessorThread :
    public InputOutputThreadInterface<CQueue<pugi::xml_document*>, CQueue<std::string*>> {

    XmlTagTextWalker            walker_;

    std::vector<std::string*>    batchOutput_;

    /* every tag needs a formatter otherwise it'll be
     * the first element of tagTextTag[tag][0] */
    TagTextOutputFormatterDict tagTextOutputFormatterDict_;

    void internalRun() final;

    inline void addBatchToQueue()
    {
        outputData_.push(batchOutput_);
        batchOutput_.clear();
    }

public:
    explicit XmlTagTextPrinterProcessorThread(CQueue<pugi::xml_document*>& inputQueue,
            CQueue<std::string*>& outputQueue,
            const TagNodeFilterDict& tagNodeFilterDict,
            const TagTextOutputFormatterDict& tagTextOutputFormatterDict,
            int batchSize = 128) :
        InputOutputThreadInterface(inputQueue, outputQueue, batchSize),
        walker_(tagNodeFilterDict),
        tagTextOutputFormatterDict_(tagTextOutputFormatterDict)
    {
        batchOutput_.reserve(batchSize_);
    }
};


#endif //TOOLS_XMLTAGTEXTPRINTERPROCESSORTHREAD_H
