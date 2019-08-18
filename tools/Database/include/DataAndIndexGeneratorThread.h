//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_DATAANDINDEXGENERATORTHREAD_H
#define TOOLS_DATAANDINDEXGENERATORTHREAD_H

#include "ThreadModelInterface.h"
#include "CQueue.h"
#include "XpathQueryTextFormatter.h"
#include "DataTypes.h"


class DataAndIndexGeneratorThread :
        public InputOutputThreadInterface<CQueue<std::pair<char*, size_t>>,
                CQueue<DataRecordFile*>> {

    XpathQueryTextFormatterDict xpathQueryTextFormatterDict_;
    /* WARNING: all memory allocated here has to be released by caller */
    DataRecordFile* dataRecordFile_;

    // TODO: hard coded string to distinguish data text from index text
    const std::vector<std::string> dataTextKeys_;
    const std::vector<std::string> indexTextKeys_;

    void internalRun() final;

public:
    explicit DataAndIndexGeneratorThread(
            CQueue<std::pair<char*, size_t>>& inputQueue,
            CQueue<DataRecordFile*>& outputQueue,
            const XpathQueryTextFormatterDict& xpathQueryTextFormatterDict,
            const std::vector<std::string>& dataTextKeys,
            const std::vector<std::string>& indexTextKeys) :

            InputOutputThreadInterface(inputQueue, outputQueue),
            xpathQueryTextFormatterDict_(xpathQueryTextFormatterDict),
            dataRecordFile_(new DataRecordFile),
            dataTextKeys_(dataTextKeys), indexTextKeys_(indexTextKeys)
    {
        if (dataTextKeys.size() != HARD_CODED_DATA_FIELDS ||
                indexTextKeys.size() != HARD_CODED_INDEX_FIELDS)
            PERROR("hard coded keys don't match");
    }

};


#endif //TOOLS_DATAANDINDEXGENERATORTHREAD_H
