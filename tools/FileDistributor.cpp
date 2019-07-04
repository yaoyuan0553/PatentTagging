//
// Created by yuan on 7/4/19.
//

#include "FileDistributor.h"
#include <iostream>

void FileDistributor::internalRun()
{
    int i = 0;
    std::vector<std::string> fileBatch;
    fileBatch.reserve(batchSize);
    for (std::string filename; std::getline(std::cin, filename); i++)
    {
        if (i % batchSize == 0) {
            filenameQueue_.push(fileBatch);
            fileBatch.clear();
        }
        fileBatch.push_back(filename);
    }
    if (!fileBatch.empty())
        filenameQueue_.push(fileBatch);

    // signal consumer threads to quit
    filenameQueue_.setQuitSignal();
}

void FileDistributor::run()
{
    thread_ = std::thread(&FileDistributor::internalRun, this);
    threadStarted_ = true;
}
