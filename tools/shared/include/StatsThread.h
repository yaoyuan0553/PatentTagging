//
// Created by yuan on 7/5/19.
//

#pragma once
#ifndef TOOLS_STATSTHREAD_H
#define TOOLS_STATSTHREAD_H

#include <string>

#include <tqdm.h>

#include "ThreadJob.h"
#include "CQueue.h"


template <typename T, bool presetTotal = false>
class StatsThread : public ThreadJob<> {
    CQueue<T>& dataQueue_;
    size_t total_ = 0;
    std::string statsTitle_;
    void internalRun() override
    {
        tqdm bar;
        bar.set_title(statsTitle_);
        for (;;)
        {
            auto i = dataQueue_.totalPoppedItems();
            size_t n;
            if constexpr (presetTotal) {
                n = total_;
            }
            else {
                n = dataQueue_.totalPushedItems();
            }
            bar.progress(i, n);
            if (i == n && dataQueue_.isQuit())
                break;
            // update every 1 seconds
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        bar.finish();
    }

public:
    explicit StatsThread(CQueue<T>& dataQueue, std::string_view statsTitle = "") :
            dataQueue_(dataQueue), statsTitle_(statsTitle)
    {
        static_assert(!presetTotal, "presetTotal must be set to false to use this overload\n");
    }
    StatsThread(CQueue<T>& dataQueue, int total, std::string_view statsTitle = "") :
            dataQueue_(dataQueue), total_(total), statsTitle_(statsTitle)
    {
        static_assert(presetTotal, "presetTotal must be set to true to use this overload\n");
    }
};




#endif //TOOLS_STATSTHREAD_H
