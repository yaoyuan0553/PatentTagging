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


constexpr int APPLICATION_ID_SIZE = 25;
constexpr int PUBLICATION_ID_SIZE = 25;
constexpr int CLASSIFICATION_SIZE = 10;


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

/* index data type */
struct IndexValue {
    std::vector<ClassificationString>* classifications;
public:
    uint32_t datId;
    char applicationId[APPLICATION_ID_SIZE];
    char publicationId[PUBLICATION_ID_SIZE];
    uint16_t classCount;
    uint32_t ti, ai, ci, di;

    const std::vector<ClassificationString>& getClassifications() const { return *classifications; }

    IndexValue() : classifications(new std::vector<ClassificationString>) { }

    ~IndexValue()
    {
        delete classifications;
    }

    /* load IndexValue from buffer */
    static void load(unsigned char* buffer)
    {

    }

    /* save IndexValue to buffer
     * returns false if buffer isn't large enough
     * remSize - remaining size of buffer */
    bool save(unsigned char* buffer, const int remSize)
    {
        // TODO:
        return false;
    }
};


using IndexEntry = std::pair<IndexKey, IndexValue>;


#endif //TOOLS_DATATYPES_H
