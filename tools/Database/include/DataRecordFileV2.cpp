//
// Created by yuan on 9/2/19.
//

#include "DataRecordFileV2.h"

DataRecordFileV2::DataRecordFileV2() :
        buf_(new char[FILE_HEAD_SIZE]),
        nBytes_(*(uint64_t*)buf_),
        nRecords_(*(uint32_t*)(buf_ + sizeof(decltype(nBytes_))))
{
    memset(buf_, 0, FILE_HEAD_SIZE);
}

DataRecordFileV2::~DataRecordFileV2()
{
    delete[] buf_;
}


