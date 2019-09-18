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
#include "ThreadPool.h"
#include "DataTypes.h"
#include "Barrier.h"

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

        /*! * @brief atomically increments when every new thread is created */
        inline static std::atomic_uint32_t nextThreadId_ = 0;

        /*! * @brief set before constructor */
        inline static uint32_t threadCount_ = 0;

        /*! * @brief keeps track of every thread of this class */
        inline static std::vector<DataRecordFileReaderThread*> allThreads_;

        /*! @brief reference to be able to call query functions from threads */
        DatabaseQueryManagerV2& dqm_;

        uint32_t threadId_;

        std::pair<uint32_t, uint32_t> binIdRange_;

        /*! @brief to store ids corresponding to their binIds and
         *         count number of documents in each bin for each run */
        std::vector<std::vector<std::pair<const IndexValue*, DataRecordV2*>>> ivOutputVecByBinId_;

        void internalRun() final;
        void reset();
        void merge();
        void requestData();

    public:
        /*!
         * @brief set thread count of the thread pool
         * @details must be called at least once before the first ever constructor call, resize the allThreads_ vector
         * @warning NOT thread-safe! should not be called once threads are constructed!
         * @param threadCount sets the new threadCount
         */
        static void setThreadCount(uint32_t threadCount);

        explicit DataRecordFileReaderThread(DatabaseQueryManagerV2& databaseQueryManager);
    };

    inline static constexpr uint32_t INVALID = -1;

    const std::string indexFilename_;
    const std::string dataPath_;
    const std::string dataFilePrefix_;

    IndexTableV2 indexTable_;

    const IndexTableV2::IdIndexTable& pidTable_;
    const IndexTableV2::IdIndexTable& aidTable_;

    std::unordered_map<uint32_t, DataRecordFileReader> dataRecordFileByBinId_;

    /*! constants */
    inline static constexpr int MAX_PC_QUEUE_SIZE = 1 << 14;
    inline static constexpr int WRITE_AHEAD = 1 << 14;
    inline static constexpr int N_CONSUMERS = 4;

    /** reader thread properties */
    ThreadPool readerThreadPool_;
    CBSQueue<std::pair<std::string, DataRecordV2*>> idOutputQueue_;
    /*! @brief barrier to synchronize all dataRecordReaderThreads after they finish loading data */
    Barrier allThreadFinished_;
    std::atomic_bool readerThreadExit = false;

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

    ~DatabaseQueryManagerV2();

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
     * @brief retrieves a collection of parsed XML text body with with given PIDs
     * @details the keys of std::unordered_map are the request of PIDs, with default initialized
     *          DataRecordV2's, and the output will be will be stored in the value of
     *          std::unordered_map
     * @param dataRecordById key - requested PIDs, value - output content
     */
    void getContentByPidList(std::unordered_map<std::string, std::shared_ptr<DataRecordV2>>& dataRecordById);


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
