//
// Created by yuan on 8/5/19.
//

#include "ClassificationStatsWriter.h"

#include <fstream>

void ClassificationStatsWriter::internalRun(CQueue<std::string>& data)
{
    using namespace std;

    for (int i = 0;; i++)
    {
        auto [tag, quit] = data.pop();

        if (quit) break;

        countByTag_[tag]++;
    }

    ofstream outputFile(filename_);
    for (const auto& [tag, count] : countByTag_)
        outputFile << tag << '\t' << count << '\n';

    outputFile.close();
}
