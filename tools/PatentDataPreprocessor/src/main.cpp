//
// Created by yuan on 7/19/19.
//
#include <iostream>
#include <string>
#include <stdio.h>

#include "ThreadJob.h"
#include "ThreadPool.h"

#include "StatsThread.h"
#include "XmlFileReader.h"
#include "PatentInfoCollector.h"
#include "PatentInfoWriter.h"
#include "PatentInfoPC.h"

using namespace std;


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file> <tag-abstract-output-file> "
           "<split-abstract-output-file> <num-threads>\n", program);
    exit(-1);
}

void testRun(int argc, char* argv[])
{

    if (argc != 5)
        printUsageAndExit(argv[0]);

    string pathFilename(argv[1]);
    string infoOutputFilename(argv[2]);
    string splitAbstractOutputFilename(argv[3]);
    int nThreads = atoi(argv[4]);

    ConcurrentQueue<string> filenameQueue;
    XmlFileReader xmlFileReader(pathFilename, filenameQueue);

    xmlFileReader.runOnMain();

    ConcurrentQueue<string> outputInfoQueue;
    ConcurrentQueue<string> splitAbstractQueue;

    ThreadPool producers;

    for (int i = 0; i < nThreads; i++)
        producers.add<PatentInfoCollector>(filenameQueue, outputInfoQueue, splitAbstractQueue);

    ThreadPool consumers;

    consumers.add<PatentInfoWriter>(infoOutputFilename, outputInfoQueue);
    consumers.add<PatentInfoWriter>(splitAbstractOutputFilename, splitAbstractQueue);

//    StatsThread<string> readStats(filenameQueue_);
    StatsThread<string, true> writeStats(outputInfoQueue, filenameQueue.totalPushedItems());

    producers.runAll();
//    readStats.run();
    consumers.runAll();
    writeStats.run();

    producers.waitAll();
    outputInfoQueue.setQuitSignal();
    splitAbstractQueue.setQuitSignal();
//    readStats.wait();
    consumers.waitAll();
    writeStats.wait();

    cout << "finished\n";
}

int main(int argc, char* argv[])
{

    if (argc != 5)
        printUsageAndExit(argv[0]);

//    string pathFilename(argv[1]);
//    string infoOutputFilename(argv[2]);
//    string splitAbstractOutputFilename(argv[3]);
    int nThreads = atoi(argv[4]);

    PatentInfoPC pcPool(argv[1], argv[2], argv[3], nThreads);

    pcPool.runAll();

    pcPool.waitAll();

    return 0;
}
