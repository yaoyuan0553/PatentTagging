//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_PATENTINFOWRITER_H
#define TOOLS_PATENTINFOWRITER_H

#include <string>
#include <fstream>

#include <ConcurrentQueue.h>
#include <ThreadJob.h>


class PatentInfoWriter : public ThreadJob<ConcurrentQueue<std::string>&> {
    std::string filename_;

    void internalRun(ConcurrentQueue<std::string>& outputInfoQueue) override;

public:
    PatentInfoWriter(std::string filename, ConcurrentQueue<std::string>& outputInfoQueue) :
        ThreadJob(outputInfoQueue), filename_(std::move(filename)) { }
};


#endif //TOOLS_PATENTINFOWRITER_H
