//
// Created by yuan on 7/24/19.
//

#include "XmlInfoWriter.h"

#include <iostream>


void XmlInfoWriter::internalRun(ConcurrentQueue<std::string> &outputInfoQueue)
{
    using namespace std;

    ofstream outputFile(filename_);
    if (!outputFile.is_open()) {
        cerr << "file not opened\n";
        return;
    }

    for (int i = 0;; i++)
    {
        auto [info, quit] = outputInfoQueue.pop();

        if (quit) break;

        outputFile << info;
    }

    cout << "XmlInfoWriter thread finished\n";

    outputFile.close();
}
