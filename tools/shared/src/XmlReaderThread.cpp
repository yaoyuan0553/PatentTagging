//
// Created by yuan on 8/15/19.
//

#include "XmlReaderThread.h"

#include "Utility.h"

#include <vector>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

void XmlReaderThread::internalRun()
{
    ifstream pathFile(pathFilename_);
    if (!pathFile.is_open()) {
        fprintf(stderr, "file [%s] not opened\n", pathFilename_.c_str());
        PERROR("ifstream fail");
    }

    /* read the whole file into stringstream for later use */
    stringstream iss;
    iss << pathFile.rdbuf();

    /* close the file */
    pathFile.close();

    uint64_t bN = 0;
    for (string filename; getline(iss, filename);)
    {
        // WARNING: memory must be released by caller
        auto doc = new pugi::xml_document;

        if (!doc->load_file(filename.c_str()))
            continue;

        batchOutput_.push_back(doc);
        if (++bN % batchSize_ == 0)
            addBatchToQueue();
    }
    if (!batchOutput_.empty())
        addBatchToQueue();
}
