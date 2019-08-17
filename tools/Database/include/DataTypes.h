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
#include <iostream>
#include <set>
#include <atomic>

#include <Utility.h>
#include "Helper.h"


#define HARD_CODED_DATA_FIELDS 4
#define HARD_CODED_INDEX_FIELDS 4


// size includes null-termination \0
constexpr int APPLICATION_ID_SIZE = 20;
constexpr int PUBLICATION_ID_SIZE = 25;

/* format: \L\d\d\L-\d{1-3}/\d{2}*/
constexpr int CLASSIFICATION_SIZE = 21;

constexpr char DATA_FILE_PREFIX_NAME[] = "patent-data";


/********************************/
/*    abstract interface types  */
/********************************/

struct Stringifiable {
    virtual std::string stringify() const = 0;
    virtual ~Stringifiable() = default;
};

struct FileReadWritable {
    virtual void readFromFile(const char* filename) = 0;
    virtual void writeToFile(const char* filename) = 0;
    virtual ~FileReadWritable() = default;
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

    inline static const std::string header = ConcatStringWithDelimiter("\t",
            "Publication ID", "Application ID", "Application Date",
            "Classification IPC", "Bin ID", "Offset", "Title Index",
            "Abstract Index", "Claim Index", "Description Index");

    std::string                 pid, aid, appDate, ipc;
    uint32_t                    binId, ti, ai, ci, di;
    uint64_t                    offset;

//    inline std::string header() const
//    {
//        return ConcatStringWithDelimiter("\t",
//                "Publication ID", "Application ID", "Application Date",
//                "Classification IPC", "Bin ID", "Offset", "Title Index",
//                "Abstract Index", "Claim Index", "Description Index");
//    }

    inline std::string stringify() const final
    {
        using std::to_string;

        return ConcatStringWithDelimiter("\t", pid, aid, appDate, ipc,
                to_string(binId), to_string(offset), to_string(ti),
                to_string(ai), to_string(ci), to_string(di));
    }

    /* for file I/O */
    friend std::ostream& operator<<(std::ostream& os, const IndexValue& ie);
    friend std::istream& operator>>(std::istream& is, IndexValue& ie);
};


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

    inline IndexTableWithSpecificKey& operator[](IndexKeyType kt)
    {
        return indexTables_[kt];
    }

    inline size_t numRecords() const { return valList_.size(); }

    inline std::vector<IndexValue*>& indexValueList() { return valList_; }

    inline const std::vector<IndexValue*>& indexValueList() const { return valList_; }

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

    void readFromFile(const char* filename) final;

    void writeToFile(const char* filename) final;
};


/* stores pointer, user of this struct
 * must NOT deallocate DataRecordFile before reading */
struct DataRecordEntry {
    /* total size of this record */
    uint32_t    size;
    /* size of each string field */
    uint32_t    ts, as, cs, ds;
    /* not these strings are NOT null-terminated
     * their sizes are indicated by ts, as, cs, ds */
    char*       title;
    char*       abstract;
    char*       claim;
    char*       description;
};

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

    /* read buffer from file */
    void readFromFile(const char* filename) final;

    // TODO: hacky implementation, to be changed later
    void writeSubIndexTableToFile(const char* filename);
    void writeSubIndexTableToStream(std::ostream& os);

    /* appends a record into buffer
     * returns false if new record is too large for the buffer
     * true if append succeeds */
    bool appendRecord(const std::vector<std::string>& dataText,
            const std::vector<std::string>& indexText,
            const uint32_t* recordSize = nullptr);

    /* returns a DataRecordEntry pointing at the offset
     * caller is responsible for the correctness of the offset */
    DataRecordEntry GetRecordAtOffset(uint32_t offset);
};


#endif //TOOLS_DATATYPES_H
