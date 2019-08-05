//
// Created by yuan on 7/24/19.
//

#include <fstream>
#include "XmlFileReader.h"


void XmlFileReader::internalRun()
{
    using namespace std;
    /* if input file is not setup properly
     * set signal to terminate other threads
     * and quit gracefully */
    ifstream pathFile(pathFilename_);
    if (!pathFile.is_open()) {
        std::cout << "file not opened\n";
        filenameQueue_.setQuitSignal();
        return;
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
