//
// Created by yuan on 8/6/19.
//

#include <iostream>
#include <string>

#include <pugixml.hpp>

#include "XmlPCProcessorInterface.h"
#include "XmlFileReader.h"
#include "PatentTagTextCollector.h"
#include "StatsThread.h"
#include "TagNodeFilterFunctors.h"
#include "XmlInfoWriter.h"


using namespace std;


struct AllTextNodeFilter : public TagNodeFilter {

    std::string operator()(pugi::xml_node& node) final
    {
        node.traverse(childWalker_);

        string content = childWalker_.getInnerText();

        childWalker_.reset();

        return content;
    }

    DEFINE_DEFAULT_CLONE(AllTextNodeFilter);

private:
    ExhaustiveChildWalker childWalker_;
};


class SplitAllTextFileOutput : public TagTextOutputFormatter {
    SplitParagraph splitParagraph_;
public:
    std::string operator()(const TagTextDict& tagTextDict) override
    {
        std::string output;
        for (const auto& [tag, texts] : tagTextDict) {
            if (tag == tags::filename)
                continue;
            for (const std::string& t : texts)
                output += splitParagraph_(RemoveExtraWhitespace(t), '\n');
        }
        if (!output.empty())
            output += "\n\n";

        return output;
    }

    DEFINE_DEFAULT_CLONE(SplitAllTextFileOutput);
};



class AllTextExtractor : public XmlPCProcessorInterface {
    string pathFilename_;
    string outputFilename_;

    CQueue<string> filenameQueue_;
    unordered_map<string, CQueue<string>> outputQueueByFile_;

    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<AllTextNodeFilter>(tags::abstract);
        tagNodeFilterDict_.add<AllTextNodeFilter>(tags::description);
        tagNodeFilterDict_.add<AllTextNodeFilter>(tags::claims);
    }

    void prepareOutputFormatters() final
    {
        tagTextOutputFormatterDict_.add<SplitAllTextFileOutput>(outputFilename_);
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
                                                   tagTextOutputFormatterDict_, tagNodeFilterDict_, 256);
        for (auto& [filename, outputQueue] : outputQueueByFile_)
            consumers_.add<XmlInfoWriter>(filename, outputQueue);

    }

    void executeThreads() final
    {
        StatsThread<string, true> writeStats(outputQueueByFile_[outputFilename_],
                                             filenameQueue_.totalPushedItems());
        producers_.runAll();
        consumers_.runAll();
        writeStats.run();

        producers_.waitAll();
        for (auto& [_, outputQueue] : outputQueueByFile_)
            outputQueue.setQuitSignal();

        consumers_.waitAll();
        writeStats.wait();
    }


public:
    AllTextExtractor(std::string_view pathFilename,
            std::string_view outputFilename, int nProducers) :
            pathFilename_(pathFilename), outputFilename_(outputFilename)
    { nProducers_ = nProducers; }
};


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t%s <path-file> <output-file> <num-threads>\n", program);
    exit(-1);
}


int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    AllTextExtractor ate(argv[1], argv[2], atoi(argv[3]));

    ate.process();


    return 0;
}
