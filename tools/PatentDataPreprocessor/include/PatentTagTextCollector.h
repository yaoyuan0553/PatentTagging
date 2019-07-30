//
// Created by yuan on 7/30/19.
//

#pragma once
#ifndef TOOLS_PATENTTAGTEXTCOLLECTOR_H
#define TOOLS_PATENTTAGTEXTCOLLECTOR_H

#include <string>
#include <initializer_list>

#include <ConcurrentQueue.h>
#include "ThreadJob.h"
#include "Utility.h"
#include "XmlTagTextWalker.h"


class PatentTagTextCollector : public ThreadJob<> {
    XmlTagTextWalker walker_;

    ConcurrentQueue<std::string>& filenameQueue_;
    ConcurrentQueue<FileOutput>& outputQueue_;

    static const std::initializer_list<char32_t> defaultSeparators_;
    ParagraphSplit paragraphSplit_;

    void internalRun() final;

    void processTagTexts(const TagTextDict& tagTextDict);

public:
    PatentTagTextCollector(ConcurrentQueue<std::string>& filenameQueue,
            ConcurrentQueue<FileOutput>& outputQueue,
            std::initializer_list<std::string> tags,
            std::initializer_list<char32_t> separators = defaultSeparators_) :
            walker_(std::move(tags)),
            filenameQueue_(filenameQueue), outputQueue_(outputQueue),
            paragraphSplit_(separators) { }
};


#endif //TOOLS_PATENTTAGTEXTCOLLECTOR_H
