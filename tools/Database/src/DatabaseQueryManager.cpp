//
// Created by yuan on 8/17/19.
//

#include "DatabaseQueryManager.h"



DatabaseQueryManager::DatabaseQueryManager(
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


bool DatabaseQueryManager::getInfoById(const std::string& id, IndexValue** output) const
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

void DatabaseQueryManager::getInfoByIdList(const std::vector<std::string>& idList,
        std::unordered_set<IndexValue*>* output) const
{
    for (const auto& id : idList) {
        IndexValue* iv;
        if (!getInfoById(id, &iv))
            continue;
        output->insert(iv);
    }
}

void DatabaseQueryManager::getAllId(std::vector<std::string>* pidList,
        std::vector<std::string>* aidList) const
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

bool DatabaseQueryManager::getContentById(const std::string& id, DataRecord* dataRecord) const
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

void DatabaseQueryManager::getContentByIdList(const std::vector<std::string>& idList,
        std::unordered_map<std::string, DataRecord>* contentById) const
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

    // TODO: optimize for caching
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


