//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_DATABASEFILEWRITERTHREAD_H
#define TOOLS_DATABASEFILEWRITERTHREAD_H


#include "ThreadModelInterface.h"
#include "CQueue.h"
#include "DataTypes.h"

class DatabaseFileWriterThread :
        public InputThreadInterface<CQueue<DataRecordFile*>> {

    std::filesystem::path dataFileDir_;
    std::filesystem::path dataFilePrefixName_;
    std::filesystem::path indexFilename_;

    void internalRun() final;

public:
    DatabaseFileWriterThread(
            InputType& dataRecordFileQueue,
            const std::filesystem::path& dataFileDir,
            const std::filesystem::path& dataFilePrefixName,
            const std::filesystem::path& indexFilename) :

            InputThreadInterface(dataRecordFileQueue),
            dataFileDir_(dataFileDir),
            dataFilePrefixName_(dataFilePrefixName),
            indexFilename_(indexFilename) { }
};


#endif //TOOLS_DATABASEFILEWRITERTHREAD_H
