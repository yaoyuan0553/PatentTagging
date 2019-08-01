//
// Created by yuan on 7/30/19.
//

#include "PatentTagTextCollector.h"

#include <filesystem>
#include <stdio.h>

#include <pugixml.hpp>

#include "TagConstants.h"


namespace fs = std::filesystem;
using namespace std;


void PatentTagTextCollector::internalRun()
{
    unordered_map<string, vector<string>> batchOutputByFile;
    for (int bN = 1;; bN++)
    {
        pugi::xml_document doc;
        auto [filename, quit] = filenameQueue_.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result)
            continue;

        walker_.reset();

        try {
            doc.traverse(walker_);
        }
        catch (range_error& e) {
            cerr << e.what() << '\n';
            cerr << "[" << filename << "]\n";
            continue;
        }

        // add name of current xml file to the collection
        walker_.getTagTexts()[tags::filename].push_back(fs::path(filename).stem());

        for (const string& outputFilename : fileOutputFormatterDict_.getKeys()) {
            if (bN == 0)
                batchOutputByFile[outputFilename].reserve(batchSize_);
            batchOutputByFile[outputFilename].push_back(
                    fileOutputFormatterDict_[outputFilename](walker_.getTagTexts()));
        }

        if (bN % batchSize_ == 0)
            generateOutputText(batchOutputByFile);
    }
    generateOutputText(batchOutputByFile);
}

void PatentTagTextCollector::generateOutputText(BatchOutputByFile& batchOutputByFile)
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

