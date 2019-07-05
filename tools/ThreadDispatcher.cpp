//
// Created by yuan on 7/4/19.
//

#include "ThreadDispatcher.h"

ThreadDispatcher::ThreadDispatcher(const std::string& pathFilename,
        int numConsumers, int batchSize):
        numConsumers_(numConsumers), producer_(pathFilename, batchSize),
        consumers_(numConsumers)
{
    // start all threads
    producer_.run();
    for (auto& consumer : consumers_)
        consumer.run(producer_.filenameQueue());
}

void ThreadDispatcher::join()
{
    producer_.wait();
    for (auto& consumer: consumers_)
        consumer.wait();
}
