//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASICTYPES_H
#define TOOLS_DATABASICTYPES_H

#include <string>

#include <unistd.h>

#include "Helper.h"
#include "Utility.h"


/**
 * @brief used to interpret & copy-out records stored in data file buffers *
 */
struct DataRecord : Stringifiable {
    /* read only attributes */
    uint32_t size = 0;
    uint32_t ts = 0, as = 0, cs = 0, ds = 0;
    std::string title = "";
    std::string abstract = "";
    std::string claim = "";
    std::string description = "";

    DataRecord() = default;

    DataRecord(
            uint32_t _size,
            uint32_t _ts,
            uint32_t _as,
            uint32_t _cs,
            uint32_t _ds,
            const char* _title,
            const char*_abstract,
            const char* _claim,
            const char* _description) :
            size(_size),
            ts(_ts),
            as(_as),
            cs(_cs),
            ds(_ds),
            title(_title, _title + _ts),
            abstract(_abstract, _abstract + _as),
            claim(_claim, _claim + _cs),
            description(_description, _description + _ds)
    { }

    /* allow move constructor */
    DataRecord(DataRecord&& other) noexcept :
            size(other.size), ts(other.ts),
            as(other.as), cs(other.cs), ds(other.ds),
            title(std::move(other.title)), abstract(std::move(other.abstract)),
            claim(std::move(other.claim)), description(std::move(other.description))
    { }

    /* disable copy constructor */
    DataRecord(const DataRecord&) = delete;

    /* move assignment */
    DataRecord& operator=(DataRecord&& other) noexcept;

    /* disable copy assigment */
    DataRecord& operator=(DataRecord&) = delete;

    /**
     * @brief pretty print data stored in this structure
     * @return converted string
     */
    inline std::string stringify() const final
    {
        return ConcatStringWithDelimiter("\n",
                "<(title)>: " + title,
                "<(abstract)>: " + abstract,
                "<(claim)>: " + claim,
                "<(description)>: " + description);
    }

    friend std::ostream& operator<<(std::ostream& os, const DataRecord& dataRecord);
};

/**
 * @brief represents a row of record stored in index.tsv files
 */
struct IndexValue : public Stringifiable {

    inline static const std::string header = ConcatStringWithDelimiter("\t",
            "Publication ID", "Application ID", "Application Date",
            "Classification IPC", "Bin ID", "Offset", "Title Index",
            "Abstract Index", "Claim Index", "Description Index");

    std::string                 pid, aid, appDate, ipc;
    uint32_t                    binId, ti, ai, ci, di;
    uint64_t                    offset;

    /**
     * @brief pretty print data stored in this structure
     * @return converted string
     */
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

/**
 * @brief stores PID, AID, and its corresponding DataRecord content
 */
struct IdDataRecord {
    std::string pid;
    std::string aid;
    DataRecord dataRecord;
};

/**
 * @brief stores PID, AID, and its corresponding partial data content
 */
struct IdDataPart {
    std::string pid;
    std::string aid;
    std::string dataPart;
};

struct DataRecordV2 : Stringifiable {

#ifndef SWIG    // for swig parser to skip this function
    template <typename T, typename = std::enable_if_t<std::is_same_v<T, int> || std::is_same_v<T, char*>, void>>
    void decode(T fileHandleOrBuf, uint64_t offset, uint64_t maxOffset)
    {
        // name aliases for better understanding
        T fileHandle = fileHandleOrBuf;
        T buf = fileHandleOrBuf;
        (void)fileHandle;
        (void)buf;

#define IS_FILE std::is_same_v<T, int>
        // T is a file handle type (int)
        if constexpr (IS_FILE) {
            // read in record size first
            if (pread(fileHandle, &recordSize, sizeof(recordSize), offset) == -1)
                throw ObjectConstructionFailure("pread() error");
        }
        else    // T is a buffer type (char*)
            recordSize = *(uint32_t*)(buf + offset);

        // check record size
        if (offset + recordSize > maxOffset)
            throw ObjectConstructionFailure("recordSize exceeds file boundary");

        if constexpr (IS_FILE) {
            buf = new char[recordSize];
        }
        char* curBuf = buf + sizeof(uint32_t);
        if constexpr (IS_FILE) {
            // read the rest of the record data with given record size
            if (pread(fileHandle, buf + sizeof(uint32_t), recordSize - sizeof(uint32_t),
                      offset + sizeof(uint32_t)) == -1) {
                delete[] buf;       // release memory before throwing
                throw ObjectConstructionFailure("pread() error");
            }
        }
#define COPY_AND_INCR(field)                                                \
    do {                                                                    \
        uint32_t size = *(uint32_t*)curBuf;                                 \
        curBuf += sizeof(uint32_t);                                         \
        if (curBuf - buf + (uint64_t)size > maxOffset)                      \
            throw ObjectConstructionFailure("size exceeds file boundary");  \
        field = std::string(curBuf, curBuf + size);                         \
        curBuf += size;                                                     \
    } while (0)

        COPY_AND_INCR(title);
        COPY_AND_INCR(abstract);
        COPY_AND_INCR(claim);
        COPY_AND_INCR(description);

        if constexpr (IS_FILE) {
            delete[] buf;
        }

#undef COPY_AND_INCR
#undef IS_FILE
    }
#endif

public:
    uint32_t recordSize = 0;
    std::string title;
    std::string abstract;
    std::string claim;
    std::string description;


    DataRecordV2() = default;

    /**
     * @brief initialize a DataRecord through interpreting a pre-read buffer
     * @param buf       buffer in which the file is stored
     * @param offset    offset to start reading
     * @param maxOffset end of buffer
     */
    DataRecordV2(char* buf, uint64_t offset, uint64_t maxOffset)
    {
        decode(buf, offset, maxOffset);
    }

    /**
     * @brief initialize a DataRecord through reading file from an opened file descriptor
     * @param fileHandle    file handle obtained by open()
     * @param offset        offset to start reading
     * @param maxOffset     maximum offset legal for the file i.e. boundary of the file
     */
    DataRecordV2(int fileHandle, uint64_t offset, uint64_t maxOffset)
    {
        // read in record size first
        if (pread(fileHandle, &recordSize, sizeof(recordSize), offset) == -1)
            throw ObjectConstructionFailure("pread() error");

        // check record size
        if (offset + recordSize > maxOffset)
            throw ObjectConstructionFailure("recordSize exceeds file boundary");

        char* buf = new char[recordSize];
        char* curBuf = buf + sizeof(uint32_t);
        // read the rest of the record data with given record size
        if (pread(fileHandle, buf + sizeof(uint32_t), recordSize - sizeof(uint32_t),
                offset + sizeof(uint32_t)) == -1) {
            delete[] buf;   // release memory before throwing
            throw ObjectConstructionFailure("pread() error");
        }

#define COPY_AND_INCR(field)                                                \
    do {                                                                    \
        uint32_t size = *(uint32_t*)curBuf;                                 \
        curBuf += sizeof(uint32_t);                                         \
        if (curBuf - buf + (uint64_t)size > maxOffset)                      \
            throw ObjectConstructionFailure("size exceeds file boundary");  \
        field = std::string(curBuf, curBuf + size);                         \
        curBuf += size;                                                     \
    } while (0)

        COPY_AND_INCR(title);
        COPY_AND_INCR(abstract);
        COPY_AND_INCR(claim);
        COPY_AND_INCR(description);

        delete[] buf;
    }
#undef COPY_AND_INCR

    DataRecordV2& operator=(DataRecordV2&& other) noexcept
    {
        recordSize = other.recordSize;
        other.recordSize = 0;
        title = std::move(other.title);
        abstract = std::move(other.abstract);
        claim = std::move(other.claim);
        description = std::move(other.description);

        return *this;
    }

    inline std::string stringify() const override
    {
        return ConcatStringWithDelimiter("\n",
                "<(title)>: " + title,
                "<(abstract)>: " + abstract,
                "<(claim)>: " + claim,
                "<(description)>: " + description);
    }
};

#endif //TOOLS_DATABASICTYPES_H
