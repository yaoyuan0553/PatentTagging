//
// Created by yuan on 2019/8/22.
//

#include "DatabaseQueryWrapper.h"

#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>

#include <tqdm.h>

using namespace std;

void hello()
{
    std::cout << "Hello, World!" << std::endl;
}

void printStr(const char* str)
{
    printf("%s\n", str);
}

std::string makeStr()
{
    return "made this string";
}

void takeStr(std::string str)
{
    std::cout << str << '\n';
}

void changeStr(char* str, int length)
{
    cout << str << " " << length << '\n';

    for (int i = 0; i < length; i++) {
        str[i] = 'a';
    }

    printf("changed str: %s\n", str);
}

void fillArray(int arr[], int n)
{
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

void changeIntPointerArray(int* array, int n)
{
    for (int i = 0; i < n; i++)
        array[i] = i * 2;
}

void sleep(int sec)
{
    this_thread::sleep_for(chrono::seconds(sec));
}

uint64_t factorial(uint64_t n)
{
    if (!n) return 1;

    return n * factorial(n-1);
}

uint64_t fib(uint64_t n)
{
    if (n < 1) return 0;
    if (n == 1) return n;
    if (n == 2) return n;

    return fib(n-1) + fib(n-2);
}

void printFactorial(uint64_t n)
{
//    cout << factorial(n) << '\n';
    cout << fib(n) << '\n';
}

void startThreadAndCount(int n)
{
    thread t(printFactorial, n);

    t.detach();
//    t.join();
}

thread* retCounterThread(int n)
{
    return new thread(printFactorial, n);
}

void waitForThread(std::thread* t)
{
    t->join();

    delete t;
}


DatabaseQueryManager::DatabaseQueryManager(
        const char* indexFilename,
        const char* dataPath,
        const char* dataFilePrefix) :

        indexFilename_(indexFilename),
        dataPath_(dataPath),
        dataFilePrefix_(dataFilePrefix),
        indexTable_({IndexTable::PID, IndexTable::AID}),
        pidTable_(indexTable_[IndexTable::PID]),
        aidTable_(indexTable_[IndexTable::AID])
{
    indexTable_.readFromFile(indexFilename);
}

void DatabaseQueryManager::getAllId(vector<string>* pidList, vector<string>* aidList) const
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

bool DatabaseQueryManager::getContentById(const char* id, DataRecord* dataRecord) const
{
    const IndexValue* iv;
    if (!(iv = getInfoById(id))) {
        fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                __FUNCTION__, id);
        return false;
    }

    std::string binFilename = getBinFilenameWithBinId(iv->binId);

    DataRecordFile dataRecordFile;
    dataRecordFile.readFromFile(binFilename.c_str());

    return dataRecordFile.GetDataRecordAtOffset(iv->offset, dataRecord);
}

const IndexValue* DatabaseQueryManager::getInfoById(const char* id) const
{

    if (pidTable_.find(id) != pidTable_.end()) {
        return pidTable_.at(id);
    }
    if (aidTable_.find(id) != aidTable_.end()) {
        return aidTable_.at(id);
    }
    // id not found
    fprintf(stderr, "%s: ID [%s] does not exist in database\n",
            __FUNCTION__, id);

    return nullptr;
}


void DatabaseQueryManager::getInfoByIdList(
        const std::vector<std::string>& idList, std::vector<const IndexValue*>* output) const
{
    for (const auto& id : idList) {
        const IndexValue* iv;
        if (!(iv = getInfoById(id.c_str())))
            continue;
        output->push_back(iv);
    }
}

void DatabaseQueryManager::getContentByIdList(const vector<string>& idList,
        vector<shared_ptr<IdDataRecord>>* idDataRecordList) const
{
    vector<const IndexValue*> infoList;
    getInfoByIdList(idList, &infoList);

    if (infoList.empty()) {
        fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
        return;
    }
    /* get all binId of data files needed to be opened */
    std::unordered_map<uint32_t, std::vector<const IndexValue*>> indexByBinId;
    for (const IndexValue* info : infoList) {
        indexByBinId[info->binId].push_back(info);
    }
    idDataRecordList->reserve(idList.size());

    // TODO: optimize for caching
    for (const auto& [binId, ivList]: indexByBinId) {
        DataRecordFile dataFile;
        std::string binName = getBinFilenameWithBinId(binId);
        dataFile.readFromFile(binName.c_str());
        /* read whole request if more than 1/3 of records must be accessed  */
        if (ivList.size() > dataFile.numRecords() / 3)
            dataFile.readFromFileFull(binName.c_str());

        for (const IndexValue* iv : ivList) {
            idDataRecordList->emplace_back(new IdDataRecord);
            if (!dataFile.GetDataRecordAtOffset(iv->offset,
                    &idDataRecordList->back()->dataRecord))
                continue;
            idDataRecordList->back()->pid = iv->pid;
            idDataRecordList->back()->aid = iv->aid;
        }
    }
}

bool DatabaseQueryManager::getContentPartById(const char* id, ContentPartType cpt, std::string* contentPart) const
{
    const IndexValue* iv;
    if (!(iv = getInfoById(id))) {
        fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                __FUNCTION__, id);
        return false;
    }

    std::string binFilename = getBinFilenameWithBinId(iv->binId);

    DataRecordFile dataRecordFile;
    dataRecordFile.readFromFile(binFilename.c_str());

    uint32_t index = getIndexByContentPartType(iv, cpt);
    if (index == INVALID)
        return false;

    return dataRecordFile.GetDataAtOffsetIndex(iv->offset, index, contentPart);
}

void DatabaseQueryManager::getContentPartByIdList(const std::vector<std::string>& idList,
                                                  DatabaseQueryManager::ContentPartType contentPartType,
                                                  std::vector<IdDataPart>* idContentPartList) const
{
    if (!checkIndexByContentPartType(contentPartType))
        return;

    vector<const IndexValue*> infoList;
    getInfoByIdList(idList, &infoList);

    if (infoList.empty()) {
        fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
        return;
    }
    /* get all binId of data files needed to be opened */
    std::unordered_map<uint32_t, std::vector<const IndexValue*>> indexByBinId;
    for (const IndexValue* info : infoList) {
        indexByBinId[info->binId].push_back(info);
    }
    idContentPartList->reserve(idList.size());

    for (const auto& [binId, ivList]: indexByBinId) {
        DataRecordFile dataFile;
        std::string binName = getBinFilenameWithBinId(binId);
        dataFile.readFromFile(binName.c_str());
        /* read whole request if more than 1/3 of records must be accessed  */
        if (ivList.size() > dataFile.numRecords() / 3)
            dataFile.readFromFileFull(binName.c_str());

        for (const IndexValue* iv : ivList) {
            idContentPartList->emplace_back();
            uint32_t index = getIndexByContentPartType(iv, contentPartType);
            // note: no need to check for INVALID since we already checked at the beginning of this function
            if (!dataFile.GetDataAtOffsetIndex(iv->offset, index, &idContentPartList->back().dataPart))
                continue;
            idContentPartList->back().pid = iv->pid;
            idContentPartList->back().aid = iv->aid;
        }
    }
}



/*******************************************************
 *      DatabaseQueryManager Version 2                 *
 *******************************************************/

DatabaseQueryManagerV2::DatabaseQueryManagerV2(
        const char* indexFilename,
        const char* dataPath,
        const char* dataFilePrefix) :

        indexFilename_(indexFilename),
        dataPath_(dataPath),
        dataFilePrefix_(dataFilePrefix),
        indexTable_(indexFilename),
        pidTable_(indexTable_.pid2Index()),
        aidTable_(indexTable_.aid2Index()),
        idOutputQueue_(MAX_PC_QUEUE_SIZE, WRITE_AHEAD, N_CONSUMERS),
        allThreadFinished_(N_CONSUMERS + 1)
{
    // initialize all dataRecordFile's corresponding to the bin IDs
    for (auto [binId, _] : indexTable_.binId2Index())
        dataRecordFileByBinId_.emplace(binId, getBinFilenameWithBinId(binId).c_str());

    DataRecordFileReaderThread::setThreadCount(N_CONSUMERS);
    readerThreadPool_.add<N_CONSUMERS, DataRecordFileReaderThread>(*this);
    readerThreadPool_.runAll();
}

void DatabaseQueryManagerV2::getAllId(vector<string>* pidList, vector<string>* aidList) const
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

bool DatabaseQueryManagerV2::getContentById(const char* id, DataRecordV2* dataRecord) const
{
    const IndexValue* iv;
    if (!(iv = getInfoById(id))) {
        fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                __FUNCTION__, id);
        return false;
    }

    const DataRecordFileReader& dataRecordFile = dataRecordFileByBinId_.at(iv->binId);

    return dataRecordFile.getDataRecordAtOffset(iv->offset, dataRecord);
}

const IndexValue* DatabaseQueryManagerV2::getInfoById(const char* id) const
{
    if (pidTable_.find(id) != pidTable_.end()) {
        return pidTable_.at(id);
    }
    if (aidTable_.find(id) != aidTable_.end()) {
        return aidTable_.at(id);
    }
    // id not found
    fprintf(stderr, "%s: ID [%s] does not exist in database\n",
            __FUNCTION__, id);

    return nullptr;
}


void DatabaseQueryManagerV2::getInfoByIdList(
        const std::vector<std::string>& idList, std::vector<const IndexValue*>* output) const
{
    for (const auto& id : idList) {
        const IndexValue* iv;
        if (!(iv = getInfoById(id.c_str())))
            continue;
        output->push_back(iv);
    }
}

void DatabaseQueryManagerV2::getContentByIdList(const vector<string>& idList,
        vector<shared_ptr<IdDataRecord>>* idDataRecordList) const
{
    vector<const IndexValue*> infoList;
    getInfoByIdList(idList, &infoList);

    if (infoList.empty()) {
        fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
        return;
    }
    /* get all binId of data files needed to be opened */
    std::unordered_map<uint32_t, std::vector<const IndexValue*>> indexByBinId;
    for (const IndexValue* info : infoList) {
        indexByBinId[info->binId].push_back(info);
    }
    idDataRecordList->reserve(idList.size());

    // TODO: optimize for caching
    for (const auto& [binId, ivList]: indexByBinId) {
        DataRecordFile dataFile;
        std::string binName = getBinFilenameWithBinId(binId);
        dataFile.readFromFile(binName.c_str());
        /* read whole request if more than 1/3 of records must be accessed  */
        if (ivList.size() > dataFile.numRecords() / 3)
            dataFile.readFromFileFull(binName.c_str());

        for (const IndexValue* iv : ivList) {
            idDataRecordList->emplace_back(new IdDataRecord);
            if (!dataFile.GetDataRecordAtOffset(iv->offset,
                                                &idDataRecordList->back()->dataRecord))
                continue;
            idDataRecordList->back()->pid = iv->pid;
            idDataRecordList->back()->aid = iv->aid;
        }
    }
}

bool DatabaseQueryManagerV2::getContentPartById(const char* id, ContentPartType cpt, std::string* contentPart) const
{
    const IndexValue* iv;
    if (!(iv = getInfoById(id))) {
        fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                __FUNCTION__, id);
        return false;
    }

    std::string binFilename = getBinFilenameWithBinId(iv->binId);

    DataRecordFile dataRecordFile;
    dataRecordFile.readFromFile(binFilename.c_str());

    uint32_t index = getIndexByContentPartType(iv, cpt);
    if (index == INVALID)
        return false;

    return dataRecordFile.GetDataAtOffsetIndex(iv->offset, index, contentPart);
}

void DatabaseQueryManagerV2::getContentPartByIdList(const std::vector<std::string>& idList,
                                                  DatabaseQueryManagerV2::ContentPartType contentPartType,
                                                  std::vector<IdDataPart>* idContentPartList) const
{
    if (!checkIndexByContentPartType(contentPartType))
        return;

    vector<const IndexValue*> infoList;
    getInfoByIdList(idList, &infoList);

    if (infoList.empty()) {
        fprintf(stderr, "%s: no IDs found\n", __FUNCTION__);
        return;
    }
    /* get all binId of data files needed to be opened */
    std::unordered_map<uint32_t, std::vector<const IndexValue*>> indexByBinId;
    for (const IndexValue* info : infoList) {
        indexByBinId[info->binId].push_back(info);
    }
    idContentPartList->reserve(idList.size());

    for (const auto& [binId, ivList]: indexByBinId) {
        DataRecordFile dataFile;
        std::string binName = getBinFilenameWithBinId(binId);
        dataFile.readFromFile(binName.c_str());
        /* read whole request if more than 1/3 of records must be accessed  */
        if (ivList.size() > dataFile.numRecords() / 4)
            dataFile.readFromFileFull(binName.c_str());

        for (const IndexValue* iv : ivList) {
            idContentPartList->emplace_back();
            uint32_t index = getIndexByContentPartType(iv, contentPartType);
            // note: no need to check for INVALID since we already checked at the beginning of this function
            if (!dataFile.GetDataAtOffsetIndex(iv->offset, index, &idContentPartList->back().dataPart))
                continue;
            idContentPartList->back().pid = iv->pid;
            idContentPartList->back().aid = iv->aid;
        }
    }
}

/*void DatabaseQueryManagerV2::getContentByPidList(unordered_map<string, shared_ptr<DataRecordV2>>& dataRecordById)
{
    tqdm bar;
    int i = 0;
    for (auto& [pid, dr] : dataRecordById)
    {
        idOutputQueue_.emplace_push(pair{pid, dr.get()});
        bar.progress(++i, dataRecordById.size());
    }
    bar.finish();
    // finish producing inout for reader threads
    idOutputQueue_.setQuitSignal();

    // blocks until all threads finishes
    allThreadFinished_.wait();

    // resets queue for next run
    idOutputQueue_.reset();
}*/

void DatabaseQueryManagerV2::getContentByPidList(unordered_map<string, shared_ptr<DataRecordV2>>& dataRecordById)
{
    tqdm bar;
    int i = 0;
    for (const auto& [pid, dr] : dataRecordById)
    {
        idOutputQueue_.emplace_push(pair{pid, dr.get()});
        bar.progress(++i, dataRecordById.size());
    }
    bar.finish();
    // finish producing inout for reader threads
    idOutputQueue_.setQuitSignal();

    // blocks until all threads finishes
    allThreadFinished_.wait();

    // resets queue for next run
    idOutputQueue_.reset();
}

DatabaseQueryManagerV2::~DatabaseQueryManagerV2()
{
    readerThreadExit = true;

    idOutputQueue_.setQuitSignal();

    readerThreadPool_.waitAll();
}

DatabaseQueryManagerV2::DataRecordFileReaderThread::DataRecordFileReaderThread(
        DatabaseQueryManagerV2& databaseQueryManager) :
        dqm_(databaseQueryManager), threadId_(nextThreadId_++),
        ivOutputVecByBinId_(dqm_.dataRecordFileByBinId_.size())
{
    if (threadCount_ == 0)
        PERROR("call setThreadCount() before constructor call!");
    // add the current object into allThreads_
    allThreads_[threadId_] = this;

    // compute binIdRange
    auto avg = dqm_.indexTable_.binId2Index().size() / threadCount_;
    auto rmd = dqm_.indexTable_.binId2Index().size() % threadCount_;
    if (threadId_ < rmd)
        binIdRange_ = {(avg + 1) * threadId_, (avg + 1) * (threadId_ + 1)};
    else {
        binIdRange_.first = (avg + 1) * rmd + avg * (threadId_ - rmd);
        binIdRange_.second = binIdRange_.first + avg;
    }
}


void DatabaseQueryManagerV2::DataRecordFileReaderThread::internalRun()
{
    for (;;)
    {
        // when idQueue is empty, thread is blocked here
        auto [idOutput, quit] = dqm_.idOutputQueue_.pop();

        if (quit) {
            // merge all threads' count table
            merge();
            // request data based on the count
            requestData();

            // quit routine
            if (dqm_.readerThreadExit) break;

            // reset routine
            dqm_.allThreadFinished_.wait();
            reset();
            continue;
        }

        if (!idOutput.second)
            PERROR("output field of idOutput is null");

        const IndexValue* iv;
        if (!(iv = dqm_.getInfoById(idOutput.first.c_str()))) {
            fprintf(stderr, "%s: ID [%s] does not exist in database\n",
                    __FUNCTION__, idOutput.first.c_str());
            continue;
        }

        ivOutputVecByBinId_.at(iv->binId).emplace_back(iv, idOutput.second);
//        dqm_.dataRecordFileByBinId_.at(iv->binId).getDataRecordAtOffset(iv->offset, idOutput.second);
    }
}

void DatabaseQueryManagerV2::DataRecordFileReaderThread::reset()
{
    // resets all counts to 0
    for (auto& idOutputVec: ivOutputVecByBinId_)
        idOutputVec.clear();
}

void DatabaseQueryManagerV2::DataRecordFileReaderThread::merge()
{
    for (uint32_t i = binIdRange_.first; i < binIdRange_.second; i++) {
        for (uint32_t j = 0; j < threadCount_; j++) {
            if (j == threadId_) continue;
            // move other threads' ids vectors into current thread's for the given binIdRange
            move(allThreads_[j]->ivOutputVecByBinId_[i].begin(), allThreads_[j]->ivOutputVecByBinId_[i].end(),
                 back_inserter(ivOutputVecByBinId_[i]));
            allThreads_[j]->ivOutputVecByBinId_[i].clear();
        }
    }
}

void DatabaseQueryManagerV2::DataRecordFileReaderThread::setThreadCount(uint32_t threadCount)
{
    threadCount_ = threadCount;
    allThreads_.resize(threadCount_);
}

void DatabaseQueryManagerV2::DataRecordFileReaderThread::requestData()
{
    constexpr int MAGIC_NUM = 5;
    for (uint32_t i = binIdRange_.first; i < binIdRange_.second; i++) {
        // greater than division of MAGIC_NUM, then we read the whole file first then retrieve the data
        const bool readWholeFile =
                ivOutputVecByBinId_[i].size() > dqm_.dataRecordFileByBinId_.at(i).numRecords() / MAGIC_NUM;
        cout << dqm_.dataRecordFileByBinId_.at(i).numRecords() << '\n';
        if (readWholeFile) {
            dqm_.dataRecordFileByBinId_.at(i).loadAllData();
        }
        for (auto [iv, output] : ivOutputVecByBinId_[i]) {
            dqm_.dataRecordFileByBinId_.at(i).getDataRecordAtOffset(iv->offset, output);
        }

        if (readWholeFile)
            dqm_.dataRecordFileByBinId_.at(i).freeAllData();
    }
}

