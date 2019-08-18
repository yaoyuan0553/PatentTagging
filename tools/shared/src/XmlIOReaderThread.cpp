//
// Created by yuan on 8/15/19.
//

#include "XmlIOReaderThread.h"

void XmlIOReaderThread::internalRun()
{
    for (uint64_t bN = 0;;)
    {
        auto [filename, quit] = inputData_.pop();

        if (quit) break;

        // WARNING: memory must be released by caller
        auto doc = new pugi::xml_document;

        if (!doc->load_file(filename.c_str())) {
            fprintf(stderr, "Load xml doc [%s] failed\n", filename.c_str());
            continue;
        }

        batchOutput_.push_back(doc);

        if (++bN % batchSize_ == 0)
            addBatchToQueue();
    }
    if (!batchOutput_.empty())
        addBatchToQueue();
}
