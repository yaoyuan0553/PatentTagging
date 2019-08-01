//
// Created by yuan on 7/24/19.
//


#include "PatentInfoCollector.h"

#include <filesystem>

namespace fs = std::filesystem;

void PatentInfoCollector::internalRun(ConcurrentQueue<std::string>& filenameQueue,
                 ConcurrentQueue<std::string>& outputInfoQueue,
                 ConcurrentQueue<std::string>& splitAbstractQueue)
{
//    std::cout << "thread " << std::this_thread::get_id() << " started\n";
    std::vector<std::string> batchInfo, splitAbstract;
    batchInfo.reserve(batchSize_);
    splitAbstract.reserve(batchSize_);
    int bN = 0;
    for (;;) {
        pugi::xml_document doc;
        // consume object from queue
        auto [filename, quit] = filenameQueue.pop();

        if (quit) break;

        pugi::xml_parse_result result = doc.load_file(filename.c_str());
        if (!result) {
            errorFiles_.push_back(filename);
            continue;
        }
        walker_.reset();

        try {
            doc.traverse(walker_);
        }
        catch (std::range_error& e) {
            std::cerr << e.what() << '\n';
            std::cerr << "[" << filename << "]: " << walker_.splitAbstract << '\n';
            continue;
        }

        if (++bN == batchSize_) {
            outputInfoQueue.push(batchInfo);
            splitAbstractQueue.push(splitAbstract);
            batchInfo.clear();
            splitAbstract.clear();
            bN = 0;
        }

        std::string info = fs::path(filename).stem();
        info += '\t';
        for (const std::string &tag : walker_.uniqueTags)
            info += tag + ',';
        info[info.size() - 1] = '\t';
        info += walker_.abstract + '\n';

        batchInfo.push_back(info);
        splitAbstract.push_back(walker_.splitAbstract + "\n\n");

        if (walker_.isIrregular)
            errorFiles_.push_back(filename);
    }
    if (!batchInfo.empty())
        outputInfoQueue.push(batchInfo);
}
