//
// Created by yuan on 7/4/19.
//

#include "PatentTagCollector.h"

#include <iostream>
#include <stdio.h>

void PatentTagCollector::internalRun(ConcurrentQueue<std::string>& filenameQueue)
{
    std::cout << "thread " << thread_.get_id() << " started\n";
    for (;;)
    {
        pugi::xml_document doc;
        // consume object from queue
        auto [filename, quit] = filenameQueue.pop();

        // std::cout << "got element: " << filename << "quit: " << quit << "\n";
        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result) {
            errorFiles_.push_back(filename);
            continue;
        }
        walker_.curFilename = filename.c_str();
        walker_.isIrregular = false;
        doc.traverse(walker_);
        if (walker_.isIrregular)
            errorFiles_.push_back(filename);
    }
    std::cout << "thread " << thread_.get_id() << " finished\n";
}
