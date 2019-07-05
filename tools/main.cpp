#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>

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
    printf("Usage:\n\ttools <path-file>\n\n<path-file>\t\tfile storing paths to xml files"
           "needed to be parsed\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        printUsageAndExit();

    string pathFilename(argv[1]);

//    ifstream ifs(pathFilename);
//
//    if (ifs.is_open()) {
//     printf("file opened\n");
//     string line;
//     getline(ifs, line);
//
//     cout << line << endl;
//    }

    ThreadDispatcher threadDispatcher(pathFilename, 10, 1);

    threadDispatcher.join();

    return 0;
}
