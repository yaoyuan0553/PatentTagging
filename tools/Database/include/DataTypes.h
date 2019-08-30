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
#include <array>
#include <map>
#include <iostream>
#include <set>
#include <atomic>

// WARNING: Non-portable code
/* for linux file I/O */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <Utility.h>
#include "Helper.h"
#include "DataBasicTypes.h"


#define HARD_CODED_DATA_FIELDS 4
#define HARD_CODED_INDEX_FIELDS 4


// size includes null-termination \0
constexpr int APPLICATION_ID_SIZE = 20;
constexpr int PUBLICATION_ID_SIZE = 25;

/* format: \L\d\d\L-\d{1-3}/\d{2}*/
constexpr int CLASSIFICATION_SIZE = 21;

constexpr char DATA_FILE_PREFIX_NAME[] = "patent-data";

//using DataRecord = std::unordered_map<std::string, std::vector<std::string>>;
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

class IndexEntry : public std::pair<IndexKey, IndexValue*>, public Stringifiable {
public:
    inline std::string stringify() const final
    {
        return first + '\t' + second->stringify();
    }

};


/* Index table with a specific key
 * table value are pointers pointing to
 * data stored elsewhere */
class IndexTableWithSpecificKey :
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



/* Index Table configurable with multiple types of keys
 * to lookup data value */
class IndexTable : public FileReadWritable {
public:
    // only supports PID and AID for now
    enum IndexKeyType {
        PID, AID, APPDATE, TITLE, IPC
    };

private:
    /* data storage */
    std::vector<IndexValue*> valList_;

    /* hash tables for retrieving data */
    std::map<IndexKeyType, IndexTableWithSpecificKey> indexTables_;

    /* hash table for storing IndexValue corresponding to bin files */
    std::unordered_map<uint32_t, std::vector<IndexValue*>> indexByBinId_;

    inline void insertValueToTables(IndexValue* iv)
    {
        // TODO: implement construction of other hash tables
        for (auto& [kt, table] : indexTables_) {
            switch (kt)
            {
                case PID:
                    table[iv->pid] = iv;
                    break;
                case AID:
                    table[iv->aid] = iv;
                    break;
                default:
                    PERROR("the key type is not supported yet!");
            }
        }
    }

public:

    explicit IndexTable(const std::vector<IndexKeyType>& ikeyTypes)
    {
        for (IndexKeyType kt : ikeyTypes)
            indexTables_[kt] = IndexTableWithSpecificKey();
    }

    ~IndexTable() final
    {
        for (IndexValue* iv : valList_)
            delete iv;
    }

    inline const IndexTableWithSpecificKey& operator[](IndexKeyType kt)
    {
        return indexTables_[kt];
    }

    inline size_t numRecords() const { return valList_.size(); }

    inline std::vector<IndexValue*>& indexValueList() { return valList_; }

    inline const std::vector<IndexValue*>& indexValueList() const { return valList_; }

    inline const std::vector<IndexValue*>& getIndexValueListByBinId(uint32_t binId)
    {
        return indexByBinId_[binId];
    }

    void reserve(size_t n);

    /* append new value to data tables
     * caller must initialize IndexValue inside
     * the callback function ivInitFunc */
    template <typename IvInitFunc>
    void appendIndexValue(IvInitFunc&& ivInitFunc)
    {
        static_assert(std::is_invocable_v<IvInitFunc, IndexValue*>,
                "callback function must of type void(*)(IndexValue*)");

        valList_.push_back(new IndexValue);
        ivInitFunc(valList_.back());

        insertValueToTables(valList_.back());
    }


    void readFromFile(const char* filename) final;

    void writeToFile(const char* filename) final;
};

//#ifndef __cplusplus
//extern "C"
//{
typedef struct __DataRecordCType {
    uint32_t size = 0;
    uint32_t ts = 0, as = 0, cs = 0, ds = 0;
    char* title = nullptr, *abstract = nullptr,
            *claim = nullptr, *description = nullptr;
//    std::string title, abstract, claim, description;
} DataRecordCType;
//}

typedef struct __IdDataRecordCType {
    uint32_t size = 0;
    uint32_t ts = 0, as = 0, cs = 0, ds = 0;
    char* title = nullptr, *abstract = nullptr,
            *claim = nullptr, *description = nullptr;
    char* pid = nullptr, *aid = nullptr;
//    std::string title, abstract, claim, description;
} IdDataRecordCType;

struct DataRecord;

extern void ConvertToDataRecordCType(DataRecordCType* drct, DataRecord* dataRecord);

//#endif



/* Data table data layout
 * -------------------------------------------------------------
 * | real bytes of data | number of records | data records...  |
 * -------------------------------------------------------------
 * */

/* storing of Data Record File in memory
 * in charge of writing/reading file to/from disk */
class DataRecordFile : public FileReadWritable {

    /* accumulate number of data record files created
     * used to assign unique number for each object
     * created */
    inline static std::atomic_uint32_t nextBinId_ = 0;

    /* data file unique number for the current object
     * assigned upon construction */
    uint32_t binId_;

    /* immutable pointer pointing to start of buffer */
    char* const buf_;
    /* references pointing to bytes and record count stored in buffer */
    uint64_t& nBytesWritten_;
    uint32_t& nRecordsWritten_;

    /* points to the current location of writing */
    char* curBuf_;

    std::vector<IndexValue*> indexSubTable_;

    inline void incrementBy(size_t size)
    {
        curBuf_ += size;
        nBytesWritten_ += size;
    }

    int fileHandle_ = -1;

public:
    inline static constexpr size_t MAX_FILE_SIZE = 1 << 30;   // 1 GB file size limit

    inline static constexpr auto FILE_HEAD_SIZE =
            sizeof(decltype(nBytesWritten_)) +
            sizeof(decltype(nRecordsWritten_));

    DataRecordFile() : buf_(new char[MAX_FILE_SIZE]),
            nBytesWritten_(*(uint64_t*)buf_),
            nRecordsWritten_(*(uint32_t*)(buf_ + sizeof(decltype(nBytesWritten_))))
    {
        clear();
    }

    ~DataRecordFile() final
    {
        if (fileHandle_ != -1)
            close(fileHandle_);
        delete[] buf_;
        for (IndexValue* iv : indexSubTable_)
            delete iv;
    }

    /* total bytes allocated */
    inline uint32_t capacity() const { return MAX_FILE_SIZE; }

    /* total bytes of data written to the buffer */
    inline uint32_t bytesWritten() const { return nBytesWritten_; }

    /* current number of records in the buffer */
    inline uint32_t numRecords() const { return nRecordsWritten_; }

    /* returns true if number of records is 0 */
    inline bool empty() const { return nRecordsWritten_ == 0; }

    /* return index sub table */
    inline const std::vector<IndexValue*>& indexSubTable() const { return indexSubTable_; }

    /* return generate filename with a prefix (e.g. input: data, output data_12.bin) */
    inline const std::string generateFilename(std::string_view prefix) const
    {
        return std::string(prefix) + "_" + std::to_string(binId_) + ".bin";
    }

    /* clearing buffer of data and reset other fields */
    void clear();

    /* write buffer to file */
    void writeToFile(const char* filename) final;

    /* read buffer from file (doesn't not read the whole file)*/
    void readFromFile(const char* filename) final;

    /* read record segment at offset from disk
     * returns false if read failed
     * returns true on success */
    bool readRecordFromFile(uint64_t offset) const;

    void readFromFileFull(const char* filename);

    // TODO: hacky implementation, to be changed later
    void writeSubIndexTableToFile(const char* filename);
    void writeSubIndexTableToStream(std::ostream& os);

    /* appends a record into buffer
     * returns false if new record is too large for the buffer
     * true if append succeeds */
    bool appendRecord(const std::vector<std::string>& dataText,
            const std::vector<std::string>& indexText,
            const uint32_t* recordSize = nullptr);


    /* retrieves a copy of record data at given offset, stored in dataRecord pointer
     * returns false if offset is invalid, and true on success */
    bool GetDataRecordAtOffset(uint64_t offset, DataRecord* dataRecord) const;

    bool GetDataAtOffsetIndex(uint64_t offset, uint32_t index, std::string* data) const;

    bool GetDataRecordAtOffset(uint64_t offset, IdDataRecordCType* dataRecord) const;
};


#endif //TOOLS_DATATYPES_H
