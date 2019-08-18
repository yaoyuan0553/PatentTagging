//
// Created by yuan on 7/26/19.
//


#include "PatentInfoPC.h"

#include "PatentInfoCollector.h"
#include "XmlInfoWriter.h"


void PatentInfoPC::initThreads()
{
    xmlFileReader_.runOnMain();
    statsThread_ = new StatsThread<std::string, true>(outputInfoQueue_, filenameQueue_.totalPushedItems());
    for (int i = 0; i < nProducers_; i++)
        producers.add<PatentInfoCollector>(filenameQueue_, outputInfoQueue_, splitAbstractQueue_);

    consumers.add<XmlInfoWriter>(infoOutputFilename_, outputInfoQueue_);
    consumers.add<XmlInfoWriter>(splitAbstractOutputFilename_, splitAbstractQueue_);
}

void PatentInfoPC::runAll()
{
    PCThreadPool::runAll();
    statsThread_->run();
}

void PatentInfoPC::waitAll()
{
    producers.waitAll();
    outputInfoQueue_.setQuitSignal();
    splitAbstractQueue_.setQuitSignal();

    consumers.waitAll();
    statsThread_->wait();
}
