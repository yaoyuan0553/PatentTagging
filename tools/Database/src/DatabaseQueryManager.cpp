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
        pidTable_(indexTable_[IndexTable::PID]),
        aidTable_(indexTable_[IndexTable::AID])
{
    indexTable_.readFromFile(indexFilename_.c_str());
}


bool DatabaseQueryManager::getInfoById(const std::string& id, IndexValue** output) const
{
    if (pidTable_.find(id) != pidTable_.end()) {
        *output = pidTable_.at(id);
        return true;
    }
    if (aidTable_.find(id) != aidTable_.end()) {
        *output = aidTable_.at(id);
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

//void DatabaseQueryManager::getAllId(std::string* pidList, std::string* aidList) const
//{
//
//    if (pidList != nullptr) {
//        pidList->reserve(pidTable_.size());
//        for (const auto& [pid, _] : pidTable_)
//            pidList->push_back(pid);
//    }
//    if (aidList != nullptr) {
//        aidList->reserve(aidTable_.size());
//        for (const auto& [aid, _] : aidTable_)
//            aidList->push_back(aid);
//    }
//}
void DatabaseQueryManager::getAllId(std::vector<std::string>* pidList,
        std::vector<std::string>* aidList) const
{
    if (pidList != nullptr) {
        pidList->reserve(pidTable_.size());
        for (const auto& [pid, _] : pidTable_)
            pidList->push_back(pid);
    }
    if (aidList != nullptr) {
        aidList->reserve(aidTable_.size());
        for (const auto& [aid, _] : aidTable_)
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

    std::string binFilename = getBinFilenameWithBinId(iv->binId);

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
        std::string binName = getBinFilenameWithBinId(binId);
        dataFile.readFromFile(binName.c_str());
        printf("data file records: %u", dataFile.numRecords());
        /* read whole request if more than 1/3 of records must be accessed  */
        if (dataFile.numRecords() > ivList.size() / 3)
            dataFile.readFromFileFull(binName.c_str());

        for (IndexValue* iv : ivList) {
            DataRecord dr;
            if (!dataFile.GetDataRecordAtOffset(iv->offset, &dr))
                continue;
            (*contentById)[iv->pid] = std::move(dr);
        }
    }
}

/* assumes idDataRecordList is initialized to idList.size() */
void DatabaseQueryManager::getContentByIdList(const char** idList,
        IdDataRecordCType idDataRecordList[], int size) const
{
    std::unordered_set<IndexValue*> infoList;
    getInfoByIdList(idList, &infoList, size);

    if (infoList.empty()) {
        fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
        return;
    }

    /* get all binId of data files needed to be opened */
    std::unordered_map<uint32_t, std::vector<IndexValue*>> indexByBinId;
    for (IndexValue* info : infoList) {
        indexByBinId[info->binId].push_back(info);
    }

    int i = 0;
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
            if (!dataFile.GetDataRecordAtOffset(iv->offset, idDataRecordList + i))
                continue;
            idDataRecordList[i].pid = new char[iv->pid.length() + 1];
            idDataRecordList[i].pid[iv->pid.length()] = 0;
            memcpy(idDataRecordList[i].pid, iv->pid.c_str(), iv->pid.length());

            idDataRecordList[i].aid = new char[iv->aid.length() + 1];
            idDataRecordList[i].aid[iv->aid.length()] = 0;
            memcpy(idDataRecordList[i].aid, iv->aid.c_str(), iv->aid.length());
        }
        i++;
    }
}

void DatabaseQueryManager::getInfoByIdList(const char** idList,
        std::unordered_set<IndexValue*>* output, int size) const
{
    for (int i = 0; i < size; i++) {
        IndexValue* iv;
        if (!getInfoById(idList[i], &iv))
            continue;
        output->insert(iv);
    }
}


