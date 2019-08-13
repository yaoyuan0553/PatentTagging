//
// Created by yuan on 8/12/19.
//


#include "ThreadPool.h"

#include "TagConstants.h"
#include "FormatFunctors.h"
#include "TagNodeFilterFunctors.h"

#include "StatsThread.h"
#include "XmlFileReader.h"
#include "PatentTagTextCollector.h"

#include "XmlPCProcessorInterface.h"


using namespace std;
namespace fs = std::filesystem;

using CopyQueue = CQueue<pair<fs::path, fs::path>>;


class PatentXmlPathDistributor : public ThreadJob<> {
    XmlTagTextWalker walker_;

    CQueue<string>& filenameQueue_;
    CopyQueue& outputQueue_;

    FirstClassFirstSpaceOutput firstClassFirstSpaceOutput_;

    const int batchSize_;

    void internalRun() final
    {
        vector<pair<fs::path, fs::path>> batchOutput;
        batchOutput.reserve(batchSize_);
        for (int bN = 0;;)
        {
            pugi::xml_document doc;
            auto [filename, quit] = filenameQueue_.pop();

            if (quit) break;

            pugi::xml_parse_result result = doc.load_file(filename.c_str());
            if (!result)
                continue;

            walker_.reset();
            doc.traverse(walker_);

            batchOutput.emplace_back(pair<fs::path, fs::path>({
                filename,
                firstClassFirstSpaceOutput_(walker_.getTagTexts()) /
                    fs::path(filename).filename()
            }));

            if (++bN % batchSize_ == 0)
                addBatchToQueue(batchOutput);
        }
        if (!batchOutput.empty())
            addBatchToQueue(batchOutput);
    }

    void addBatchToQueue(vector<pair<fs::path, fs::path>>& batchOutput)
    {
        outputQueue_.push(batchOutput);
        batchOutput.clear();
    }

public:
    PatentXmlPathDistributor(CQueue<string>& filenameQueue, CopyQueue& outputQueue,
            TagNodeFilterDict& tagNodeFilterDict, int batchSize = 128) :
            walker_(tagNodeFilterDict), filenameQueue_(filenameQueue),
            outputQueue_(outputQueue), batchSize_(batchSize) { }
};


class PatentXmlCopyer : public ThreadJob<CopyQueue&> {
    fs::path rootDir_;
    unordered_set<string> createdDirs_;

    void internalRun(CopyQueue& output) final
    {
        for (;;)
        {
            auto [srcDist, quit] = output.pop();

            if (quit) break;

            fs::path outputPath = rootDir_ / srcDist.second;
            if (createdDirs_.insert(outputPath).second)
                fs::create_directory(outputPath);
            fs::copy_file(srcDist.first, outputPath);
        }
    }
public:
    PatentXmlCopyer(const fs::path& rootDir, CopyQueue& outputQueue) : ThreadJob(outputQueue),
        rootDir_(rootDir) { }
};




class DistributeXml : public XmlPCProcessorInterface {
    string pathFilename_;
    fs::path outputDir_;

    CQueue<string> filenameQueue_;
    CopyQueue outputQueue_;

    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<ClassificationNodeFilter>(tags::classification);
    }

    void prepareOutputFormatters() final
    {

    }

    void initializeData() final
    {
        XmlFileReader xmlFileReader(pathFilename_, filenameQueue_);
        xmlFileReader.runOnMain();

    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++)
            producers_.add<PatentXmlPathDistributor>(filenameQueue_,
                    outputQueue_, tagNodeFilterDict_);
        consumers_.add<PatentXmlCopyer>(outputDir_, outputQueue_);
    }

    void executeThreads() final
    {
        StatsThread<pair<fs::path, fs::path>, true> processedStats(outputQueue_,
                filenameQueue_.totalPushedItems());
        producers_.runAll();
        consumers_.runAll();
        processedStats.run();

        producers_.waitAll();
        outputQueue_.setQuitSignal();

        consumers_.waitAll();
        processedStats.wait();
    }


public:
    DistributeXml(std::string_view pathFilename,
                               const fs::path& outputDir, int nProducers) :
            pathFilename_(pathFilename), outputDir_(outputDir)
    { nProducers_ = nProducers; }
};


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file> <output-dir> <num-threads>\n", program);
    exit(-1);
}


int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    DistributeXml ccs(argv[1], argv[2], atoi(argv[3]));

    ccs.process();

    return 0;
}
