//
// Created by yuan on 7/5/19.
//

#pragma once
#ifndef TOOLS_STATSTHREAD_H
#define TOOLS_STATSTHREAD_H

#include <string>
#include "ThreadJob.h"
#include "ConcurrentQueue.h"
#include "FileDistributor.h"


class StatsThread : public ThreadJob<ConcurrentQueue<std::string>&> {
    void internalRun(ConcurrentQueue<std::string>& dataQueue) override;
};


#endif //TOOLS_STATSTHREAD_H
