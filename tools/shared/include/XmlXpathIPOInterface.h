//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XMLXPATHIPOINTERFACE_H
#define TOOLS_XMLXPATHIPOINTERFACE_H

#include "ThreadPool.h"
#include "FormatFunctors.h"
#include "XpathQueryTextFormatter.h"

/* a helper execution flow interface class for clarifying
 * execution order of Xml processing tasks that
 * takes advantage of a I/O + Processor threading model
 * IPO stands for Input + Process + Output triple pool-threading */
class XmlXpathIPOInterface {
protected:
    /* thread pool counts */
    int         nReaders_ = 1,
                nProcessors_ = 1,
                nWriters_ = 1;

    /* thread pools */
    ThreadPool  readerPool_,    // Input threads
                processorPool_, // Process threads
                writerPool_;    // Output threads

    /* storing query processing functors  */
    XpathQueryTextFormatterDict xpathQueryTextFormatterDict_;

    /* abstract method to be implemented to add custom query functors */
    virtual void initializeQuery() = 0;
    /* abstract method to be implemented to initialize custom data objects */
    virtual void initializeData() = 0;
    /* abstract method to be implemented to add initialize IPO threads
     * or any additional threads (e.g. StatsThread) */
    virtual void initializeThreads() = 0;
    /* abstract method to be implemented to execute all threads synchronously */
    virtual void executeThreads() = 0;

    explicit XmlXpathIPOInterface(int nReaders = 1, int nProcessors = 1, int nWriters = 1) :
            nReaders_(nReaders), nProcessors_(nProcessors), nWriters_(nWriters) { }

public:
    virtual ~XmlXpathIPOInterface() = default;
    /* calls implemented abstract methods in order */
    virtual void process()
    {
        initializeQuery();
        initializeData();
        initializeThreads();
        executeThreads();
    }
};

#endif //TOOLS_XMLXPATHIPOINTERFACE_H
