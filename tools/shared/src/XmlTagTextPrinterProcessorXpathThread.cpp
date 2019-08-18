//
// Created by yuan on 8/15/19.
//

#include "XmlTagTextPrinterProcessorXpathThread.h"

using namespace std;


void XmlTagTextPrinterProcessorXpathThread::internalRun()
{
    for (uint64_t bN = 0;;)
    {
        auto [doc, quit] = inputData_.pop();

        if (quit) break;

        // WARNING: must be released by caller
        string* singleOutput = new string;
        try {
            for (const string& request : xpathQueryTextFormatterDict_.getOrderedKeys()) {
                *singleOutput += request + ": " + xpathQueryTextFormatterDict_[request](*doc) + '\n';
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

        delete doc;
    }

    if (!batchOutput_.empty())
        addBatchToQueue();
}
