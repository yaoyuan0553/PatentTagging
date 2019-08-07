//
// Created by yuan on 8/7/19.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>

#include <stdio.h>

using namespace std;

void loadRemoveClassSet(unordered_set<string>& classSet, const string& filename)
{
    ifstream ifs(filename);
    if (!ifs.is_open()) {
        fprintf(stderr, "%s: file [%s] failed to open\n", __FUNCTION__, filename.c_str());
        exit(-1);
    }

    for (string line; getline(ifs, line);)
        classSet.insert(line);

    ifs.close();
}


void removeRecordWithClass(const unordered_set<string>& classSet,
        const string& inputFilename, const string& outputFilename)
{
    ofstream ofs(outputFilename);
    if (!ofs.is_open()) {
        fprintf(stderr, "%s: file [%s] failed to open\n", __FUNCTION__, outputFilename.c_str());
        exit(-1);
    }

    ifstream ifs(inputFilename);
    if (!ifs.is_open()) {
        fprintf(stderr, "%s: file [%s] failed to open\n", __FUNCTION__, inputFilename.c_str());
        exit(-1);
    }

    for (string line; getline(ifs, line);)
    {
        size_t firstTab = line.find_first_of('\t');
        size_t commaOrTab = line.find_first_of(",\t", firstTab + 1);
        if (classSet.find(line.substr(firstTab+1, commaOrTab - firstTab - 1)) == classSet.end())
            ofs << line << '\n';
    }
}


void printUsageAndExit(const char* filename)
{
    printf("Usage:\n\t%s <class-to-be-removed-file> <record-file> <output-new-record-file>\n",
            filename);
    exit(-1);
}


int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    unordered_set<string> classSet;
    loadRemoveClassSet(classSet, argv[1]);
    removeRecordWithClass(classSet, argv[2], argv[3]);

    return 0;
}
