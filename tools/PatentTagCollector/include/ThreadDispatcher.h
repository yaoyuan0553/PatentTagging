//
// Created by yuan on 7/4/19.
//

#pragma once
#ifndef TOOLS_THREADDISPATCHER_H
#define TOOLS_THREADDISPATCHER_H

#include <vector>

#include "ThreadJob.h"
#include "FileDistributor.h"
#include "PatentTagCollector.h"
#include "StatsThread.h"

// single producer, multiple consumer model
class ThreadDispatcher {
    int numConsumers_;

    ConcurrentQueue<std::string> filenameQueue_;
    FileDistributor producer_;
    std::vector<PatentTagCollector> consumers_;

    StatsThread stats_;


public:
    // TODO: check how to do this
    // template <typename... ProducerArgs, typename... ConsumerArgs>
    // ThreadDispatcher(int numConsumers, FileDistributor, ProducerArgs&&... producerArgs, ConsumerArgs&&... consumerArgs):
    //     numConsumers_(numConsumers), producer_(producerArgs...)
    // {
    //     consumers_.reserve(numConsumers_);
    //     for (int i = 0; i < numConsumers_; i++)
    //         consumers_.emplace_back(consumerArgs...);
    // }
    explicit ThreadDispatcher(const std::string& pathFilename, int numConsumers = 1,
            int batchSize = 128);

    void join();

    void writeResult2File(const std::string& filename);
};


#endif //TOOLS_THREADDISPATCHER_H
