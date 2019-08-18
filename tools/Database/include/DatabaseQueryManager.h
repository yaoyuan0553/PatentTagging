//
// Created by yuan on 8/17/19.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYMANAGER_H
#define TOOLS_DATABASEQUERYMANAGER_H


#include <filesystem>
#include <string>
#include "DataTypes.h"


class DatabaseQueryManager {
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

    DatabaseQueryManager(
            std::string_view indexFilename,
            std::string_view dataPath,
            std::string_view dataFilePrefix = "patent-data"
            );


    bool getInfoById(const std::string& id, IndexValue** output) const;

    void getInfoByIdList(const std::vector<std::string>& idList,
            std::unordered_set<IndexValue*>* output) const;

    void getAllId(std::vector<std::string>* pidList = nullptr,
            std::vector<std::string>* aidList = nullptr) const;

    /* returns false if id is not found in index table */
    bool getContentById(const std::string& id, DataRecord* dataRecord) const;

    void getContentByIdList(const std::vector<std::string>& idList,
            std::unordered_map<std::string, DataRecord>* contentById) const;
};


#endif //TOOLS_DATABASEQUERYMANAGER_H
