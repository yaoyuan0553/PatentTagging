//
// Created by yuan on 8/14/19.
//

#pragma once
#ifndef TOOLS_XMLPCPROCESSORTESTER_H
#define TOOLS_XMLPCPROCESSORTESTER_H


#include "XmlPCProcessorInterface.h"
#include "PatentTagTextCollector.h"
#include "StatsThread.h"
#include "XmlPathFileReader.h"
#include "XmlInfoWriter.h"


class TestTagTextExtractor : public ThreadJob<> {
    XmlTagTextWalker walker_;

    ConcurrentQueue<std::string>& filenameQueue_;
    CQueue<std::string>& outputQueue_;

    /* every tag needs a formatter otherwise it'll be
     * the first element of tagTextTag[tag][0] */
    TagTextOutputFormatterDict fileOutputFormatterDict_;

    const int batchSize_;

    void internalRun() final;

    void addBatchToQueue(std::vector<std::string>& batchOutput);

public:
    TestTagTextExtractor(ConcurrentQueue<std::string>& filenameQueue,
            CQueue<std::string>& outputQueue,
            const TagTextOutputFormatterDict& fileOutputFormatterDict,
            const TagNodeFilterDict& tagNodeFilterDict,
            const int batchSize = 128) :
            walker_(tagNodeFilterDict),
            filenameQueue_(filenameQueue), outputQueue_(outputQueue),
            fileOutputFormatterDict_(fileOutputFormatterDict), batchSize_(batchSize) { }

};

class XmlPCProcessorTester : public XmlPCProcessorInterface {
protected:
    std::string pathFilename_;
    std::string outputFilename_;

    ConcurrentQueue<std::string> filenameQueue_;
    CQueue<std::string> outputQueue_;

    /* user of this class must implement
     * prepareNodeFilters() and
     * prepareOutputFormatters() */

    void initializeData() final
    {
        XmlPathFileReader xmlPathFileReader(pathFilename_, filenameQueue_);
        xmlPathFileReader.runOnMain();
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++)
            producers_.add<TestTagTextExtractor>(filenameQueue_, outputQueue_,
                    tagTextOutputFormatterDict_, tagNodeFilterDict_, 256);
        consumers_.add<XmlInfoWriter>(outputFilename_, outputQueue_);
    }

    void executeThreads() final
    {
        StatsThread<std::string, true> processedStats(outputQueue_,
                filenameQueue_.totalPushedItems());
        producers_.runAll();
        consumers_.runAll();
        processedStats.run();

        producers_.waitAll();
        outputQueue_.setQuitSignal();

        consumers_.waitAll();
        processedStats.wait();
    }

    XmlPCProcessorTester(std::string_view pathFilename,
            std::string_view outputFilename, int nProducers) :
            pathFilename_(pathFilename), outputFilename_(outputFilename)
    { nProducers_ = nProducers; }
};


#endif //TOOLS_XMLPCPROCESSORTESTER_H
