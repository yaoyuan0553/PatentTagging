//
// Created by yuan on 8/17/19.
//

#include <iostream>
#include <fstream>
#include <string>

#include "DatabaseQuerySelector.h"



using namespace std;


/*
IndexValue* IndexValue_new()
{
    return new IndexValue;
}

const char* IndexValue_stringify(IndexValue* iv)
{
    string cppStr = iv->stringify();

    char* cStr = new char[cppStr.length() + 1];

    snprintf(cStr, cppStr.length() + 1, "%s", cppStr.c_str());

    return cStr;
}
*/

DatabaseQueryManager* DatabaseQueryManager_new(char* indexFilename,
        char* dataPath)
{
    auto p = new DatabaseQueryManager(indexFilename, dataPath);
//    printf("c++ &dqmPtr = %p\n", p);

    return p;
}

void DatabaseQueryManager_delete(DatabaseQueryManager* dqm)
{
    delete dqm;
}

void DatabaseQueryManager_getAllId(DatabaseQueryManager* dqm,
        char**& pidList,
        char**& aidList,
        int& size)
{
//    vector<string> *pidVec = new vector<string>, *aidVec = new vector<string>;
    vector<string> pidVec, aidVec;
    dqm->getAllId(&pidVec, &aidVec);

    pidList = new char*[pidVec.size()];
    aidList = new char*[aidVec.size()];
    for (size_t i = 0; i < pidVec.size(); i++) {
        auto len = pidVec[i].length();
        pidList[i] = new char[len + 1];
        memcpy(pidList[i], pidVec[i].c_str(), len);
        pidList[i][len] = 0;
    }

    for (size_t i = 0; i < aidVec.size(); i++) {
        auto len = aidVec[i].length();
        aidList[i] = new char[len + 1];
        memcpy(aidList[i], aidVec[i].c_str(), len);
        aidList[i][len] = 0;
    }
    size = pidVec.size();
}

DataRecordCType DatabaseQueryManager_getContentById(DatabaseQueryManager* dqm,
        const char* id/*, DataRecordCType* drct*/)
{
    DataRecord dr;
    if (!dqm->getContentById(id, &dr))
        return DataRecordCType();

    DataRecordCType drct;

    ConvertToDataRecordCType(&drct, &dr);

#define PRINT(name) printf("c++: %s = %d\n", #name, drct.name);
#define PRINT_STR(name) printf("c++: %s = %s\n", #name, drct.name);

//    printf("c++: %d\n", drct.size);

//    PRINT(size);
//    PRINT(ts);
//    PRINT(as);
//    PRINT(cs);
//    PRINT(ds);
//
//    PRINT_STR(title);
//    printf("c++ &title = 0x%016lx\n", (uint64_t)(drct.title));
//    printf("c++ &abstract = 0x%016lx\n", (uint64_t)(drct.abstract));

    return drct;
}


//void delete_arrayPtr(void* arrPtr)
//{
//    delete[] arrPtr;
//}
//
//void delete_ptr(void* ptr)
//{
//
//}

struct QueryUsage {
    static constexpr int ARGC = 4;
    static void printAndExit(const char* program)
    {
        printf("Usage\n\t%s <index-file> <data-dir> <request-id-file>\n", program);
        exit(-1);
    }

};

int main(int argc, char* argv[])
{
    if (argc != QueryUsage::ARGC)
        QueryUsage::printAndExit(argv[0]);

    ifstream requestIdFile(argv[3]);
    if (!requestIdFile.is_open()) {
        fprintf(stderr, "failed to open [%s]\n", argv[3]);
        PERROR("ifstream");
    }

    DatabaseQueryManager databaseQueryManager(argv[1], argv[2], "patent-data");


    vector<string> idList;
    for (string line; getline(requestIdFile, line);) {
        if (line.length() == 14)
            line.insert(6, 1, '0'); // insert 0 at position 6 (after US2007)
        idList.push_back(line);
    }

//    unordered_set<IndexValue*> result;
//    databaseQueryManager.getInfoByIdList(idList, &result);
//
//    for (IndexValue* iv : result)
//        cout << *iv <<'\n';


    unordered_map<std::string, DataRecord> contentById;
    databaseQueryManager.getContentByIdList(idList, &contentById);

    for (const auto& [id, dr] : contentById) {
        cout << "ID: " << id << '\n' << dr << '\n';
    }

    requestIdFile.close();

    return 0;
}


