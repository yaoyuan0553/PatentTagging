//
// Created by yuan on 8/5/19.
//

#pragma once
#ifndef TOOLS_CLASSIFICATIONSTATSWRITER_H
#define TOOLS_CLASSIFICATIONSTATSWRITER_H

#include <string>
#include <string_view>
#include <unordered_map>

#include <ThreadJob.h>
#include <ConcurrentQueue.h>


class ClassificationStatsWriter : ThreadJob<CQueue<std::string>&> {
    std::string filename_;
    std::unordered_map<std::string, uint64_t> countByTag_;

    void internalRun(CQueue<std::string>& data) override;
public:
    explicit ClassificationStatsWriter(std::string_view filename, CQueue<std::string>& data) :
        ThreadJob(data), filename_(filename) { }
};


#endif //TOOLS_CLASSIFICATIONSTATSWRITER_H
