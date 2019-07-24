//
// Created by yuan on 7/24/19.
//


#include "PatentInfoCollector.h"



void PatentInfoCollector::internalRun(ConcurrentQueue<std::string>& filenameQueue,
                 ConcurrentQueue<std::string>& outputInfoQueue)
{
//    std::cout << "thread " << std::this_thread::get_id() << " started\n";
    int bN = 0;
    for (;;) {
        pugi::xml_document doc;
        // consume object from queue
        auto[filename, quit] = filenameQueue.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result) {
            errorFiles_.push_back(filename);
            continue;
        }
        walker_.reset();
        doc.traverse(walker_);

        if (++bN == batchSize_) {
            outputInfoQueue.push(batchInfo_);
            batchInfo_.clear();
        }

        std::string info;
        for (const std::string &tag : walker_.uniqueTags)
            info += tag + ',';
        info[info.size() - 1] = '\t';
        info += walker_.abstract + '\n';

        batchInfo_.push_back(info);

        if (walker_.isIrregular)
            errorFiles_.push_back(filename);
    }
    if (!batchInfo_.empty())
        outputInfoQueue.push(batchInfo_);
}
