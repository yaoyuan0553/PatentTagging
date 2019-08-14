//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTPRINTERWRITERTHREAD_H
#define TOOLS_XMLTAGTEXTPRINTERWRITERTHREAD_H

#include <string>

#include "ThreadJob.h"
#include "ThreadModelInterface.h"
#include "ConcurrentQueue.h"

class XmlTagTextPrinterWriterThread :
        public InputThreadInterface<CQueue<std::string>> {

    std::string filename_;

    void internalRun() final;

public:
    XmlTagTextPrinterWriterThread(std::string_view filename, CQueue<std::string>& textQueue) :
        InputThreadInterface(textQueue), filename_(filename) { }
};


#endif //TOOLS_XMLTAGTEXTPRINTERWRITERTHREAD_H
