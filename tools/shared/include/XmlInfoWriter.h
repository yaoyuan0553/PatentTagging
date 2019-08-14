//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_XMLINFOWRITER_H
#define TOOLS_XMLINFOWRITER_H

#include <string>
#include <fstream>

#include <ConcurrentQueue.h>
#include <ThreadJob.h>


class XmlInfoWriter : public ThreadJob<ConcurrentQueue<std::string>&> {
    std::string filename_;

    void internalRun(ConcurrentQueue<std::string>& outputInfoQueue) override;

public:
    XmlInfoWriter(std::string_view filename, ConcurrentQueue<std::string>& outputInfoQueue) :
        ThreadJob(outputInfoQueue), filename_(filename) { }
};


#endif //TOOLS_XMLINFOWRITER_H
