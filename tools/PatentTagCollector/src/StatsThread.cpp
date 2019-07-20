//
// Created by yuan on 7/5/19.
//

#include "StatsThread.h"

#include <tqdm.h>


void StatsThread::internalRun(ConcurrentQueue<std::string>& dataQueue)
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

