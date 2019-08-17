//
// Created by yuan on 8/17/19.
//

#pragma once
#ifndef TOOLS_DATABASEQUERY_H
#define TOOLS_DATABASEQUERY_H


#include <filesystem>
#include <string>
#include "DataTypes.h"


class DatabaseQuery {
    std::filesystem::path indexFilename_;
    std::filesystem::path dataPath_;
    std::filesystem::path dataFilePrefix_;

    IndexTable indexTable_;

    const IndexTableWithSpecificKey& pidTable;
    const IndexTableWithSpecificKey& aidTable;

    inline std::filesystem::path getBinFilenameWithBinId(uint32_t binId) const
    {
        return std::string(dataPath_ / dataFilePrefix_) + "_" +
                std::to_string(binId) + ".bin";
    }

public:

    enum IdType {
        PID = IndexTable::PID,
        AID = IndexTable::AID
    };

    DatabaseQuery(
            std::string_view indexFilename,
            std::string_view dataPath,
            std::string_view dataFilePrefix) :

            indexFilename_(indexFilename),
            dataPath_(dataPath),
            dataFilePrefix_(dataFilePrefix),
            indexTable_({IndexTable::PID, IndexTable::AID}),
            pidTable(indexTable_[IndexTable::PID]),
            aidTable(indexTable_[IndexTable::AID])
    {
        indexTable_.readFromFile(indexFilename_.c_str());
    }

    bool getInfoById(const std::string& id, IndexValue** output) const
    {
        if (pidTable.find(id) != pidTable.end()) {
            *output = pidTable.at(id);
            return true;
        }
        if (aidTable.find(id) != aidTable.end()) {
            *output = aidTable.at(id);
            return true;
        }
        // id not found
        fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                __FUNCTION__, id.c_str());
        return false;
    }

    void getInfoByIdList(const std::vector<std::string>& idList,
            std::unordered_set<IndexValue*>* output) const
    {
        for (const auto& id : idList) {
            IndexValue* iv;
            if (!getInfoById(id, &iv))
                continue;
            output->insert(iv);
        }
    }

    void getAllId(std::vector<std::string>* pidList = nullptr,
            std::vector<std::string>* aidList = nullptr) const
    {
        if (pidList != nullptr) {
            pidList->reserve(pidTable.size());
            for (const auto& [pid, _] : pidTable)
                pidList->push_back(pid);
        }
        if (aidList != nullptr) {
            aidList->reserve(aidTable.size());
            for (const auto& [aid, _] : aidTable)
                aidList->push_back(aid);
        }
    }

    /* returns false if id is not found in index table */
    bool getContentById(const std::string& id, DataRecord* dataRecord) const
    {
        IndexValue* iv;
        if (!getInfoById(id, &iv)) {
            fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                    __FUNCTION__, id.c_str());
            return false;
        }

        std::filesystem::path binFilename = getBinFilenameWithBinId(iv->binId);

        DataRecordFile dataRecordFile;
        dataRecordFile.readFromFile(binFilename.c_str());

        return dataRecordFile.GetDataRecordAtOffset(iv->offset, dataRecord);
    }

    void getContentByIdList(const std::vector<std::string>& idList,
            std::unordered_map<std::string, DataRecord>* contentById)
    {
        std::unordered_set<IndexValue*> infoList;
        getInfoByIdList(idList, &infoList);

        if (infoList.empty()) {
            fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
            return;
        }

        contentById->reserve(infoList.size());

        /* get all binId of data files needed to be opened */
        std::unordered_map<uint32_t, std::vector<IndexValue*>> indexByBinId;
        for (IndexValue* info : infoList) {
            indexByBinId[info->binId].push_back(info);
        }

        for (const auto& [binId, ivList]: indexByBinId) {
            DataRecordFile dataFile;
            dataFile.readFromFile(getBinFilenameWithBinId(binId).c_str());
            for (IndexValue* iv : ivList) {
                DataRecord dr;
                if (!dataFile.GetDataRecordAtOffset(iv->offset, &dr))
                    continue;
                (*contentById)[iv->pid] = std::move(dr);
            }
        }
    }
};


#endif //TOOLS_DATABASEQUERY_H
