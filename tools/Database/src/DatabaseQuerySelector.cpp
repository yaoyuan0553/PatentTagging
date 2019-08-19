//
// Created by yuan on 8/17/19.
//

#include <iostream>

#include "DatabaseQuerySelector.h"

#include <fstream>


using namespace std;


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


