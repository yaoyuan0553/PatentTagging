//
// Created by yuan on 8/13/19.
//

#include "DataTypes.h"

#include <Utility.h>
#include <TagConstants.h>

#include <fstream>

using namespace std;

void DataRecordFile::clear()
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

bool DataRecordFile::appendRecord(const vector<string>& formattedText, uint32_t recordSize)
{
    if (recordSize == 0) {
        recordSize = sizeof(uint32_t) + sizeof(uint32_t) * formattedText.size();
        for (const auto& t : formattedText)
            recordSize += t.length();
    }

    if (bytesWritten() + recordSize > capacity())
        return false;

    *(uint32_t*)curBuf_ = recordSize;
    incrementBy(sizeof(uint32_t));
    for (const auto& t : formattedText) {
        *(uint32_t*) curBuf_ = t.length();
        incrementBy(sizeof(uint32_t));
        memcpy(curBuf_, t.c_str(), t.length());
        incrementBy(t.length());
    }
    nRecordsWritten_++;

    return true;
}

void DataRecordFile::readFromFile(const char* filename)
{
    clear();

    FILE* filePtr = fopen(filename, "rb");

    if (fread(buf_, MAX_FILE_SIZE, 1, filePtr) != 1)
        PERROR("fread()");

    nBytesWritten_ = MAX_FILE_SIZE; // make full so no-one appends new records without clearing

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

void IndexTable::readFromFile(const char* filename)
{
    if (!valList_.empty())
        PERROR("valList must be empty before reading from file");

    ifstream ifs((string(filename)));
    if (!ifs.is_open()) {
        fprintf(stderr, "file [%s] failed to open\n", filename);
        PERROR("ifstream open");
    }

    for (string line; getline(ifs, line);) {
        istringstream ss(line);
        auto* iv = new IndexValue;
        ss >> *iv;

        valList_.push_back(iv);

        insertValueToTables(iv);
    }
}

void IndexTable::writeToFile(const char* filename)
{
    ofstream ofs((string(filename)));
    if (!ofs.is_open()) {
        fprintf(stderr, "file [%s] failed to open\n", filename);
        PERROR("ofstream file open");
    }

    for (IndexValue* val : valList_)
        ofs << *val << '\n';

    ofs.close();
}

void IndexTable::reserve(size_t n)
{
    valList_.reserve(n);
    for (auto& [_, table] : indexTables_)
        table.reserve(n);
}

ostream& operator<<(std::ostream& os, const IndexValue& ie)
{
    os << ie.stringify();

    return os;
}

istream& operator>>(std::istream& is, IndexValue& ie)
{
    // WARNING: It is presumed there is a SINGLE tab delimiting here!!!
    string field;

    getline(is, ie.pid, '\t');
    getline(is, ie.aid, '\t');
    getline(is, ie.appDate, '\t');

/* helper macro for repetitive code */
#define GET_UINT32(name)            \
    do {                            \
        getline(is, field, '\t');   \
        ie.name = stoi(field);      \
    } while(0)

    GET_UINT32(binId);
    GET_UINT32(offset);
    GET_UINT32(ti);
    GET_UINT32(ai);
    GET_UINT32(ci);
    GET_UINT32(di);

    getline(is, field, '\t');

    ie.ipcList.clear();
    stringstream ss(field);
    for (string ipc; getline(ss, ipc, ',');)
        ie.ipcList.push_back(ipc);

    return is;
}

#undef GET_UINT32
