//
// Created by yuan on 7/4/19.
//
#pragma once

#ifndef TOOLS_PATENTTAGCOLLECTOR_H
#define TOOLS_PATENTTAGCOLLECTOR_H

#include <type_traits>
#include <pugixml.hpp>
#include <thread>

#include "ConcurrentQueue.h"
#include "PatentTagWalker.h"
#include "ThreadJob.h"


class PatentTagCollector : public ThreadJob {
    friend std::thread;

    pugi::xml_document doc_;
    PatentTagWalker walker_;

    std::vector<std::string> errorFiles_;

    void internalRun(ConcurrentQueue<std::string>& filenameQueue);

public:
    PatentTagCollector() = default;

    void run(ConcurrentQueue<std::string>& filenameQueue);
};


#endif //TOOLS_PATENTTAGCOLLECTOR_H
