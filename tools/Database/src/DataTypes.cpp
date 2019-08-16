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
    /* atomically obtains file id */
    binId_ = nextBinId_.fetch_add(1);

    memset(buf_, 0, MAX_FILE_SIZE);
    curBuf_ = buf_ + FILE_HEAD_SIZE;
    nBytesWritten_ = FILE_HEAD_SIZE;
    nRecordsWritten_ = 0;

    for (IndexValue* iv : indexSubTable_)
        delete iv;
    indexSubTable_.clear();
}

void DataRecordFile::writeToFile(const char* filename)
{
    FILE* filePtr = fopen(filename, "wb");

    fwrite(buf_, MAX_FILE_SIZE, 1, filePtr);

    fclose(filePtr);
}

bool DataRecordFile::appendRecord(const vector<string>& dataText,
        const vector<string>& indexText,
        const uint32_t* recordSize)
{
    uint32_t rsz;
    if (recordSize == nullptr) {
        rsz = sizeof(uint32_t) + sizeof(uint32_t) * dataText.size();
        for (const auto& t : dataText)
            rsz += t.length();
    }
    else
        rsz = *recordSize;

    if (bytesWritten() + rsz > capacity())
        return false;

    /* create new IndexValue */
    auto iv = new IndexValue;

    // TODO: hard coded check
    if (indexText.size() != 4)
        PERROR("hard coded IndexValue doesn't match");
    if (dataText.size() != 4)
        PERROR("hard coded IndexValue doesn't match");

    // TODO: change this, hard-coded for now
    iv->pid = indexText[0];
    iv->aid = indexText[1];
    iv->appDate = indexText[2];
    iv->ipc = indexText[3];
    iv->binId = binId_;
    iv->offset = curBuf_ - buf_;
    iv->ti = sizeof(uint32_t);
    iv->ai = iv->ti + dataText[0].length() + sizeof(uint32_t);
    iv->ci = iv->ai + dataText[1].length() + sizeof(uint32_t);
    iv->di = iv->ci + dataText[2].length() + sizeof(uint32_t);

    /* append to Data File (buf_)*/
    *(uint32_t*)curBuf_ = rsz;
    incrementBy(sizeof(uint32_t));
    for (const auto& t : dataText) {
        *(uint32_t*) curBuf_ = t.length();
        incrementBy(sizeof(uint32_t));
        memcpy(curBuf_, t.c_str(), t.length());
        incrementBy(t.length());
    }
    /* update records written */
    nRecordsWritten_++;

    /* append to index subtable */
    indexSubTable_.push_back(iv);

    return true;
}

void DataRecordFile::readFromFile(const char* filename)
{
    clear();

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
        if ((size_t)(curBuf - buf_) > MAX_FILE_SIZE)    \
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

void DataRecordFile::writeSubIndexTableToFile(const char* filename)
{
    ofstream ofs(filename);
    for (IndexValue* iv : indexSubTable_) {
        ofs << *iv << '\n';
    }
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
    getline(is, ie.ipc, '\t');

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

    return is;
}

#undef GET_UINT32
