//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYSELECTORSWIG_H
#define TOOLS_DATABASEQUERYSELECTORSWIG_H

#include <string>
#include <thread>
#include <memory>

#include "ThreadJob.h"
#include "DataTypes.h"

#include "DataRecordFileV2.h"
#include "IndexTableV2.h"
#include "ConcurrentBlockingStaticQueue.h"

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

    inline static constexpr uint32_t INVALID = -1;

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

    /**
     * @brief retrieves a single document's title/abstract/claim/description with a given PID or AID
     * @warning this method is meant for single queries only. For a collection of
     *          PIDs or AIDs, if this method is called frequently, it'll be
     *          extremely inefficient. Please use getContentPartByIdList instead
     * @param id                INPUT: PID or AID
     * @param contentPartType   INPUT: ContentPartType to be retrieved, options are: TITLE, ABSTRACT, CLAIM, DESCRIPTION
     * @param contentPart       OUTPUT: the retrieved data, empty if not found
     * @return                  true if found, false if not found
     */
    bool getContentPartById(const char* id, ContentPartType contentPartType, std::string* contentPart) const;

    /**
     * @brief retrieves a collection of parsed XML text body with a list of PIDs or AIDs
     * @param idList            INPUT: list of PIDs or AIDs
     * @param contentPartType   INPUT: ContentPartType to be retrieved, options are: TITLE, ABSTRACT, CLAIM, DESCRIPTION
     * @param idContentPartList OUTPUT: list of IdContentPart
     */
    void getContentPartByIdList(const std::vector<std::string>& idList,
            ContentPartType contentPartType, std::vector<IdDataPart>* idContentPartList) const;

    /**
     * @brief retrieves a collection of parsed XML text body with a list of PIDs or AIDs
     * @param idList            INPUT: list of PIDs or AIDs
     * @param idDataRecordList  OUTPUT: list of IdDataRecord
     */
    void getContentByIdList(const std::vector<std::string>& idList,
            std::vector<std::shared_ptr<IdDataRecord>>* idDataRecordList) const;

private:

    inline bool checkIndexByContentPartType(ContentPartType cpt) const;

    inline uint32_t getIndexByContentPartType(const IndexValue* iv, ContentPartType cpt) const;
};

inline bool DatabaseQueryManager::checkIndexByContentPartType(ContentPartType cpt) const
{
    switch (cpt)
    {
    case TITLE: break;
    case ABSTRACT: break;
    case CLAIM: break;
    case DESCRIPTION: break;
    default:
        fprintf(stderr, "%s: incorrect ContentPartType option %d\n", __FUNCTION__, cpt);
        return false;
    }
    return true;
}

uint32_t DatabaseQueryManager::getIndexByContentPartType(const IndexValue* iv, ContentPartType cpt) const
{
    switch (cpt)
    {
    case TITLE:
        return iv->ti;
    case ABSTRACT:
        return iv->ai;
    case CLAIM:
        return iv->ci;
    case DESCRIPTION:
        return iv->di;
    default:
        fprintf(stderr, "%s: incorrect ContentPartType option %d\n", __FUNCTION__, cpt);
        return INVALID;
    }
}


/****************************************************
 * @brief DatabaseQueryManager Version 2
 ****************************************************/
class DatabaseQueryManagerV2 {

    class DataRecordFileReaderThread : public ThreadJob<> {
        /*! constants */
        inline static constexpr int MAX_PC_QUEUE_SIZE = 1 << 10;
        inline static constexpr int WRITE_AHEAD = 1 << 10;
        inline static constexpr int N_CONSUMERS = 1;

        uint32_t binId_;
        const DataRecordFileReader& dataRecordFileReader_;
        CBSQueue<std::string> idQueue_;
        void internalRun() final;
    public:
        DataRecordFileReaderThread(uint32_t binId, const DataRecordFileReader& dataRecordFileReader);

        inline CBSQueue<std::string>& idQueue() { return idQueue_; }
    };

    inline static constexpr uint32_t INVALID = -1;

    const std::string indexFilename_;
    const std::string dataPath_;
    const std::string dataFilePrefix_;

    IndexTableV2 indexTable_;

    const IndexTableV2::IdIndexTable& pidTable_;
    const IndexTableV2::IdIndexTable& aidTable_;

    std::unordered_map<uint32_t, const DataRecordFileReader> dataRecordFileByBinId_;

    std::unordered_map<uint32_t, std::unique_ptr<CBSQueue<std::string>>> dataRecordFileReaderThreads_;

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
    DatabaseQueryManagerV2(
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
    bool getContentById(const char* id, DataRecordV2* dataRecord) const;

    /**
     * @brief retrieves a single document's title/abstract/claim/description with a given PID or AID
     * @warning this method is meant for single queries only. For a collection of
     *          PIDs or AIDs, if this method is called frequently, it'll be
     *          extremely inefficient. Please use getContentPartByIdList instead
     * @param id                INPUT: PID or AID
     * @param contentPartType   INPUT: ContentPartType to be retrieved, options are: TITLE, ABSTRACT, CLAIM, DESCRIPTION
     * @param contentPart       OUTPUT: the retrieved data, empty if not found
     * @return                  true if found, false if not found
     */
    bool getContentPartById(const char* id, ContentPartType contentPartType, std::string* contentPart) const;

    /**
     * @brief retrieves a collection of parsed XML text body with a list of PIDs or AIDs
     * @param idList            INPUT: list of PIDs or AIDs
     * @param contentPartType   INPUT: ContentPartType to be retrieved, options are: TITLE, ABSTRACT, CLAIM, DESCRIPTION
     * @param idContentPartList OUTPUT: list of IdContentPart
     */
    void getContentPartByIdList(const std::vector<std::string>& idList,
                                ContentPartType contentPartType, std::vector<IdDataPart>* idContentPartList) const;

    /**
     * @brief retrieves a collection of parsed XML text body with a list of PIDs or AIDs
     * @param idList            INPUT: list of PIDs or AIDs
     * @param idDataRecordList  OUTPUT: list of IdDataRecord
     */
    void getContentByIdList(const std::vector<std::string>& idList,
                            std::vector<std::shared_ptr<IdDataRecord>>* idDataRecordList) const;

    /**
     * @brief retrieves a collection of parsed XML text body with a 
     * @param dataRecordById
     */
    void getContentByPidList(std::unordered_map<std::string, DataRecordV2>& dataRecordById);


private:

    inline bool checkIndexByContentPartType(ContentPartType cpt) const;

    inline uint32_t getIndexByContentPartType(const IndexValue* iv, ContentPartType cpt) const;
};

inline bool DatabaseQueryManagerV2::checkIndexByContentPartType(ContentPartType cpt) const
{
    switch (cpt)
    {
    case TITLE: break;
    case ABSTRACT: break;
    case CLAIM: break;
    case DESCRIPTION: break;
    default:
        fprintf(stderr, "%s: incorrect ContentPartType option %d\n", __FUNCTION__, cpt);
        return false;
    }
    return true;
}

uint32_t DatabaseQueryManagerV2::getIndexByContentPartType(const IndexValue* iv, ContentPartType cpt) const
{
    switch (cpt)
    {
    case TITLE:
        return iv->ti;
    case ABSTRACT:
        return iv->ai;
    case CLAIM:
        return iv->ci;
    case DESCRIPTION:
        return iv->di;
    default:
        fprintf(stderr, "%s: incorrect ContentPartType option %d\n", __FUNCTION__, cpt);
        return INVALID;
    }
};

#endif //TOOLS_DATABASEQUERYSELECTORSWIG_H
