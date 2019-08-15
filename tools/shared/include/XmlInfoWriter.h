//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_XMLINFOWRITER_H
#define TOOLS_XMLINFOWRITER_H

#include <string>
#include <fstream>

#include <CQueue.h>
#include <ThreadJob.h>


class XmlInfoWriter : public ThreadJob<CQueue<std::string>&> {
    std::string filename_;

    void internalRun(CQueue<std::string>& outputInfoQueue) override;

public:
    XmlInfoWriter(std::string_view filename, CQueue<std::string>& outputInfoQueue) :
        ThreadJob(outputInfoQueue), filename_(filename) { }
};


#endif //TOOLS_XMLINFOWRITER_H
