//
// Created by yuan on 8/15/19.
//

#include "XmlTagTextPrinterWriterThread.h"

#include <fstream>

using namespace std;

void XmlTagTextPrinterWriterThread::internalRun()
{
    ofstream outputFile(filename_);

    if (!outputFile.is_open()) {
        cerr << "file not opened\n";
        return;
    }

    for (int i = 0;; i++)
    {
        auto [info, quit] = inputData_.pop();

        if (quit) break;

        outputFile << info;
    }

    cout << "XmlTagTextPrinterWriterThread finished\n";

    outputFile.close();
}
