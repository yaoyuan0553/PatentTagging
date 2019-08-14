//
// Created by yuan on 8/13/19.
//

#pragma once
#ifndef TOOLS_DATATYPES_H
#define TOOLS_DATATYPES_H


#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <map>

#include <Utility.h>


// TODO: find out the correct Application & Publication & Classifiaciton length
// size includes null-termination \0
constexpr int APPLICATION_ID_SIZE = 20;
constexpr int PUBLICATION_ID_SIZE = 25;

/* format: \L\d\d\L-\d{1-3}/\d{2}*/
constexpr int CLASSIFICATION_SIZE = 21;


struct Stringifiable {
    virtual std::string stringify() const = 0;
    virtual ~Stringifiable() = 0;
};


using DataRecord = std::unordered_map<std::string, std::vector<std::string>>;
/* need:
 * application-id,
 * publication-id,
 * classification,
 * application-date,
 * title,
 * abstract,
 * claim,
 * description
 * */


/* application-id / publication-id as key of the index table */
using IndexKey = std::string;

using ClassificationString = std::array<char, CLASSIFICATION_SIZE>;

/* pack data size as it is */
/*
#pragma pack(push, 1)
*/
/* this struct defines how each index value is represented in memory and in disk
 * ------------------------------
 * |        field        | bytes |
 * |        datId        |  4    |
 * |    applicationId    |  AIS  |
 * |    publicationId    |  PIS  |
 * |        ti           |  4    |
 * |        ai           |  4    |
 * |        ci           |  4    |
 * |        di           |  4    |
 * |     classCount      |  2    |
 * |   classifications   |  VAR  |
 * -------------------------------
 * where AIS is short for APPLICATION_ID_SIZE
 * PIS is PUBLICATION_ID_SIZE
 * VAR stands for variable length, in this case
 * VAR = classCount * CLASSIFICATION_SIZE
 * *//*

struct IndexValue : public Stringifiable {
private:
    */
/* we ignore this field when packing data into buffer and deep copy
     * at where it's pointed *//*

    std::vector<ClassificationString>*  classifications_;                    // 8 bytes
public:
    uint32_t                            datId;                              // 4 bytes
    char                                applicationId[APPLICATION_ID_SIZE]; // APPLICATION_ID_SIZE bytes
    char                                publicationId[PUBLICATION_ID_SIZE]; // PUBLICATION_ID_SIZE bytes
    uint32_t                            ti, ai, ci, di;                     // 16 bytes
    uint16_t                            classCount;                         // 2 bytes

    inline static constexpr int         INDEX_VALUE_STATIC_SIZE =
            sizeof(uint32_t) +
            sizeof(applicationId) / sizeof(char) +
            sizeof(publicationId) / sizeof(char) +
            sizeof(uint32_t) * 4 + sizeof(uint16_t);

    std::vector<ClassificationString>& classifications() const { return *classifications_; }

    IndexValue() : classifications_(new std::vector<ClassificationString>) { }

    ~IndexValue()
    {
        delete classifications_;
    }

    inline std::string stringify() const final
    {
        std::string str(
                std::to_string(datId) + '\t' +
                applicationId + '\t' +
                publicationId + '\t' +
                std::to_string(ti) + '\t' +
                std::to_string(ai) + '\t' +
                std::to_string(ci) + '\t' +
                std::to_string(di) + '\t' +
                std::to_string(classCount));
        for (const auto& cls : classifications()) {
            str += cls.data();
            str += ',';
        }
        str.back() = '\n';
        return str;
    }

    */
/* load IndexValue from buffer (disk) *//*

    inline void load(char* buffer)
    {
        memcpy((char*)this + sizeof(void*), buffer, INDEX_VALUE_STATIC_SIZE);
        for (short i = 0; i < classCount; i++) {
            ClassificationString clsStr;
            memcpy(clsStr.data(),
                    buffer + INDEX_VALUE_STATIC_SIZE + i * CLASSIFICATION_SIZE,
                    CLASSIFICATION_SIZE);
            classifications_->push_back(clsStr);
        }
    }

    */
/* save IndexValue to buffer
     * returns false if buffer isn't large enough
     * remSize - remaining size of buffer
     * sizeWritten - total written size into buffer *//*

    bool save(char* buffer, const int remSize, int* sizeWritten)
    {
        if (remSize < getTotalBytes())
            return false;

        *sizeWritten = 0;
        memcpy(buffer, (char*)this + sizeof(void*), INDEX_VALUE_STATIC_SIZE);
        char* clsArray = buffer + INDEX_VALUE_STATIC_SIZE;
        for (short i = 0; i < classCount; i++) {
            memcpy(clsArray + i * CLASSIFICATION_SIZE,
                    (*classifications_)[i].data(), CLASSIFICATION_SIZE);
        }

        *sizeWritten += getTotalBytes();

        return true;
    }

    inline constexpr int getTotalBytes() const
    {
        return INDEX_VALUE_STATIC_SIZE + classCount * CLASSIFICATION_SIZE;
    }
};

#pragma pack(pop)

*/

struct IndexValue : public Stringifiable {
    std::string                 pid, aid, appDate;
    uint32_t                    binId, offset, ti, ai, ci, di;
    std::vector<std::string>    ipcList;

    inline std::string stringify() const final
    {
        std::string str(
                pid +'\t' + aid + '\t' + appDate + '\t' +
            std::to_string(binId) + '\t' + std::to_string(offset) + '\t' +
            std::to_string(ti) + '\t' + std::to_string(ai) + '\t' +
            std::to_string(ci) + '\t' + std::to_string(di) + '\t');
        for (const auto& ipc : ipcList)
            str += ipc + ',';
        str.pop_back();

        return str;
    }
};


class IndexEntry : public std::pair<IndexKey, IndexValue*>, public Stringifiable {
public:
    inline std::string stringify() const final
    {
        return first + '\t' + second->stringify();
    }
};


class IndexTable :
        public std::unordered_map<IndexKey, IndexValue*,
            std::hash<IndexKey>, std::equal_to<IndexKey>, std::allocator<IndexEntry>>,
        public Stringifiable {
public:

    inline std::string stringify() const final
    {
        std::string str;

        for (const auto& [key, val]: *this)
            str += val->stringify() + '\n';

        return str;
    }
};


/* stores pointer, user of this struct
 * must not deallocate DataRecordFile before reading */
struct DataRecordEntry {
    uint32_t    size;
    uint32_t    ts, as, cs, ds;
    char*       title;
    char*       abstract;
    char*       claim;
    char*       description;
};


/* storing of Data Record File in memory
 * in charge of writing/reading file to/from disk */
class DataRecordFile {
    inline static constexpr uint32_t MAX_FILE_SIZE = 1 << 30;   // 1 GB file size limit

    char* buf_;
    /* points to the current location of writing */
    char* curBuf_;
    uint32_t nBytesWritten_;
    uint32_t nRecordsWritten_;

public:

    DataRecordFile() : buf_(new char[MAX_FILE_SIZE])
    {
        reset();
    }

    ~DataRecordFile()
    {
        delete[] buf_;
    }

    inline uint32_t capacity() const { return MAX_FILE_SIZE; }

    inline uint32_t bytesWritten() const { return nBytesWritten_; }

    inline uint32_t numRecords() const { return nRecordsWritten_; }

    void reset();

    void writeToFile(const char* filename);

    void readFromFile(const char* filename);

    bool appendRecord(uint32_t recordSize, const std::vector<std::string>& formattedText);

    DataRecordEntry GetRecordAtOffset(uint32_t offset);
};


#endif //TOOLS_DATATYPES_H
