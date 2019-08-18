//
// Created by yuan on 8/13/19.
//


#include <iostream>
#include <filesystem>

#include "Shared.h"

#include "DataTypes.h"

#include "XpathQueryCollection.h"

#include "DatabaseGenerator.h"

#include "Helper.h"

#include "DatabaseQueryManager.h"

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









void testDataRecordFile()
{
    DataRecordFile dataRecordFile;

    vector<string> dataText = {"title", "abstract", "claim", "description"};

    vector<string> indexText = { "pid", "aid", "appDate", "ipc1,ipc2"};
    vector<string> dataText1 = {"title1", "abstract1", "claim1", "description1"};

    vector<string> indexText1 = { "pid11", "aid1", "appDate1", "ipc111,ipc21"};
/*
    uint32_t recordSize = sizeof(uint32_t) * 5;
    for (const auto& r : records)
        recordSize += r.length();

*/
    if (!dataRecordFile.appendRecord(dataText, indexText))
        cerr << "what?\n";

    if (!dataRecordFile.appendRecord(dataText1, indexText1))
        cerr << "what?\n";

    dataRecordFile.writeToFile("test.bin");
    dataRecordFile.writeSubIndexTableToFile("index.tsv");

    IndexValue iv0 = *dataRecordFile.indexSubTable()[0];
    IndexValue iv1 = *dataRecordFile.indexSubTable()[1];
    dataRecordFile.clear();

    printf("should be empty\n");

    dataRecordFile.readFromFile("test.bin");

    DataRecord rcd;
    dataRecordFile.GetDataRecordAtOffset(DataRecordFile::FILE_HEAD_SIZE, &rcd);
    cout << rcd << '\n';

    printf("check dre\n");
}

void testIndexValueStream()
{
    IndexValue iv;
    iv.pid = "pid";
    iv.aid = "aid";
    iv.appDate = "appDate";
    iv.ipc = "ipc1,ipc2,ipc3";
    iv.binId = 1;
    iv.offset = 2;
    iv.ti = 3;
    iv.ai = 4;
    iv.ci = 5;
    iv.di = 6;

    cout << iv << '\n';
    stringstream ss("pid\taid\tappDate\tipc\tbinId\toffset\tti\tai\tci\tdi\t");
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


class XpathIPOTagTester : public XmlBufferXpathIPOTagTextPrinterTester {
    /* index file fields */
    ISC_STRING(PID) = "pid";
    ISC_STRING(AID) = "aid";
    ISC_STRING(PUB_DATE) = "pubDate";
    ISC_STRING(APP_DATE) = "appDate";
    ISC_STRING(IPC) = "ipc";                // TODO

    /* data file fields */
    ISC_STRING(TITLE) = "title";
    ISC_STRING(ABSTRACT) = "abstract";
    ISC_STRING(CLAIM) = "claim";
    ISC_STRING(DESCRIPTION) = "description";

    void initializeQuery() final
    {
//        xpathQueryTextFormatterDict_.add<XpathSingleQueryGreedyNoExtraSpaceInnerText>(
//                "claimText", XpathQueryString("//claim-text")
//                );
        xpathQueryTextFormatterDict_.add<XpathIdQuery>(
                PID, "//publication-reference",
                vector<XpathQueryString>{".//country", ".//doc-number", ".//kind"}
        );

        xpathQueryTextFormatterDict_.add<XpathIdQuery>(
                AID, "//application-reference",
                vector<XpathQueryString>{".//country", ".//doc-number"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery>(
                PUB_DATE, "//publication-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery>(
                APP_DATE, "//application-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathIpcQuery>(
                IPC, ".//classification-ipcr"
        );

        xpathQueryTextFormatterDict_.add<XpathTitleQuery>(
                TITLE, "//invention-title"
        );

        xpathQueryTextFormatterDict_.add<XpathAbstractQuery>(
                ABSTRACT, "//abstract"
        );

        xpathQueryTextFormatterDict_.add<XpathClaimQuery>(
                CLAIM, "//claim"
        );

        xpathQueryTextFormatterDict_.add<XpathDescriptionQuery>(
                DESCRIPTION, "//description"
        );
    }
public:
    XpathIPOTagTester(string_view pathFilename, string_view outputFilename,
            int nReaders, int nProcessors) :
            XmlBufferXpathIPOTagTextPrinterTester(pathFilename,
                    outputFilename, nReaders, nProcessors) { }
};

#define MODEL4

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

#elif defined(MODEL4)
struct Usage {
    static constexpr int ARGC = 6;
    static void printAndExit(const char* program)
    {
        printf("Usage:\n\t%s <in-path-file> <out-data-dir> <out-index-file> "
               "<n-readers> <n-workers>\n", program);
        exit(-1);
    }
};

//#elif defined(TEST_QUERY_API)
struct QueryUsage {
    static constexpr int ARGC = 3;
    static void printAndExit(const char* program)
    {
        printf("Usage\n\t%s <index-file> <data-dir> \n", program);
        exit(-1);
    }
};

#endif

void testDatabaseQuery(int argc, char* argv[])
{
    if (argc != QueryUsage::ARGC)
        QueryUsage::printAndExit(argv[0]);

    DatabaseQueryManager databaseQuery(argv[1], argv[2], "patent-data");

    unordered_set<IndexValue*> result;
    databaseQuery.getInfoByIdList(
            {"US20140345317A1", "US20140122454A1", "US14351605", "US14356959"},
            &result);

    for (IndexValue* iv : result)
        cout << *iv << '\n';

    vector<string> resPid, resAid;
    databaseQuery.getAllId(&resPid, &resAid);

//    for (auto& id : resAid) {
//        cout << id << '\n';
//    }

    DataRecord dataRecord;
    databaseQuery.getContentById("US20140345317A1", &dataRecord);
//    cout << dataRecord << '\n';

    unordered_map<string, DataRecord> contentById;
    databaseQuery.getContentByIdList(resPid, &contentById);

//    for (const auto& [id, record] : contentById)
//        cout << id << ": " << record << '\n';
    cout << contentById.size() << endl;
}


int main(int argc, char* argv[])
{
    if (argc != Usage::ARGC)
        Usage::printAndExit(argv[0]);

//    XpathIPOTagTester tagTester(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
//
//    tagTester.process();
//    testDataRecordFile();

    DatabaseGenerator databaseGenerator(argv[1], argv[2], argv[3],
            atoi(argv[4]), atoi(argv[5]));

    databaseGenerator.process();

//    testDatabaseQuery(argc, argv);

    return 0;
}

