//
// Created by yuan on 8/16/19.
//

#include "DatabaseFileWriterThread.h"

#include <fstream>
#include <stdio.h>

using namespace std;

void DatabaseFileWriterThread::internalRun()
{
    ofstream ofs(indexFilename_);
    if (!ofs.is_open()) {
        fprintf(stderr, "failed to open [%s]\n", indexFilename_.c_str());
        PERROR("ofstream");
    }

    for (;;)
    {
        auto [dataRecordFile, quit] = inputData_.pop();

        if (quit) break;

        /* write data record to file */
        dataRecordFile->writeToFile(
                (dataFileDir_ / dataRecordFile->generateFilename(
                        dataFilePrefixName_.c_str())).c_str());

        dataRecordFile->writeSubIndexTableToStream(ofs);

        // NOTE: release memory of dataRecordFile
        delete dataRecordFile;
    }
    ofs.close();
}
