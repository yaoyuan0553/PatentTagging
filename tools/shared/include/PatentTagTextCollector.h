//
// Created by yuan on 7/30/19.
//

#pragma once
#ifndef TOOLS_PATENTTAGTEXTCOLLECTOR_H
#define TOOLS_PATENTTAGTEXTCOLLECTOR_H

#include <string>
#include <initializer_list>
#include <unordered_map>

#include <ConcurrentQueue.h>
#include "ThreadJob.h"
#include "Utility.h"
#include "XmlTagTextWalker.h"
#include "FormatFunctors.h"


class PatentTagTextCollector : public ThreadJob<> {
    using OutputQueueByFile = std::unordered_map<std::string, CQueue<std::string>>;
    using BatchOutputByFile = std::unordered_map<std::string, std::vector<std::string>>;

    XmlTagTextWalker walker_;

    CQueue<std::string>& filenameQueue_;
    OutputQueueByFile& outputQueueByFile_;

    TagTextOutputFormatterDict fileOutputFormatterDict_;

    const int batchSize_;

    void internalRun() final;

    void addBatchToQueue(BatchOutputByFile& batchOutputByFile);

public:
    PatentTagTextCollector(CQueue<std::string>& filenameQueue,
            OutputQueueByFile& outputQueueByFile,
            const TagTextOutputFormatterDict& fileOutputFormatterDict,
            const TagNodeFilterDict& tagNodeFilterDict,
            const int batchSize = 128) :
            walker_(tagNodeFilterDict),
            filenameQueue_(filenameQueue), outputQueueByFile_(outputQueueByFile),
            fileOutputFormatterDict_(fileOutputFormatterDict), batchSize_(batchSize) { }

};


#endif //TOOLS_PATENTTAGTEXTCOLLECTOR_H
