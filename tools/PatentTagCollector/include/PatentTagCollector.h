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


class PatentTagCollector : public ThreadJob<ConcurrentQueue<std::string>&> {
    friend std::thread;

    PatentTagWalker walker_;

    std::vector<std::string> errorFiles_;

    void internalRun(ConcurrentQueue<std::string>& filenameQueue) override;

public:
    explicit PatentTagCollector(ConcurrentQueue<std::string>& filenameQueue) :
        ThreadJob(filenameQueue) { }

    PatentTagCollector(PatentTagCollector&&) = default;

    auto& uniqueTags() { return walker_.uniqueTags; }
};


#endif //TOOLS_PATENTTAGCOLLECTOR_H
