//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_XMLXPATHIPOGENERICINTERFACE_H
#define TOOLS_XMLXPATHIPOGENERICINTERFACE_H

#include "ThreadPool.h"
#include "FormatFunctors.h"

/* a helper execution flow interface class for clarifying
 * execution order of Xml processing tasks that
 * takes advantage of a I/O + Processor threading model
 * IPO stands for Input + Process + Output triple pool-threading */
class XmlXpathIPOGenericInterface {
protected:
    /* thread pool counts */
    int         nReaders_ = 1,
                nProcessors_ = 1,
                nWriters_ = 1;

    /* thread pools */
    ThreadPool  readerPool_,    // Input threads
                processorPool_, // Process threads
                writerPool_;    // Output threads

    /* abstract method to be implemented to add custom query functors */
    virtual void initializeQuery() = 0;
    /* abstract method to be implemented to initialize custom data objects */
    virtual void initializeData() = 0;
    /* abstract method to be implemented to add initialize IPO threads
     * or any additional threads (e.g. StatsThread) */
    virtual void initializeThreads() = 0;
    /* abstract method to be implemented to execute all threads synchronously */
    virtual void executeThreads() = 0;

    explicit XmlXpathIPOGenericInterface(int nReaders = 1,
            int nProcessors = 1, int nWriters = 1) :
            nReaders_(nReaders), nProcessors_(nProcessors), nWriters_(nWriters) { }

public:
    virtual ~XmlXpathIPOGenericInterface() = default;
    /* calls implemented abstract methods in order */
    virtual void process()
    {
        initializeQuery();
        initializeData();
        initializeThreads();
        executeThreads();
    }
};

#endif //TOOLS_XMLXPATHIPOGENERICINTERFACE_H
