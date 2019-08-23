//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_XMLBUFFERXPATHIPOTAGTEXTPRINTERTESTER_H
#define TOOLS_XMLBUFFERXPATHIPOTAGTEXTPRINTERTESTER_H

#include "XmlXpathIPOInterface.h"
#include "CQueue.h"
#include "XmlPathFileReader.h"
#include "FileReaderThread.h"
#include "XmlBufferTagTextPrinterProcessorThread.h"
#include "XmlTagTextPrinterWriterThread.h"
#include "StatsThread.h"


class XmlBufferXpathIPOTagTextPrinterTester : public XmlXpathIPOInterface {
protected:
    std::string pathFilename_;
    std::string outputFilename_;

    ConcurrentQueue<std::string> filenameQueue_;
    CQueue<XmlFile> xmlDocQueue_;
    CQueue<std::string*> outputStringQueue_;

    void initializeData() override
    {
        XmlPathFileReader xmlPathFileReader(pathFilename_, filenameQueue_);
        xmlPathFileReader.runOnMain();
    }

    void initializeThreads() final
    {
//        for (int i = 0; i < nReaders_; i++)
        readerPool_.add<FileReaderThread>(nReaders_, filenameQueue_, xmlDocQueue_, 128);

        processorPool_.add<XmlBufferTagTextPrinterProcessorThread>(
                nProcessors_, xmlDocQueue_,
                outputStringQueue_, xpathQueryTextFormatterDict_, 128);

        writerPool_.add<XmlTagTextPrinterWriterThread>(outputFilename_, outputStringQueue_);
    }

    void executeThreads() final
    {
        StatsThread<std::string*, true> processedStats(outputStringQueue_,
                                                       filenameQueue_.totalPushedItems());

        readerPool_.runAll();
        processorPool_.runAll();
        writerPool_.runAll();
        processedStats.run();

        readerPool_.waitAll();
        xmlDocQueue_.setQuitSignal();

        processorPool_.waitAll();
        outputStringQueue_.setQuitSignal();

        writerPool_.waitAll();
        processedStats.wait();
    }

    XmlBufferXpathIPOTagTextPrinterTester(std::string_view pathFilename,
            std::string_view outputFilename, int nReaders, int nProcessors) :
            XmlXpathIPOInterface(nReaders, nProcessors),
            pathFilename_(pathFilename), outputFilename_(outputFilename) { }
};

#endif //TOOLS_XMLBUFFERXPATHIPOTAGTEXTPRINTERTESTER_H
