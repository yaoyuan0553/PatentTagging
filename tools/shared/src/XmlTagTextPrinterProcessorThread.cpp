//
// Created by yuan on 8/15/19.
//

#include "XmlTagTextPrinterProcessorThread.h"

using namespace std;

void XmlTagTextPrinterProcessorThread::internalRun()
{
    for (uint64_t bN = 0;;)
    {
        auto [doc ,quit] = inputData_.pop();

        if (quit) break;

        walker_.reset();

        doc->traverse(walker_);

        string singleOutput;
        TagTextDict& tagTextDict = walker_.getTagTexts();
        try {
            for (const auto&[tag, textVec] : tagTextDict) {
                if (tagTextOutputFormatterDict_.getKeys().find(tag) ==
                    tagTextOutputFormatterDict_.getKeys().end())
                    singleOutput += tag + ": " + tagTextDict.at(tag)[0] + '\n';
                else
                    singleOutput += tag + ": " +
                            tagTextOutputFormatterDict_[tag](tagTextDict) + '\n';
            }
        }
        catch (std::out_of_range& e)
        {
            cerr << "out of range here " << e.what() << '\n';
            cerr << "possible node filter & formatter mismatch (formatter using nodes"
                    "that doesn't exist in filter\n";
            exit(-1);
        }
        singleOutput += '\n';
        batchOutput_.emplace_back(singleOutput);

        if (++bN % batchSize_ == 0)
            addBatchToQueue();

        delete doc;
    }
    if (!batchOutput_.empty())
        addBatchToQueue();
}
