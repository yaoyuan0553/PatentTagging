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


int main()
{
    ThreadDispatcher threadDispatcher(1, 1);
    printf("threads started\n");

    threadDispatcher.join();

    return 0;
}
