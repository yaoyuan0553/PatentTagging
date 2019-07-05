#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>

#include <tqdm.h>

#include <pugixml.hpp>  // linker "pugixml" needed
#include "ConcurrentQueue.h"  // linker "pthread" needed
#include "PatentTagWalker.h"
#include "PatentTagCollector.h"
#include "ThreadDispatcher.h"


using namespace std;
using namespace pugi;

void test()
{
    string filename;
    xml_document doc;
    PatentTagWalker walker;

    while (getline(cin, filename)) {
        xml_parse_result result = doc.load_file(filename.c_str());
        if (!result) {
            fprintf(stderr, "file [%s] not found or empty!\n", filename.c_str());
            continue;
        }
        walker.curFilename = filename.c_str();
        doc.traverse(walker);
    }

    ofstream outputFile("output.tmp");
    for (const string& tag : walker.uniqueTags) {
        outputFile << tag << "\n";
    }
    outputFile.close();
}

void printUsageAndExit()
{
    printf("Usage:\n\ttools <path-file> <output-file> <n-threads>\n\n"
           "<path-file>\t\tfile storing paths to xml files"
           "needed to be parsed\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit();

    string pathFilename(argv[1]);
    string outputFilename(argv[2]);
    int nWorkers = atoi(argv[3]);

    ThreadDispatcher threadDispatcher(pathFilename, nWorkers, 128);

    threadDispatcher.join();

    threadDispatcher.writeResult2File(outputFilename);

    return 0;
}
