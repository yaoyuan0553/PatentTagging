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

using namespace std;


void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file> <output-file> <num-threads>\n", program);
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    string pathFilename(argv[1]);
    string outputFilename(argv[2]);
    int nThreads = atoi(argv[3]);

    ConcurrentQueue<string> filenameQueue;
    XmlFileReader xmlFileReader(pathFilename, filenameQueue);

    xmlFileReader.runOnMain();

    ConcurrentQueue<string> outputInfoQueue;
    ThreadPool producers;
    for (int i = 0; i < nThreads; i++)
        producers.add(new PatentInfoCollector(filenameQueue, outputInfoQueue));

    ThreadPool consumers;
    consumers.add(new PatentInfoWriter(outputFilename, outputInfoQueue));

    StatsThread<string> readStats(filenameQueue);
    StatsThread<string> writeStats(outputInfoQueue);

    producers.runAll();
    readStats.run();
    consumers.runAll();
    writeStats.run();

    for (int i = 0; i < nThreads; i++)
        producers.waitAll();
    outputInfoQueue.setQuitSignal();
    readStats.wait();
    consumers.waitAll();
    writeStats.wait();

    cout << "finished\n";

    return 0;
}
