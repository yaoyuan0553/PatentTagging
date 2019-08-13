//
// Created by yuan on 8/13/19.
//


#include <iostream>
#include <filesystem>

#include "Shared.h"

#include "NodeFilters.h"
#include "DataTypes.h"
#include "OutputFormatters.h"

using namespace std;
namespace fs = std::filesystem;


/* takes output data and write to disk every n GB */
/*
class CombineOutputWriter : public ThreadJob<> {

};
*/

class XmlTagTextPtrWalker : public pugi::xml_tree_walker {

    /* WARNING: memory released by caller */
    TagTextDict emptyTagTextDict_;
    TagTextDict* tagTextDict_;
    TagNodeFilterDict tagNodeFilterDict_;

public:

    explicit XmlTagTextPtrWalker(const TagNodeFilterDict& tagNodeFilterDict) :
        tagNodeFilterDict_(tagNodeFilterDict)
    {
        /* call reset before running traverse() on this walker */
        for (const std::string& tag : tagNodeFilterDict_.getKeys())
            emptyTagTextDict_[tag] = std::vector<std::string>();
    }

    bool for_each(pugi::xml_node& node) override
    {
        if (auto tagNode = tagTextDict_->find(std::string(node.name()));
                tagNode != tagTextDict_->end())
        {
            std::string filteredText = tagNodeFilterDict_[tagNode->first](node);
            if (!filteredText.empty())
                tagNode->second.emplace_back(filteredText);
        }

        return true;
    }

    TagTextDict* getTagTexts() { return tagTextDict_; }

    const TagTextDict* getTagTexts() const { return tagTextDict_; }

    void reset()
    {
        /* WARNING: old pointer should be released by caller */
        tagTextDict_ = new TagTextDict(emptyTagTextDict_);
    }
};


class PatentTagInfoStructCollector : ThreadJob<> {
    XmlTagTextPtrWalker walker_;

    CQueue<string>& filenameQueue_;

    /* WARNING: reader must release memory */
    CQueue<DataRecord*>& outputQueue_;

    DatabaseOutputFormatterDict databaseOutputFormatterDict;

    const int batchSize_;

    void internalRun() final
    {
        vector<DataRecord*> batchOutput;
        batchOutput.reserve(batchSize_);
        for (int bN = 0;;)
        {
            pugi::xml_document doc;
            auto [filename, quit] = filenameQueue_.pop();

            if (quit) break;

            pugi::xml_parse_result result = doc.load_file(filename.c_str());

            if (!result) continue;

            walker_.reset();
            doc.traverse(walker_);

            /* WARNING: to be released by caller */
            auto* singleOutput = new DataRecord;
            try {
                for (const auto& [tag, textVec] : *walker_.getTagTexts())
                    (*singleOutput)[tag] = databaseOutputFormatterDict[tag](textVec);
                batchOutput.push_back(singleOutput);
            }
            catch (range_error& e) {
                cerr << e.what() << '\n';
                cerr << "[" << filename << "]\n";
                continue;
            }

            delete walker_.getTagTexts();

            if (++bN % batchSize_ == 0)
                addBatchToQueue(batchOutput);
        }
        if (!batchOutput.empty())
            addBatchToQueue(batchOutput);
    }

    void addBatchToQueue(vector<DataRecord*>& batchOutput)
    {
        outputQueue_.push(batchOutput);
        batchOutput.clear();
    }

public:
    PatentTagInfoStructCollector(CQueue<string>& filenameQueue,
            CQueue<DataRecord*>& outputQueue, const TagNodeFilterDict& tagNodeFilterDict,
            const int batchSize = 128) :
            walker_(tagNodeFilterDict),
            filenameQueue_(filenameQueue), outputQueue_(outputQueue), batchSize_(batchSize)
    { }
};


struct OutputFileCollection {
    DataRecord dataRecord;
    
};


class DatabaseFileWriter : public ThreadJob<CQueue<DataRecord*>&> {
    inline static constexpr char DEFAULT_FILE_PREFIX[] = "patent-data";

    fs::path rootDir_;
    const int recordsPerFile_;

    fs::path filename_ = DEFAULT_FILE_PREFIX;
    int fileNo = 0;

    unordered_map<string, string> fileBuffer_;

    vector<string> tagsToWrite_;

    void internalRun(CQueue<DataRecord*>& outputQueue) final
    {
        for (int i = 0;;)
        {
            auto [record, quit] = outputQueue.pop();

            if (quit) break;

            for (const string& tag : tagsToWrite_) {

                fileBuffer_[tag] += (*record)[tag];
            }
        }
    }

public:
    DatabaseFileWriter(const fs::path& rootDir, CQueue<DataRecord*>& outputQueue,
            const vector<string>& tagsToWrite, const int recordsPerFile = 20000) :
        ThreadJob(outputQueue), rootDir_(rootDir), recordsPerFile_(recordsPerFile),
        tagsToWrite_(tagsToWrite) { }
};


class GenerateDatabase : public XmlPCProcessorInterface {
    string pathFilename_;
    string outputFilename_;

    CQueue<string> filenameQueue_;
    unordered_map<string, CQueue<string>> outputQueueByFile_;

    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<IdNodeFilter>(tags::publication_reference);
        tagNodeFilterDict_.add<IdNodeFilter>(tags::application_reference);
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
/*
        for (auto& [filename, outputQueue] : outputQueueByFile_)
            consumers_.add<>(filename, outputQueue);
*/

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
    GenerateDatabase(std::string_view pathFilename,
            std::string_view outputFilename, int nProducers) :
            pathFilename_(pathFilename), outputFilename_(outputFilename)
    { nProducers_ = nProducers; }
};

void testDataType()
{
    cout << sizeof(IndexValue) << endl;
    cout << IndexValue::INDEX_VALUE_STATIC_SIZE << endl;

    IndexValue idxVal1;
    idxVal1.datId = 32;
    strcpy(idxVal1.publicationId, "US20140026733A1-20140130");
    strcpy(idxVal1.applicationId, "US13945385-20130718");
    idxVal1.ti = 0; idxVal1.ai = 0; idxVal1.ci = 0; idxVal1.di = 0;
    idxVal1.classCount = 2;

    ClassificationString c1, c2;
    strcpy(c1.data(), "B01F-3/04-(2006.01)");
    strcpy(c2.data(), "C02F-1/44-(2006.01)");
    idxVal1.classifications().push_back(c1);
    idxVal1.classifications().push_back(c2);

    cout << idxVal1.getTotalBytes() << endl;

    char* buffer = new char[200];

    int sizeWritten = 0;
    if (!idxVal1.save(buffer, 200, &sizeWritten))
        cout << "what\n";

    IndexValue idxVal2;
    idxVal2.load(buffer);

    printf("what\n");

    delete[] buffer;
}


int main()
{
    testDataType();

    return 0;
}