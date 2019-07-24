//
// Created by yuan on 7/5/19.
//

#pragma once
#ifndef TOOLS_STATSTHREAD_H
#define TOOLS_STATSTHREAD_H

#include <string>

#include <tqdm.h>

#include "ThreadJob.h"
#include "ConcurrentQueue.h"


template <typename T>
class StatsThread : public ThreadJob<> {
    ConcurrentQueue<T>& dataQueue_;
    void internalRun() override
    {
        tqdm bar;
        // bar.set_theme_line();
        for (;;)
        {
            auto i = dataQueue_.totalPoppedItems();
            auto n = dataQueue_.totalPushedItems();
            bar.progress(i, n);
            if (i == n)
                break;
            // update every 1 seconds
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        bar.finish();
    }

public:
    explicit StatsThread(ConcurrentQueue<T>& dataQueue) : dataQueue_(dataQueue) { }
};




#endif //TOOLS_STATSTHREAD_H
