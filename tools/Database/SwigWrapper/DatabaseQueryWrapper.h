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
    enum ContentPartType {
        TITLE,
        ABSTRACT,
        CLAIM,
        DESCRIPTION
    };

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

    /**
     * @brief generate a full-path database bin filename with the given bin ID
     * @param binId bin ID of the desired file
     * @return string of the generated filename
     */
    inline std::string getBinFilenameWithBinId(uint32_t binId) const
    {
        return dataPath_ + '/' +  dataFilePrefix_ + "_" +
               std::to_string(binId) + ".bin";
    }


    /**
     * @brief get a single index info with a given PID or AID
     * @param id    PID or AID
     * @return      corresponding index record if found,
     *              if neither PID or AID were found,
     *              it returns nullptr
     */
    const IndexValue* getInfoById(const char* id) const;

    /**
     * @brief plural version of getInfoById, use a vector of PID or AID's
     *        to retrieve corresponding index values
     * @param idList    INPUT: a list of PID or AIDs
     * @param output    OUTPUT: a list of IndexValue pointers
     */
    void getInfoByIdList(const std::vector<std::string>& idList,
            std::vector<const IndexValue*>* output) const;

    /**
     * @brief returns a pidList or a aidList or both
     * @param pidList   OUTPUT: returns a list of PIDs if not null
     * @param aidList   OUTPUT: returns a list of AIDs if not null
     */
    void getAllId(std::vector<std::string>* pidList = nullptr,
            std::vector<std::string>* aidList = nullptr) const;

    /**
     * @brief retrieves a single parsed XML text body with a given PID or AID
     * @warning this method is meant for single queries only. For a collection of
     *          PIDs or AIDs, if this method is called frequently, it'll be
     *          extremely inefficient. Please use getContentByIdList instead
     * @param id            INPUT: PID or AID
     * @param dataRecord    OUTPUT: retrieved dataRecord, null if not found
     * @return              true if found, false if not found
     */
    bool getContentById(const char* id, DataRecord* dataRecord) const;

    bool getContentPartById(const char* id, ContentPartType contentPartType, std::string* contentPart) const;


//    bool getAbstractById(const char* id, std::string* abstract) const;


    /**
     * @brief retrieves a collection of parsed XML text body with a list of PIDs or AIDs
     * @param idList            INPUT: list of PIDs or AIDs
     * @param idDataRecordList  OUTPUT: list of IdDataRecord
     */
    void getContentByIdList(const std::vector<std::string>& idList,
            std::vector<std::shared_ptr<IdDataRecord>>* idDataRecordList) const;
};





#endif //TOOLS_DATABASEQUERYSELECTORSWIG_H
