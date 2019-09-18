//
// Created by yuan on 9/2/19.
//

#include "DataRecordFileV2.h"

using namespace std;

DataRecordFileV2::DataRecordFileV2(uint64_t sizeToAllocate) :
        buf_((char*)checkMalloc(max(sizeToAllocate, FILE_HEAD_SIZE))),
        capacity_(max(sizeToAllocate, FILE_HEAD_SIZE)),
        nBytes_(*(uint64_t*)buf_),
        nRecords_(*(uint32_t*)(buf_ + sizeof(decltype(nBytes_))))
{
    nBytes_ = FILE_HEAD_SIZE;   // only 12 bytes are used
    nRecords_ = 0;
}

DataRecordFileV2::~DataRecordFileV2()
{
    free(buf_);
}

void DataRecordFileV2::reserve(size_t newSize)
{
    // if not larger, do nothing
    if (newSize <= capacity_) return;

    buf_ = (char*)realloc(buf_, newSize);
    if (!buf_)
        PSYS_FATAL("realloc()");
}

void DataRecordFileV2::shrink(size_t newSize)
{
    // if not smaller, do nothing
    if (newSize >= capacity_) return;

    // can't be smaller than file head
    newSize = max(newSize, FILE_HEAD_SIZE);

    buf_ = (char*)realloc(buf_, newSize);
    if (!buf_)
        PSYS_FATAL("realloc()");
}

DataRecordFileWriter::DataRecordFileWriter(size_t maxFileSize) :
        DataRecordFileV2(maxFileSize), curBuf_(buf()), maxFileSize_(maxFileSize)
{
    obtainBinId();
}

void DataRecordFileWriter::writeToFile(const char*)
{

}


/***********************************************
 *          DataRecordFileReader               *
 ***********************************************/

DataRecordFileReader::DataRecordFileReader(const char* dataFilename) :
        DataRecordFileV2(), filename_(dataFilename)
{
    openDataFile();
}

void DataRecordFileReader::openDataFile()
{
    fileHandle_ = open(filename_.c_str(), O_RDONLY);
    if (fileHandle_ == -1)
        PSYS_FATAL("failed to open (open()) [%s]", filename_.c_str());

    // note: pread() doesn't change the file pointer offset
    if (pread(fileHandle_, buf(), FILE_HEAD_SIZE, 0) == -1)
        PSYS_FATAL("error reading (pread()) header of file [%s]", filename_.c_str());
}

DataRecordFileReader::~DataRecordFileReader()
{
    closeDataFile();
}

void DataRecordFileReader::closeDataFile()
{
    if (close(fileHandle_) == -1)
        PSYS_FATAL("failed to close (close()) [%s]", filename_.c_str());
}

bool DataRecordFileReader::getDataRecordAtOffset(uint64_t offset, DataRecordV2* dataRecord) const
{
    try {
        if (allDataLoaded_)
            *dataRecord = DataRecordV2(buf(), offset, nBytes_);
        else
            *dataRecord = DataRecordV2(fileHandle_, offset, nBytes_);
    }
    catch (ObjectConstructionFailure& ocf) {
        fprintf(stderr, "DataRecord failed to construct: %s", ocf.what());
        return false;
    }
    return true;
}

void DataRecordFileReader::loadAllData()
{
    reserve(numBytes());
    if (pread(fileHandle_, buf() + FILE_HEAD_SIZE, numBytes() - FILE_HEAD_SIZE, FILE_HEAD_SIZE) == -1)
        PSYS_FATAL("error reading (pread()) file [%s]", filename_.c_str());
    allDataLoaded_ = true;
}

void DataRecordFileReader::freeAllData()
{
    shrink(FILE_HEAD_SIZE);
    allDataLoaded_ = false;
}
