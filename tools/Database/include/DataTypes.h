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


// TODO: find out the correct Application & Publication & Classifiaciton length
// size includes null-termination \0
constexpr int APPLICATION_ID_SIZE = 20;
constexpr int PUBLICATION_ID_SIZE = 25;

/* format: \L\d\d\L-\d{1-3}/\d{2}*/
constexpr int CLASSIFICATION_SIZE = 21;


using DataRecord = std::unordered_map<std::string, std::string>;
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
#pragma pack(push, 1)
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
 * */
struct IndexValue {
private:
    /* we ignore this field when packing data into buffer and deep copy
     * at where it's pointed */
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

    /* load IndexValue from buffer (disk) */
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

    /* save IndexValue to buffer
     * returns false if buffer isn't large enough
     * remSize - remaining size of buffer
     * sizeWritten - total written size into buffer */
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

    inline constexpr int getTotalBytes()
    {
        return INDEX_VALUE_STATIC_SIZE + classCount * CLASSIFICATION_SIZE;
    }
};

#pragma pack(pop)


using IndexEntry = std::pair<IndexKey, IndexValue>;


#endif //TOOLS_DATATYPES_H
