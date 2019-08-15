//
// Created by yuan on 8/14/19.
//

#pragma once
#ifndef TOOLS_XMLIPOTAGTEXTPRINTERTESTER_H
#define TOOLS_XMLIPOTAGTEXTPRINTERTESTER_H

#include "XmlFilterOutputIPOInterface.h"
#include "XmlReaderThread.h"
#include "XmlTagTextPrinterProcessorThread.h"
#include "XmlTagTextPrinterWriterThread.h"
#include "StatsThread.h"
#include "XmlPathFileReader.h"
#include "XmlIOReaderThread.h"


class XmlIPOTagTextPrinterTester : public XmlFilterOutputIPOInterface {
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
        readerPool_.add<XmlIOReaderThread>(nReaders_, filenameQueue_, xmlDocQueue_, 128);

        processorPool_.add<XmlTagTextPrinterProcessorThread>(nProcessors_, xmlDocQueue_,
                outputStringQueue_, tagNodeFilterDict_, tagTextOutputFormatterDict_, 128);

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

    XmlIPOTagTextPrinterTester(std::string_view pathFilename,
            std::string_view outputFilename, int nReaders, int nProcessors) :
            XmlFilterOutputIPOInterface(nReaders, nProcessors),
            pathFilename_(pathFilename), outputFilename_(outputFilename) { }
};


#endif //TOOLS_XMLIPOTAGTEXTPRINTERTESTER_H
