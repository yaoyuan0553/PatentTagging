//
// Created by yuan on 8/5/19.
//

#pragma once
#ifndef TOOLS_XMLPCPROCESSORINTERFACE_H
#define TOOLS_XMLPCPROCESSORINTERFACE_H


#include "ThreadPool.h"
#include "TagNodeFilterFunctors.h"
#include "FormatFunctors.h"


/* a helper execution flow interface class for clarifying
 * execution order of Patent Xml processing tasks that
 * takes advantage of a producer-consumer threading model */
class XmlPCProcessorInterface {
protected:
    /* number of producers or consumers are default to 1 */
    int nProducers_ = 1, nConsumers_ = 1;
    /* producer and consumer thread pools */
    ThreadPool producers_, consumers_;

    /* tag node filters */
    TagNodeFilterDict tagNodeFilterDict_;
    /* output formatters */
    TagTextOutputFormatterDict tagTextOutputFormatterDict_;

    /* abstract method to be implemented to add custom node filters */
    virtual void prepareNodeFilters() = 0;
    /* abstract method to be implemented to add custom output formatters */
    virtual void prepareOutputFormatters() = 0;
    /* abstract method to be implemented to initialize custom data objects */
    virtual void initializeData() = 0;
    /* abstract method to be implemented to add initialize producer consumer threads
     * or any additional threads (e.g. StatsThread) */
    virtual void initializeThreads() = 0;
    /* abstract method to be implemented to execute all threads synchronously */
    virtual void executeThreads() = 0;
    XmlPCProcessorInterface() = default;

public:
    /* method that calls implemented abstract methods in order */
    virtual void process()
    {
        prepareNodeFilters();
        prepareOutputFormatters();
        initializeData();
        initializeThreads();
        executeThreads();
    }
};


#endif //TOOLS_XMLPCPROCESSORINTERFACE_H
