//
// Created by yuan on 7/30/19.
//

#include "PatentTagTextCollector.h"

#include <filesystem>
#include <stdio.h>

#include <pugixml.hpp>
#include <bits/unordered_map.h>

#include "TagConstants.h"


namespace fs = std::filesystem;
using namespace std;


void PatentTagTextCollector::internalRun()
{
    unordered_map<string, vector<string>> batchOutputByFile;
    for (int bN = 0;;)
    {
        pugi::xml_document doc;
        auto [filename, quit] = filenameQueue_.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result)
            continue;

        walker_.reset();

        doc.traverse(walker_);

        // add name of current xml file to the collection
        walker_.getTagTexts()[tags::filename].push_back(fs::path(filename).stem());

        unordered_map<string, string> singleOutputByFile;
        /* range_error can be thrown by functors who use SplitParagraph functor
         * when invalid char32_t is encountered */
        try {
            for (const string& outputFilename : fileOutputFormatterDict_.getKeys()) {
                if (bN == 0)
                    batchOutputByFile[outputFilename].reserve(batchSize_);
                singleOutputByFile[outputFilename] =
                        fileOutputFormatterDict_[outputFilename](walker_.getTagTexts());
            }
        }
        catch (range_error& e) {
            cerr << e.what() << '\n';
            cerr << "[" << filename << "]\n";
            continue;
        }

        for (auto&& [filename, output] : singleOutputByFile)
            batchOutputByFile[filename].emplace_back(output);

        if (++bN % batchSize_ == 0)
            addBatchToQueue(batchOutputByFile);
    }
    addBatchToQueue(batchOutputByFile);
}

void PatentTagTextCollector::addBatchToQueue(BatchOutputByFile& batchOutputByFile)
{
    for (auto&& [outputFilename, output] : batchOutputByFile) {
        if (outputQueueByFile_.find(outputFilename) == outputQueueByFile_.end()) {
            fprintf(stderr, "file entry [%s] not found\n", outputFilename.c_str());
            exit(-1);
        }
        if (!output.empty())
            outputQueueByFile_[outputFilename].push(output);
        output.clear();
    }
}

