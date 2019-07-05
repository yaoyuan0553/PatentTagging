//
// Created by yuan on 7/4/19.
//

#include "FileDistributor.h"
#include <iostream>

void FileDistributor::internalRun()
{
    /* if input file is not setup properly
     * set signal to terminate other threads
     * and quit gracefully */
    if (!pathFile_.is_open()) {
        std::cout << "file not opened\n";
        filenameQueue_.setQuitSignal();
        return;
    }

    int i = 0;
    std::vector<std::string> fileBatch;
    fileBatch.reserve(batchSize_);
    for (std::string filename; std::getline(pathFile_, filename);)
    {
        std::cout << filename << "\n";
        fileBatch.push_back(filename);
        if (++i % batchSize_ == 0) {
            filenameQueue_.push(fileBatch);
            fileBatch.clear();
        }
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
