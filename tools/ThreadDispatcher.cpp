//
// Created by yuan on 7/4/19.
//

#include "ThreadDispatcher.h"

ThreadDispatcher::ThreadDispatcher(int numConsumers, int batchSize):
    numConsumers_(numConsumers), producer_(batchSize), consumers_(numConsumers)
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
