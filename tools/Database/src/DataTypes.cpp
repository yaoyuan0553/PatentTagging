//
// Created by yuan on 8/13/19.
//

#include "DataTypes.h"

#include <Utility.h>

using namespace std;

void DataRecordFile::reset()
{
    memset(buf_, 0, MAX_FILE_SIZE);
    curBuf_ = buf_;
    nBytesWritten_ = 0;
    nRecordsWritten_ = 0;
}


void DataRecordFile::writeToFile(const char* filename)
{
    FILE* filePtr = fopen(filename, "wb");

    fwrite(buf_, MAX_FILE_SIZE, 1, filePtr);

    fclose(filePtr);
}

bool DataRecordFile::appendRecord(uint32_t recordSize, const vector<string>& formattedText)
{
    if (bytesWritten() + recordSize > capacity())
        return false;

    *(uint32_t*)curBuf_ = recordSize;
    curBuf_ += sizeof(uint32_t);
    for (const auto& t : formattedText) {
        *(uint32_t*) curBuf_ = t.length();
        curBuf_ += sizeof(uint32_t);
        memcpy(curBuf_, t.c_str(), t.length());
        curBuf_ += t.length();
    }
    nRecordsWritten_++;

    return true;
}

void DataRecordFile::readFromFile(const char* filename)
{
    FILE* filePtr = fopen(filename, "rb");

    if (fread(buf_, MAX_FILE_SIZE, 1, filePtr) != 1)
        PERROR("fread()");

    fclose(filePtr);
}

DataRecordEntry DataRecordFile::GetRecordAtOffset(uint32_t offset)
{
    if (offset > MAX_FILE_SIZE)
        PERROR("offset exceeds file size");

#define APPEND_INCR(sizeMem, bufMem)                    \
    do {                                                \
        dataRecordEntry.sizeMem = *(uint32_t*)curBuf;   \
        curBuf += sizeof(uint32_t);                     \
        dataRecordEntry.bufMem = curBuf;                \
        curBuf += dataRecordEntry.sizeMem;              \
        if (curBuf - buf_ > MAX_FILE_SIZE)              \
            PERROR("pointer exceeds file boundary");    \
    } while(0)

    char* curBuf = buf_ + offset;
    DataRecordEntry dataRecordEntry;
    dataRecordEntry.size = *(uint32_t*)curBuf;
    curBuf += sizeof(uint32_t);
    APPEND_INCR(ts, title);
    APPEND_INCR(as, abstract);
    APPEND_INCR(cs, claim);
    APPEND_INCR(ds, description);

#undef APPEND_INCR

    return dataRecordEntry;
}
