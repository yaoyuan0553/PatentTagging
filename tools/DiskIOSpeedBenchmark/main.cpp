//
// Created by yuan on 8/5/19.
//

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <XmlFileReader.h>
#include <PatentTagTextCollector.h>
#include "XmlPCProcessorInterface.h"


using namespace std;

using OutputQueueByFile = unordered_map<string, vector<string>>;


class DoNothingWriter : public ThreadJob<CQueue<string>&> {
    vector<string> filenames_;

    void internalRun(CQueue<string>& data) final
    {
        for (;;)
        {
            auto [filename, quit] = data.pop();

            if (quit) break;

            filenames_.push_back(filename);
        }

        cout << filenames_.size() << '\n';
    }
};


class DoNothingCollector : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    void internalRun() final
    {
        for (;;)
        {
            pugi::xml_document doc;
            auto [filename, quit] = filenameQueue_.pop();

            if (quit) break;

            pugi::xml_parse_result result = doc.load_file(filename.c_str());
        }
    }
public:
    DoNothingCollector(CQueue<string>& filenameQueue) : filenameQueue_(filenameQueue) { }
};


class DoNothingReader : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;
            ifstream ifs(filename);

            stringstream buffer;
            buffer << ifs.rdbuf();
        }
    }
public:
    DoNothingReader(CQueue<string>& filenameQueue) : filenameQueue_(filenameQueue) { }
};



class DiskIOSpeedBenchmark : public XmlPCProcessorInterface {
    string pathFilename_;

    CQueue<string> filenameQueue_;

    void prepareNodeFilters() final { }
    void prepareOutputFormatters() final { }
    void initializeData() final
    {
        XmlFileReader xmlFileReader(pathFilename_, filenameQueue_);
        xmlFileReader.runOnMain();
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++)
            producers_.add<DoNothingReader>(filenameQueue_);
    }

    void executeThreads() final
    {
        producers_.runAll();
        producers_.waitAll();
    }

public:
    DiskIOSpeedBenchmark(string_view pathFilename, int nFileReaders) :
        pathFilename_(pathFilename)
    { nProducers_ = nFileReaders; }
};

void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file> <num-threads>\n", program);
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        printUsageAndExit(argv[0]);

    string pathFilename(argv[1]);
    DiskIOSpeedBenchmark diskIoSpeedBenchmark(pathFilename, atoi(argv[2]));

    diskIoSpeedBenchmark.process();

    return 0;
}
