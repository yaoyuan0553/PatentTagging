//
// Created by yuan on 7/4/19.
//

#include <iostream>
#include "ThreadDispatcher.h"

ThreadDispatcher::ThreadDispatcher(const std::string& pathFilename,
        int numConsumers, int batchSize):
        numConsumers_(numConsumers), producer_(pathFilename, filenameQueue_, batchSize),
        stats_(filenameQueue_)
{
    for (int i = 0; i < numConsumers; i++)
        consumers_.emplace_back(filenameQueue_);
    // start all threads
    producer_.run();
    stats_.run();
    for (auto& consumer : consumers_)
        consumer.run();
}

void ThreadDispatcher::join()
{
    producer_.wait();
    for (auto& consumer: consumers_)
        consumer.wait();
    stats_.wait();
}

void ThreadDispatcher::writeResult2File(const std::string &filename)
{
    std::ofstream outputFile(filename);

    std::unordered_set<std::string> uniqueTagAll;
    for (auto& consumer: consumers_)
        uniqueTagAll.merge(consumer.uniqueTags());

    for (const auto& tag : uniqueTagAll)
        outputFile << tag << "\n";
}
