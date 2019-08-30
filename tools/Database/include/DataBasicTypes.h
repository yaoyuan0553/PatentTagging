//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASICTYPES_H
#define TOOLS_DATABASICTYPES_H

#include <string>

#include "Helper.h"


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


#endif //TOOLS_DATABASICTYPES_H
