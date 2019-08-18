//
// Created by yuan on 8/14/19.
//

#include <fstream>
#include <vector>
#include "XmlPCProcessorTester.h"

using namespace std;

void TestTagTextExtractor::internalRun()
{
    vector<string> batchOutput;
    batchOutput.reserve(batchSize_);
    for (int bN = 0;;)
    {
        pugi::xml_document doc;
        auto [filename, quit] = filenameQueue_.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());

        if (!result) continue;

        walker_.reset();
        doc.traverse(walker_);

        string singleOutput;
        TagTextDict& tagTextDict = walker_.getTagTexts();
        try {
            try {
                for (const auto&[tag, textVec] : tagTextDict) {
                    if (fileOutputFormatterDict_.getKeys().find(tag) ==
                        fileOutputFormatterDict_.getKeys().end())
                        singleOutput += tag + ": " + tagTextDict.at(tag)[0] + '\n';
                    else
                        singleOutput += tag + ": " + fileOutputFormatterDict_[tag](tagTextDict) + '\n';
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
            batchOutput.emplace_back(singleOutput);
        }
        catch (range_error& e) {
            cerr << e.what() << '\n';
            cerr << "[" << filename << "]\n";
            continue;
        }

        if (++bN % batchSize_ == 0)
            addBatchToQueue(batchOutput);
    }
    if (!batchOutput.empty())
        addBatchToQueue(batchOutput);
}

void TestTagTextExtractor::addBatchToQueue(std::vector<std::string>& batchOutput)
{
    outputQueue_.push(batchOutput);
    batchOutput.clear();
}
