//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASICTYPES_H
#define TOOLS_DATABASICTYPES_H

#include <string>

#include "Helper.h"


/* used to interpret & copy-out records stored in data file buffers */
struct DataRecord : Stringifiable {
    /* read only attributes */
    uint32_t size = 0;
    uint32_t ts = 0, as = 0, cs = 0, ds = 0;
    const std::string* title = nullptr;
    const std::string* abstract = nullptr;
    const std::string* claim = nullptr;
    const std::string* description = nullptr;

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
            title(new std::string(_title, _title + _ts)),
            abstract(new std::string(_abstract, _abstract + _as)),
            claim(new std::string(_claim, _claim + _cs)),
            description(new std::string(_description, _description + _ds))
    { }
    ~DataRecord()
    {
        delete title;
        delete abstract;
        delete claim;
        delete description;
    }

    /* allow move constructor */
    DataRecord(DataRecord&& other) noexcept : size(other.size), ts(other.ts),
                                              as(other.as), cs(other.cs), ds(other.ds),
                                              title(other.title), abstract(other.abstract), claim(other.claim),
                                              description(other.description)
    {
        /* swap pointers */
        other.title = nullptr;
        other.abstract = nullptr;
        other.claim = nullptr;
        other.description = nullptr;
    }

    /* disable copy constructor */
    DataRecord(const DataRecord&) = delete;

    /* move assignment */
    DataRecord& operator=(DataRecord&& other) noexcept;

    DataRecord& operator=(DataRecord&) = delete;

    inline std::string stringify() const final
    {
        if (!title || !abstract ||
            !claim || !description)
            return "empty data record";

        return ConcatStringWithDelimiter("\n",
                "<(title)>: " + *title,
                "<(abstract)>: " + *abstract,
                "<(claim)>: " + *claim,
                "<(description)>: " + *description);
    }

    friend std::ostream& operator<<(std::ostream& os, const DataRecord& dataRecord);
};

struct IndexValue : public Stringifiable {

    inline static const std::string header = ConcatStringWithDelimiter("\t",
                                                                       "Publication ID", "Application ID", "Application Date",
                                                                       "Classification IPC", "Bin ID", "Offset", "Title Index",
                                                                       "Abstract Index", "Claim Index", "Description Index");

    std::string                 pid, aid, appDate, ipc;
    uint32_t                    binId, ti, ai, ci, di;
    uint64_t                    offset;

    inline std::string stringify() const final
    {
        using std::to_string;

        return ConcatStringWithDelimiter("\t", pid, aid, appDate, ipc,
                                         to_string(binId), to_string(offset), to_string(ti),
                                         to_string(ai), to_string(ci), to_string(di));
    }

//    IndexValue& operator=(const IndexValue&)
//    {
//        std::cout << "copy assignment called\n";
//
//        return *this;
//    }

    /* for file I/O */
    friend std::ostream& operator<<(std::ostream& os, const IndexValue& ie);
    friend std::istream& operator>>(std::istream& is, IndexValue& ie);
};

struct IdDataRecord {
    std::string pid;
    std::string aid;
    DataRecord dataRecord;
};


#endif //TOOLS_DATABASICTYPES_H
