//
// Created by yuan on 9/2/19.
//

#include "DataRecordFileV2.h"


DataRecordFileV2::~DataRecordFileV2()
{
    delete[] buf_;
}

DataRecordFileV2::DataRecordFileV2(uint64_t sizeToAllocate) :
        buf_(new char[sizeToAllocate]),
        nBytes_(*(uint64_t*)buf_),
        nRecords_(*(uint32_t*)(buf_ + sizeof(decltype(nBytes_))))
{
    nBytes_ = FILE_HEAD_SIZE;   // only 12 bytes are used
    nRecords_ = 0;
}


DataRecordFileWriter::DataRecordFileWriter(size_t maxFileSize) :
        DataRecordFileV2(maxFileSize), curBuf_(buf_), maxFileSize_(maxFileSize)
{

}
