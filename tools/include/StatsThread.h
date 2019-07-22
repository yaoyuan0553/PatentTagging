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
#include "FileDistributor.h"


class StatsThread : public ThreadJob<ConcurrentQueue<std::string>&> {
    void internalRun(ConcurrentQueue<std::string>& dataQueue) override
    {
        tqdm bar;
        // bar.set_theme_line();
        for (;;)
        {
            auto i = dataQueue.totalPoppedItems();
            auto n = dataQueue.totalPushedItems();
            bar.progress(i, n);
            if (i == n)
                break;
            // update every 1 seconds
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        bar.finish();
    }
};




#endif //TOOLS_STATSTHREAD_H
