//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLXPATHIPOTAGTEXTPRINTERTESTER_H
#define TOOLS_XMLXPATHIPOTAGTEXTPRINTERTESTER_H

#include "XmlXpathIPOInterface.h"
#include "XmlPathFileReader.h"
#include "XmlIOReaderThread.h"
#include "XmlTagTextPrinterWriterThread.h"
#include "StatsThread.h"
#include "XmlTagTextPrinterProcessorXpathThread.h"

class XmlXpathIPOTagTextPrinterTester : public XmlXpathIPOInterface {
protected:
    std::string pathFilename_;
    std::string outputFilename_;

    ConcurrentQueue<std::string> filenameQueue_;
    CQueue<pugi::xml_document*> xmlDocQueue_;
    CQueue<std::string*> outputStringQueue_;

    void initializeData() override
    {
        XmlPathFileReader xmlPathFileReader(pathFilename_, filenameQueue_);
        xmlPathFileReader.runOnMain();
    }

    void initializeThreads() final
    {
//        for (int i = 0; i < nReaders_; i++)
        readerPool_.add<XmlIOReaderThread>(nReaders_, filenameQueue_, xmlDocQueue_, 1);

        processorPool_.add<XmlTagTextPrinterProcessorXpathThread>(nProcessors_, xmlDocQueue_,
                outputStringQueue_, xpathQueryTextFormatterDict_, 1);

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

    XmlXpathIPOTagTextPrinterTester(std::string_view pathFilename,
            std::string_view outputFilename, int nReaders, int nProcessors) :
            XmlXpathIPOInterface(nReaders, nProcessors),
            pathFilename_(pathFilename), outputFilename_(outputFilename) { }
};

#endif //TOOLS_XMLXPATHIPOTAGTEXTPRINTERTESTER_H
