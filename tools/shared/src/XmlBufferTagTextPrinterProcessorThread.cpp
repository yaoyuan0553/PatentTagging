//
// Created by yuan on 8/16/19.
//

#include "XmlBufferTagTextPrinterProcessorThread.h"

using namespace std;

void XmlBufferTagTextPrinterProcessorThread::internalRun()
{
    for (uint64_t bN = 0;;)
    {
        auto [xmlFile, quit] = inputData_.pop();

        if (quit) break;

        pugi::xml_document doc;
        doc.load_buffer_inplace_own(xmlFile.buffer, xmlFile.bufferSize);

        // WARNING: must be released by caller
        string* singleOutput = new string;
        try {
            for (const string& request : xpathQueryTextFormatterDict_.getOrderedKeys()) {
                *singleOutput += request + ": " + xpathQueryTextFormatterDict_[request](doc) + '\n';
            }
        }
        catch (std::out_of_range& e) {
            cerr << "out of range here " << e.what() << '\n';
            PERROR("out_of_range()");
        }

        *singleOutput += '\n';
        batchOutput_.push_back(singleOutput);

        if (++bN % batchSize_ == 0)
            addBatchToQueue();
    }

    if (!batchOutput_.empty())
        addBatchToQueue();
}
