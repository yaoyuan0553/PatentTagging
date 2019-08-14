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

    TagTextDict emptyTagTextDict_;
    /* WARNING: memory released by caller */
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
    IndexTableWithSpecificKey pidIndexTable_, aidIndexTable_;
    /* where real index value stores memory released on class
     * destruction */
    vector<IndexValue*> indexValueList;
    IndexTable& indexTable_;

    vector<string> tagsToWrite_;

    /* for formatting tag text writing to the data file */
    DatabaseOutputFormatterDict databaseOutputFormatterDict_;

    /* data file buffer */
    DataRecordFile dataRecordFile_;
    uint32_t nRecords = 0;

    void internalRun(CQueue<DataRecord*>& outputQueue) final
    {
        for (;;)
        {
            auto [record, quit] = outputQueue.pop();

            if (quit) break;

            appendToIndexTable(record);
            appendToDataRecordFile(record);

            // release memory of DataRecord already copied/processed data
            // of this record and add them into index table & data file
            delete record;
        }
    }


    void appendToIndexTable(DataRecord* record)
    {
        indexTable_.appendIndexValue([&](IndexValue* iv) {
            iv->pid = record->at(tags::publication_reference)[0];
            iv->aid = record->at(tags::application_reference)[0];
//         TODO: appDate
//        indexValueList.back()->appDate = record->()
            iv->binId = binNo_;
            iv->ipcList = record->at(tags::classification);
        });
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
        if (!dataRecordFile_.appendRecord(tagFormattedText, recordSize)) {
            fs::path binFilename = rootDir_ / binFilename_;
            binFilename += to_string(binNo_) + ".bin";
#ifdef DEBUG
            cout << binFilename << '\n';
#endif
            // synchronized version. in async, there should be multiple DataRecordFile
            // to avoid clearing buffer before I/O finishes
            dataRecordFile_.writeToFile(binFilename.c_str());
            dataRecordFile_.clear();
            binNo_++;
            if (!dataRecordFile_.appendRecord(tagFormattedText, recordSize)) {
                fprintf(stderr, "%s: something is wrong here %d\n", __FUNCTION__, __LINE__);
                exit(-1);
            }
        }
        nRecords++;
    }

public:
    DatabaseFileWriter(const fs::path& rootDir, CQueue<DataRecord*>& outputQueue,
            const vector<string>& tagsToWrite, IndexTable& indexTable,
            const int recordsPerFile = RECORDS_PER_FILE) :
        ThreadJob(outputQueue), rootDir_(rootDir), recordsPerFile_(recordsPerFile),
        indexTable_(indexTable), tagsToWrite_(tagsToWrite) { }

    ~DatabaseFileWriter() final
    {
        for (IndexValue* iv : indexValueList)
            delete iv;
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
        XmlPathFileReader xmlFileReader(pathFilename_, filenameQueue_);
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


void testDataRecordFile()
{
    DataRecordFile dataRecordFile;

    vector<string> records = {"title", "abstract", "claim", "abstract"};

/*
    uint32_t recordSize = sizeof(uint32_t) * 5;
    for (const auto& r : records)
        recordSize += r.length();

*/
    dataRecordFile.appendRecord(records);

    dataRecordFile.writeToFile("test.bin");

    dataRecordFile.clear();

    printf("should be empty\n");

    dataRecordFile.readFromFile("test.bin");

    DataRecordEntry dre = dataRecordFile.GetRecordAtOffset(0);
    cout << dre.size;

    printf("check dre\n");
}

void testIndexValueStream()
{
    IndexValue iv;
    iv.pid = "pid";
    iv.aid = "aid";
    iv.appDate = "appDate";
    iv.binId = 1;
    iv.offset = 2;
    iv.ti = 3;
    iv.ai = 4;
    iv.ci = 5;
    iv.di = 6;
    iv.ipcList = {"okay", "hello"};

    cout << iv << '\n';
    stringstream ss("ppp\taaaaa\taappp\t2\t3\t2222\t4444\t555\t12\thuehue,haha\t23");
    ss >> iv;

    cout << iv << '\n';

    printf("stop here\n");
}

void generateRandomString(char *s, const int len)
{
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
}


void testIndexTableMemUsage()
{
    constexpr int N = 20000000;
    // 2000,0000 IndexValues uses about 3.4 GB of memory
    std::vector<IndexValue*> ivList;
    ivList.reserve(N);

    // each table with 2000,0000 entries uses about
    // 3.3 GB of memory
    unordered_map<string, vector<IndexValue*>> table1;
    unordered_map<string, vector<IndexValue*>> table2;

    table1.reserve(N);
    table2.reserve(N);

    printf("test here\n");

    constexpr int keyLen = 40;
    char str[keyLen + 1];
    for (int i = 0; i < N; i++) {
        ivList.push_back(new IndexValue);
        generateRandomString(str, keyLen);
        table1[string(str)].push_back(ivList.back());
        generateRandomString(str, keyLen);
        table2[string(str)].push_back(ivList.back());
    }

    printf("press to delete all pointers ");
    getchar();

    for (int i = 0; i < N; i++)
        delete ivList[i];

    printf("press to exit\n");
    getchar();
}


void getlineTest()
{
    ofstream ofs("test.tmp");
    string str("test\tokay  yea\t\t  blah\nx\nz y\n");
    ofs << str;
    ofs.close();

    ifstream ifs("test.tmp");
    for (string line; getline(ifs, line);) {
        cout << line << '\n';
        istringstream ss(line);
        for (string field; getline(ss, field, '\t');) {
            if (field.empty()) continue;
            cout << field << '\n';
        }
        cout << "---------------\n";
    }
    ifs.close();
}

void testIndexTable()
{
    constexpr int N = 20000000;

    IndexTable indexTable({IndexTable::PID, IndexTable::AID});

    indexTable.reserve(N);

    constexpr int keyLen = 25;
    char str[keyLen + 1];
    for (int i = 0; i < N; i++) {
        indexTable.appendIndexValue([&](IndexValue* iv) {
            generateRandomString(str, keyLen);
            iv->pid = str;
            generateRandomString(str, keyLen);
            iv->aid = str;
        });
    }

    printf("finished making index values");
    getchar();
}

class TagTester : public XmlPCProcessorTester {
    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<IdNodeFilter>(tags::publication_reference);
        tagNodeFilterDict_.add<IdNodeFilter>(tags::application_reference);
//        tagNodeFilterDict_.add<AbstractGreedyNodeFilter>(tags::abstract);
//        tagNodeFilterDict_.add<ClaimNodeFilter>(tags::claim);
    }

    void prepareOutputFormatters() final
    {
//        tagTextOutputFormatterDict_.add<AbstractDataFormatter>(tags::abstract);
//        tagTextOutputFormatterDict_.add<ClaimDataFormatter>(tags::claim);
    }

public:
    TagTester(string_view pathFilename, string_view outputFilename,
            int nProducers) : XmlPCProcessorTester(pathFilename, outputFilename, nProducers)
    { }
};

class IPOTagTester : public XmlIPOTagTextPrinterTester {
    void prepareNodeFilters() final
    {
        tagNodeFilterDict_.add<IdNodeFilter>(tags::publication_reference);
        tagNodeFilterDict_.add<IdNodeFilter>(tags::application_reference);
        tagNodeFilterDict_.add<AbstractGreedyNodeFilter>(tags::abstract);
        tagNodeFilterDict_.add<ClaimNodeFilter>(tags::claim);
    }

    void prepareOutputFormatters() final
    {
        tagTextOutputFormatterDict_.add<AbstractDataFormatter>(tags::abstract);
        tagTextOutputFormatterDict_.add<ClaimDataFormatter>(tags::claim);
    }
public:
    IPOTagTester(string_view pathFilename, string_view outputFilename,
            int nReaders, int nProcessors) :
            XmlIPOTagTextPrinterTester(pathFilename, outputFilename, nReaders, nProcessors) { }
};

#define MODEL3

#if defined(MODEL2)
struct Usage {
    static constexpr int ARGC = 4;
    static void printAndExit(const char* program)
    {
        printf("Usage:\n\t%s <path-file> <output-file> <n-workers>\n", program);
        exit(-1);
    }
};


int main(int argc, char* argv[])
{
    if (argc != Usage::ARGC)
        Usage::printAndExit(argv[0]);

    TagTester tagTester(argv[1], argv[2], atoi(argv[3]));

    tagTester.process();

    return 0;
}

#elif defined(MODEL3)
struct Usage {
    static constexpr int ARGC = 5;
    static void printAndExit(const char* program)
    {
        printf("Usage:\n\t%s <path-file> <output-file> <n-readers> <n-workers>\n", program);
        exit(-1);
    }
};


int main(int argc, char* argv[])
{
    if (argc != Usage::ARGC)
        Usage::printAndExit(argv[0]);

    IPOTagTester tagTester(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));

    tagTester.process();

    return 0;
}

#endif


