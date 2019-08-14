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

#define DEBUG 1


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

            DataRecord* singleOutput = walker_.getTagTexts();
            try {
                batchOutput.push_back(singleOutput);
            }
            catch (range_error& e) {
                cerr << e.what() << '\n';
                cerr << "[" << filename << "]\n";
                continue;
            }

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
    inline static constexpr int RECORDS_PER_FILE = 20000;
    inline static constexpr uint32_t MAX_FILE_SIZE = 1 << 30;   // 1 GB file size limit

    fs::path rootDir_;
    const int recordsPerFile_;

    fs::path binFilename_ = DEFAULT_FILE_PREFIX;
    int binNo_ = 0;

    /* two kinds of entry (pid & aid) to find index entries */
    IndexTable pidIndexTable_, aidIndexTable_;
    /* where real index value stores memory released on class
     * destruction */
    vector<IndexValue*> indexValueList;

    vector<string> tagsToWrite_;

    /* for formatting tag text writing to the data file */
    DatabaseOutputFormatterDict databaseOutputFormatterDict_;

    /* data file buffer */
    DataRecordFile dataRecordFile_;
    uint32_t nRecords = 0;

    bool threadRan = false;

    void internalRun(CQueue<DataRecord*>& outputQueue) final
    {
        for (;;)
        {
            auto [record, quit] = outputQueue.pop();

            if (quit) break;

            appendToIndexTable(record);
            appendToDataRecordFile(record);

            // release memory of DataRecord
            delete record;
        }
        threadRan = true;
    }


    void appendToIndexTable(DataRecord* record)
    {
        indexValueList.push_back(new IndexValue);

        indexValueList.back()->pid = record->at(tags::publication_reference)[0];
        indexValueList.back()->aid = record->at(tags::application_reference)[0];
//         TODO: appDate
//        indexValueList.back()->appDate = record->()
        indexValueList.back()->binId = binNo_;
        indexValueList.back()->ipcList = record->at(tags::classification);
        // TODO: ti, ai, ci, di, offset
    }

    void appendToDataRecordFile(DataRecord* record)
    {
        uint32_t recordSize = sizeof(uint32_t) + sizeof(uint32_t) * tagsToWrite_.size();
        vector<string> tagFormattedText;
        tagFormattedText.reserve(tagsToWrite_.size());
        for (const auto& tag : tagsToWrite_) {
            tagFormattedText.push_back(databaseOutputFormatterDict_[tag](record->at(tag)));
            recordSize += tagFormattedText.back().length();
        }
        if (!dataRecordFile_.appendRecord(recordSize, tagFormattedText)) {
            fs::path binFilename = rootDir_ / binFilename_;
            binFilename += to_string(binNo_) + ".bin";
#ifdef DEBUG
            cout << binFilename << '\n';
#endif
            // synchronized version. in async, there should be multiple DataRecordFile
            // to avoid clearing buffer before I/O finishes
            dataRecordFile_.writeToFile(binFilename.c_str());
            dataRecordFile_.reset();
            binNo_++;
            if (!dataRecordFile_.appendRecord(recordSize, tagFormattedText)) {
                fprintf(stderr, "%s: something is wrong here %d\n", __FUNCTION__, __LINE__);
                exit(-1);
            }
        }
        nRecords++;
    }

public:
    DatabaseFileWriter(const fs::path& rootDir, CQueue<DataRecord*>& outputQueue,
            const vector<string>& tagsToWrite, const int recordsPerFile = RECORDS_PER_FILE) :
        ThreadJob(outputQueue), rootDir_(rootDir), recordsPerFile_(recordsPerFile),
        tagsToWrite_(tagsToWrite) { }

    ~DatabaseFileWriter() final
    {
        if (threadRan) {
            for (IndexValue* iv : indexValueList)
                delete iv;
        }
    }
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





int main()
{


    return 0;
}