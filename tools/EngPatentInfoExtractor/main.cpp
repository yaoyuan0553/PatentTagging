//
// Created by yuan on 8/9/19.
//

#include "Shared.h"

using namespace std;

struct PublicationRefIdChildWalker : public pugi::xml_tree_walker {
    std::string innerText_;
public:
    bool for_each(pugi::xml_node& node) final
    {
        bool isDate = node.type() == pugi::node_element && strcmp(node.name(), tags::date) == 0;
        if (node.type() == pugi::node_element && !isDate)
            innerText_ += node.text().get();
        else if (isDate)
            innerText_ += '-' + string(node.text().get());

        return true;
    }
    const std::string& getInnerText() const { return innerText_; }
    void reset() { innerText_.clear(); }
};

struct PublicationRefIdNodeFilter : public TagNodeFilter {

    std::string operator()(pugi::xml_node& node) final
    {
        node.traverse(childWalker_);

        string content = childWalker_.getInnerText();

        childWalker_.reset();

        return content;
    }

    DEFINE_DEFAULT_CLONE(PublicationRefIdNodeFilter);

private:
    PublicationRefIdChildWalker childWalker_;
};

using ApplicationRefIdNodeFilter = PublicationRefIdNodeFilter;

struct EngPatentInfoFileOutput : public TagTextOutputFormatter {
    std::string operator()(const TagTextDict& tagTextDict) override
    {
        std::string output;
        for (const auto& [tag, texts] : tagTextDict) {
            if (tag == tags::filename)
                continue;
            for (const std::string& t : texts)
                output += t + ',';
            output.back() = '\t';
        }
        if (!output.empty())
            output.back() = '\n';

        return output;
    }

    DEFINE_DEFAULT_CLONE(EngPatentInfoFileOutput);
};

class EngPatentInfoExtractor : public XmlPCProcessorInterface {
    string pathFilename_;
    string outputFilename_;

    CQueue<string> filenameQueue_;
    unordered_map<string, CQueue<string>> outputQueueByFile_;

    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<PublicationRefIdNodeFilter>(tags::publication_reference);
        tagNodeFilterDict_.add<ApplicationRefIdNodeFilter>(tags::application_reference);
    }

    void prepareOutputFormatters() final
    {
        tagTextOutputFormatterDict_.add<EngPatentInfoFileOutput>(outputFilename_);
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
            consumers_.add<PatentInfoWriter>(filename, outputQueue);

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
    EngPatentInfoExtractor(std::string_view pathFilename,
                     std::string_view outputFilename, int nProducers) :
            pathFilename_(pathFilename), outputFilename_(outputFilename)
    { nProducers_ = nProducers; }
};


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t%s <path-file> <output-file> <nThreads>\n", program);
    exit(-1);
}


int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    EngPatentInfoExtractor engPatentInfoExtractor(argv[1], argv[2], atoi(argv[3]));

    engPatentInfoExtractor.process();

    return 0;
}
