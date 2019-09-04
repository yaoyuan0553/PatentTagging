//
// Created by yuan on 9/2/19.
//

#pragma once
#ifndef TOOLS_INDEXTABLEV2_H
#define TOOLS_INDEXTABLEV2_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <iostream>

#include "DataBasicTypes.h"
#include "DataTypes.h"


/**
 * data storage for IndexValue pointers, releases memory of all IndexValue pointer
 * when object is destroyed
 */
class IndexValueList : public std::vector<const IndexValue*> {
public:
    friend std::ostream& operator<<(std::ostream& os, const IndexValueList& ivl);
    friend std::istream& operator>>(std::istream& is, IndexValueList& ivl);
    // collect memory in IndexValue*
    ~IndexValueList();
};

/**
 * @brief represents on-disk IndexTable
 * @details loads on-disk IndexTable upon construction and all values are read-only
 */
class IndexTableV2 {
public:
    using IdIndexTable = std::unordered_map<std::string, const IndexValue*>;
private:
    /* data storage */
    IndexValueList indexValueList_;

    /* 3-way look up  using binId, PID, and AID */
    std::unordered_map<uint32_t, IndexValueList> binId2Index_;
    IdIndexTable pid2Index_;
    IdIndexTable aid2Index_;

public:

    explicit IndexTableV2(const char* filename, bool hasHeader = true);

    inline size_t numRecords() const { return indexValueList_.size(); }

    /* read-only accesses */
    const IndexValueList&           indexValueList() const  { return indexValueList_; }
    const decltype(pid2Index_)&     pid2Index() const       { return pid2Index_; }
    const decltype(aid2Index_)&     aid2Index() const       { return aid2Index_; }
    const decltype(binId2Index_)&   binId2Index() const     { return binId2Index_; }

    const IndexValueList&   binId2Index(uint32_t binId) const       { return binId2Index_.at(binId); }
    const IndexValue*       pid2Index(const std::string& pid) const { return pid2Index_.at(pid); }
    const IndexValue*       aid2Index(const std::string& aid) const { return aid2Index_.at(aid); }
};


#endif //TOOLS_INDEXTABLEV2_H
