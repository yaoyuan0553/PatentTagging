//
// Created by yuan on 8/5/19.
//

#include "ThreadPool.h"

#include "TagConstants.h"
#include "FormatFunctors.h"
#include "TagNodeFilterFunctors.h"

#include "StatsThread.h"
#include "XmlFileReader.h"
#include "PatentTagTextCollector.h"
#include "ClassificationStatsWriter.h"

#include "XmlPCProcessorInterface.h"


using namespace std;

class CollectClassificationStats : public XmlPCProcessorInterface {
    string pathFilename_;
    string outputFilename_;

    CQueue<string> filenameQueue_;
    unordered_map<string, CQueue<string>> outputQueueByFile_;

    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<ClassificationNodeFilter>(tags::classification);
    }

    void prepareOutputFormatters() final
    {
        tagTextOutputFormatterDict_.add<FirstClassOutput>(outputFilename_);
    }

    void initializeData() final
    {
        XmlFileReader xmlFileReader(pathFilename_, filenameQueue_);
        xmlFileReader.runOnMain();

        outputQueueByFile_.emplace(piecewise_construct, make_tuple(outputFilename_), make_tuple());
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++)
            producers_.add<PatentTagTextCollector>(filenameQueue_, outputQueueByFile_,
                    tagTextOutputFormatterDict_, tagNodeFilterDict_, 1024);
        for (auto& [filename, outputQueue] : outputQueueByFile_)
            consumers_.add<ClassificationStatsWriter>(filename, outputQueue);

    }

    void executeThreads() final
    {
        StatsThread<string, false> processedStats(filenameQueue_);
        producers_.runAll();
        consumers_.runAll();
        processedStats.run();

        producers_.waitAll();
        for (auto& [_, outputQueue] : outputQueueByFile_)
            outputQueue.setQuitSignal();

        consumers_.waitAll();
        processedStats.wait();
    }


public:
    CollectClassificationStats(std::string_view pathFilename,
            std::string_view outputFilename, int nProducers) :
            pathFilename_(pathFilename), outputFilename_(outputFilename)
            { nProducers_ = nProducers; }
};


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file> <output-file> <num-threads>\n", program);
    exit(-1);
}


int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    CollectClassificationStats ccs(argv[1], argv[2], atoi(argv[3]));

    ccs.process();

    return 0;
}
