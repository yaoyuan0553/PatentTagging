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
    /*! binary data, accessible only through getter function buf() */
    char* buf_;

    /*! number of bytes allocated for buf_ */
    uint64_t capacity_;

    /*!
     * @brief malloc and checks for error, used in constructor initialization list
     * @param size number of bytes to allocate
     * @return allocated buffer
     */
    inline static void* checkMalloc(size_t size);

protected:
    /*! keeps track of number of bytes used in buf_ */
    uint64_t& nBytes_;
    /*! keeps track of number of records stored in buf_ */
    uint32_t& nRecords_;

    inline static constexpr size_t FILE_HEAD_SIZE =
            sizeof(decltype(nBytes_)) +
            sizeof(decltype(nRecords_));

    /*!
     * @brief provides access to internal buffer buf_
     * @return buf_
     */
    inline char* buf() const { return buf_; }

    /**
     * @brief initializes the file to be only the headers
     * @details headers contains an 8 byte unsigned integer - nBytes and
     *          a 4 byte unsigned integer - nRecords
     *          which means the buf_ will only be 12 bytes initially
     */
    explicit DataRecordFileV2(uint64_t sizeToAllocate = 0);

public:
    /*! @brief releases all memory allocated under buf_ */
    virtual ~DataRecordFileV2();

    /*! @brief request for a resize of the buffer buf_ */
    void reserve(size_t newSize);

    /*! total bytes allocated */
    inline uint32_t capacity() const { return capacity_; }

    /*! total bytes of data written to the buffer */
    inline uint32_t numBytes() const { return nBytes_; }

    /*! current number of records in the buffer */
    inline uint32_t numRecords() const { return nRecords_; }

    /* returns true if number of records is 0 */
    inline bool empty() const { return nRecords_ == 0; }
};


inline void* DataRecordFileV2::checkMalloc(size_t size)
{
    if (void* buf = malloc(size); buf)
        return buf;

    PSYS_FATAL("malloc()");
}


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

    /*! points to current buffer location writing to */
    char* curBuf_;

    /*! stores the corresponding index value list and writes to disk */
    IndexValueList indexValueList_;

    /*! max number of bytes in the file */
    uint64_t maxFileSize_;

    /*! obtains a new bin Id for a newly created instance */
    inline void obtainBinId();

public:

    explicit DataRecordFileWriter(size_t maxFileSize);

    inline const IndexValueList& indexValueList() const { return indexValueList_; }

    inline std::string generateFilename(const char* prefix) const
    {
        return std::string(prefix) + "_" + std::to_string(binId_) + ".bin";
    }

    /*!
     * @brief writes data records to file
     * @param filename file name of this data record file
     */
    void writeToFile(const char* filename) final;

};

inline void DataRecordFileWriter::obtainBinId()
{
    binId_ = nextBinId_++;
}


class DataRecordFileReader : DataRecordFileV2 {

    const std::string filename_;
    int fileHandle_;

    void openDataFile();
    void closeDataFile();

public:
    explicit DataRecordFileReader(const char* dataFilename);
    ~DataRecordFileReader() override;

    bool getDataRecordAtOffset(uint64_t offset, DataRecordV2* dataRecord) const;
};




#endif //TOOLS_DATARECORDFILEV2_
