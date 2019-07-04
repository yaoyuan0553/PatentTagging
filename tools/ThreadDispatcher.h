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

// single producer, multiple consumer model
class ThreadDispatcher {
    int numConsumers_;

    FileDistributor producer_;
    std::vector<PatentTagCollector> consumers_;

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
    explicit ThreadDispatcher(int numConsumers = 1, int batchSize = 128);

    void join();
};


#endif //TOOLS_THREADDISPATCHER_H
