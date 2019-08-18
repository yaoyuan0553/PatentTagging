//
// Created by yuan on 7/24/19.
//
#pragma once
#ifndef TOOLS_XMLFILEREADER_H
#define TOOLS_XMLFILEREADER_H

#endif //TOOLS_XMLFILEREADER_H

#include <string>
#include <pugixml.hpp>

#include "ThreadJob.h"
#include "CQueue.h"


class XmlPathFileReader : public ThreadJob<> {
    int batchSize_;
    std::string pathFilename_;

    ConcurrentQueue<std::string>& filenameQueue_;

    void internalRun() override;

public:
    XmlPathFileReader(std::string pathFilename, ConcurrentQueue<std::string>& filenameQueue,
            int batchSize = 128):
            batchSize_(batchSize), pathFilename_(std::move(pathFilename)),
            filenameQueue_(filenameQueue) { }
};
