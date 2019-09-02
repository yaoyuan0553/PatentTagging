//
// Created by yuan on 9/2/19.
//

#pragma once
#ifndef TOOLS_DATARECORDFILEV2_H
#define TOOLS_DATARECORDFILEV2_H

#include "DataBasicTypes.h"
#include "DataTypes.h"
#include "IndexTableV2.h"


class DataRecordFileV2 {
protected:
    /*! binary data */
    char* const buf_;
    /*! keeps track of number of bytes used in buf_ */
    uint64_t& nBytes_;
    /*! keeps track of number of records stored in buf_ */
    uint32_t& nRecords_;

    uint64_t allocatedSize_;

    inline static constexpr auto FILE_HEAD_SIZE =
            sizeof(decltype(nBytes_)) +
            sizeof(decltype(nRecords_));
    /**
     * @brief initializes the file to be only the headers
     * @details headers contains an 8 byte unsigned integer - nBytes and
     *          a 4 byte unsigned integer - nRecords
     *          which means the buf_ will only be 12 bytes initially
     */
    explicit DataRecordFileV2(uint64_t sizeToAllocate);

public:
    /**
     * @brief releases all memory allocated under buf_
     */
    ~DataRecordFileV2();
};


class DataRecordFileWriter : DataRecordFileV2, public FileWritable {
    /**
     * accumulate number of data record files created
     * used to assign unique number for each object
     * created
     */
    inline static std::atomic_uint32_t nextBinId_ = 0;

    /**
     * data file unique number for the current object
     * assigned upon construction
     */
    uint32_t binId_;

    /**
     * points to current buffer location writing to
     */
    char* curBuf_;

    /*! stores the corresponding index value list and writes to disk */
    IndexValueList indexValueList_;

    /*! max number of bytes in the file */
    uint64_t maxFileSize_;

    /**
     * obtains a new bin Id for a newly created instance
     */
    inline void obtainBinId();

public:

    explicit DataRecordFileWriter(size_t maxFileSize);

};

inline void DataRecordFileWriter::obtainBinId()
{
    binId_ = nextBinId_++;
}


#endif //TOOLS_DATARECORDFILEV2_H
