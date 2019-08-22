//
// Created by yuan on 8/17/19.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYMANAGER_H
#define TOOLS_DATABASEQUERYMANAGER_H


#include <string>

#include "DataTypes.h"
#include "ConcurrentStaticQueue.h"

#include "ThreadModelInterface.h"


class DatabaseQueryManager {
    const std::string indexFilename_;
    const std::string dataPath_;
    const std::string dataFilePrefix_;

    IndexTable indexTable_;

    const IndexTableWithSpecificKey& pidTable_;
    const IndexTableWithSpecificKey& aidTable_;

    /* testing reader threads */
    ConcurrentStaticQueue<IdDataRecord*> contentQueue_;

public:

    inline std::string getBinFilenameWithBinId(uint32_t binId) const
    {
        return dataPath_ + '/' +  dataFilePrefix_ + "_" +
               std::to_string(binId) + ".bin";
    }

//    void (*ProcessContent)()

    enum IdType {
        PID = IndexTable::PID,
        AID = IndexTable::AID
    };

    DatabaseQueryManager(
            std::string_view indexFilename,
            std::string_view dataPath,
            std::string_view dataFilePrefix = "patent-data"
            );

    ConcurrentStaticQueue<IdDataRecord*>& getContentQueue() { return contentQueue_; }

    bool getInfoById(const std::string& id, IndexValue** output) const;

    void getInfoByIdList(const std::vector<std::string>& idList,
            std::unordered_set<IndexValue*>* output) const;

    void getInfoByIdList(const char** idList,
            std::unordered_set<IndexValue*>* output, int size) const;

    void getAllId(std::vector<std::string>* pidList = nullptr,
            std::vector<std::string>* aidList = nullptr) const;
//    void getAllId(std::string* pidList = nullptr, std::string* aidList = nullptr);

    /* returns false if id is not found in index table */
    bool getContentById(const std::string& id, DataRecord* dataRecord) const;

    void getContentByIdList(const std::vector<std::string>& idList,
            std::unordered_map<std::string, DataRecord>* contentById) const;

    void getContentByIdList(const char** idList,
            IdDataRecordCType idDataRecordList[], int size) const;

    void getContentByIdList(const std::vector<std::string>& idList)
    {
        std::unordered_set<IndexValue*> infoList;
        getInfoByIdList(idList, &infoList);

        if (infoList.empty()) {
            fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
            return;
        }

        /* get all binId of data files needed to be opened */
        std::unordered_map<uint32_t, std::vector<IndexValue*>> indexByBinId;
        for (IndexValue* info : infoList) {
            indexByBinId[info->binId].push_back(info);
        }

        // TODO: optimize for caching
        for (const auto& [binId, ivList]: indexByBinId) {
            DataRecordFile dataFile;
            std::string binName = getBinFilenameWithBinId(binId);
            dataFile.readFromFile(binName.c_str());
            printf("data file records: %u", dataFile.numRecords());
            /* read whole request if more than 1/3 of records must be accessed  */
            if (dataFile.numRecords() > ivList.size() / 3)
                dataFile.readFromFileFull(binName.c_str());

            for (IndexValue* iv : ivList) {
                // WARNING released by caller
                auto* dr = new IdDataRecord;
                if (!dataFile.GetDataRecordAtOffset(iv->offset, &dr->dataRecord))
                    continue;
                contentQueue_.push(dr);
            }
        }
    }
};

class ContentReaderThread : public OutputThreadInterface<
        ConcurrentStaticQueue<IdDataRecord*>> {

    const std::vector<std::string>& idList_;
    const DatabaseQueryManager& databaseQueryManager_;

    void internalRun() final
    {
        std::unordered_set<IndexValue*> infoList;
        databaseQueryManager_.getInfoByIdList(idList_, &infoList);

        if (infoList.empty()) {
            fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
            return;
        }

        /* get all binId of data files needed to be opened */
        std::unordered_map<uint32_t, std::vector<IndexValue*>> indexByBinId;
        for (IndexValue* info : infoList) {
            indexByBinId[info->binId].push_back(info);
        }

        // TODO: optimize for caching
        for (const auto&[binId, ivList]: indexByBinId) {
            DataRecordFile dataFile;
            std::string binName = databaseQueryManager_.getBinFilenameWithBinId(binId);
            dataFile.readFromFile(binName.c_str());
            printf("data file records: %u", dataFile.numRecords());
            /* read whole request if more than 1/3 of records must be accessed  */
            if (dataFile.numRecords() > ivList.size() / 3)
                dataFile.readFromFileFull(binName.c_str());

            for (IndexValue* iv : ivList) {
                // WARNING released by caller
                auto* dr = new IdDataRecord;
                if (!dataFile.GetDataRecordAtOffset(iv->offset, &dr->dataRecord)) {
                    fprintf(stderr, "%s skipped\n", iv->pid.c_str());
                    continue;
                }
                dr->pid = iv->pid;
                dr->aid = iv->aid;
                outputData_.push(dr);
            }
        }
    }

public:
    ContentReaderThread(OutputType& contentQueue,
                        const DatabaseQueryManager& databaseQueryManager,
                        const std::vector<std::string>& idList) :
            OutputThreadInterface(contentQueue),
            idList_(idList),
            databaseQueryManager_(databaseQueryManager) { }
};

#endif //TOOLS_DATABASEQUERYMANAGER_H
