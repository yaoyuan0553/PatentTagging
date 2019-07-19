//
// Created by yuan on 7/4/19.
//

#include <iostream>
#include "ThreadDispatcher.h"

ThreadDispatcher::ThreadDispatcher(const std::string& pathFilename,
        int numConsumers, int batchSize):
        numConsumers_(numConsumers), producer_(pathFilename, batchSize),
        consumers_(numConsumers)
{
    // start all threads
    producer_.run();
    stats_.run(producer_.filenameQueue());
    for (auto& consumer : consumers_)
        consumer.run(producer_.filenameQueue());
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
