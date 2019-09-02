//
// Created by yuan on 9/2/19.
//

#include "DataRecordFileV2.h"

using namespace std;

DataRecordFileV2::DataRecordFileV2(uint64_t sizeToAllocate) :
        buf_(new char[max(sizeToAllocate, FILE_HEAD_SIZE)]),
        nBytes_(*(uint64_t*)buf_),
        nRecords_(*(uint32_t*)(buf_ + sizeof(decltype(nBytes_)))),
        capacity_(sizeToAllocate)
{
    nBytes_ = FILE_HEAD_SIZE;   // only 12 bytes are used
    nRecords_ = 0;
}

DataRecordFileV2::~DataRecordFileV2()
{
    delete[] buf_;
}

DataRecordFileWriter::DataRecordFileWriter(size_t maxFileSize) :
        DataRecordFileV2(maxFileSize), curBuf_(buf_), maxFileSize_(maxFileSize)
{
    obtainBinId();
}

void DataRecordFileWriter::writeToFile(const char* filename)
{

}
