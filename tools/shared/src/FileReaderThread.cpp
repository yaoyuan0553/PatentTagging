//
// Created by yuan on 8/16/19.
//

#include "FileReaderThread.h"

#include "Utility.h"
#include "XmlFile.h"


void FileReaderThread::internalRun()
{
    for (uint64_t bN = 0;;)
    {
        auto [filename, quit] = inputData_.pop();

        if (quit) break;

        size_t bytes = GetFileSize(filename.c_str());

        // WARNING: released by the caller
        char* buffer = (char*)pugi::get_memory_allocation_function()(bytes);

        FILE* inputFile = fopen(filename.c_str(), "rb");
        if (!inputFile) {
            fprintf(stderr, "failed to open [%s]\n", filename.c_str());
            PERROR("fopen()");
        }

        fread(buffer, sizeof(char), bytes, inputFile);

        batchOutput_.emplace_back(XmlFile{filename, buffer, bytes});

        if (++bN % batchSize_ == 0)
            addBatchToQueue();

        fclose(inputFile);
    }
    if (!batchOutput_.empty())
        addBatchToQueue();
}
