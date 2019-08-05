//
// Created by yuan on 7/24/19.
//

#include "XmlFileReader.h"

#include <fstream>
#include <stdio.h>


void XmlFileReader::internalRun()
{
    using namespace std;
    /* if input file is not setup properly
     * set signal to terminate other threads
     * and quit gracefully */
    ifstream pathFile(pathFilename_);
    if (!pathFile.is_open()) {
        fprintf(stderr, "file [%s] not opened\n", pathFilename_.c_str());
        exit(-1);
    }

    int i = 0;
    std::vector<std::string> fileBatch;
    fileBatch.reserve(batchSize_);
    for (std::string filename; std::getline(pathFile, filename);)
    {
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
    pathFile.close();
}
