//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYSELECTORSWIG_H
#define TOOLS_DATABASEQUERYSELECTORSWIG_H

#include <string>
#include <thread>

#include "DataTypes.h"

void hello();

template <typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
};

void printStr(const char* str);

std::string makeStr();

void takeStr(std::string str);

void changeStr(char* str, int length);

void fillArray(int arr[], int n);

void changeIntPointerArray(int* array, int n);

void sleep(int sec);

void startThreadAndCount(int n);

std::thread* retCounterThread(int n);

/**
 * @brief blocks caller until the parameter represented thread finishes
 *
 * @param t     std::thread pointer representing a running thread
 */
void waitForThread(std::thread* t);





class DatabaseQueryManager {

    const std::string indexFilename_;
    const std::string dataPath_;
    const std::string dataFilePrefix_;

    IndexTable indexTable_;

    const IndexTableWithSpecificKey& pidTable_;
    const IndexTableWithSpecificKey& aidTable_;

public:

    /**
     * @brief constructs a singleton DatabaseQueryManager instance
     *
     * @param indexFilename     path to the index file (index.tsv)
     * @param dataPath          path to the directory containing database binaries (data/)
     * @param dataFilePrefix    prefix name of database binary files (patent-data)
     */
    DatabaseQueryManager(
            const char* indexFilename,
            const char* dataPath,
            const char* dataFilePrefix = "patent-data"
    );

    inline std::string getBinFilenameWithBinId(uint32_t binId) const
    {
        return dataPath_ + '/' +  dataFilePrefix_ + "_" +
               std::to_string(binId) + ".bin";
    }


    /**
     * @brief get index info with a given PID or AID
     * @param id    PID or AID
     * @return      corresponding index record if found,
     *              if not found returns nullptr
     */
    const IndexValue* getInfoById(const char* id) const;

    void getInfoByIdList(const std::vector<std::string>& idList,
            std::vector<const IndexValue*>* output) const;

    void getAllId(std::vector<std::string>* pidList = nullptr,
            std::vector<std::string>* aidList = nullptr) const;

    bool getContentById(const char* id, DataRecord* dataRecord) const;

    void getContentByIdList(const std::vector<std::string>& idList,
            std::vector<std::shared_ptr<IdDataRecord>>* idDataRecordList) const;
};





#endif //TOOLS_DATABASEQUERYSELECTORSWIG_H
