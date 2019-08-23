//
// Created by yuan on 8/16/19.
//

#include "DataAndIndexGeneratorThread.h"

#include <fstream>

using namespace std;

void DataAndIndexGeneratorThread::internalRun()
{
    // increment counter by 1 every time a thread starts running
    nRunning++;

    for (;;)
    {
        auto [xmlFile, quit] = inputData_.pop();

        if (quit) break;

        pugi::xml_document doc;

        // NOTE: this will deallocate buffer in doc automatically
        if (!doc.load_buffer_inplace_own(xmlFile.buffer, xmlFile.bufferSize)) {
            fprintf(stderr, "Error parsing xml file\n");
            continue;
        }

        vector<string> dataText;
        vector<string> indexText;
        try {
            // skip files that have tags that don't match
            // TODO: log which files don't match
            bool emptyFlag = false;
            string unmatchedKey;

            for (const string& dataKey : dataTextKeys_) {
                string dt = xpathQueryTextFormatterDict_[dataKey](doc);
                if (dt.empty()) {
                    emptyFlag = true;
                    unmatchedKey = dataKey;
                    break;
                }
                dataText.emplace_back(dt);
            }
            // skip files that have tags that don't match
            if (emptyFlag) {
                // record files that don't match
                errorFileQueue.push(XmlErrorFile{xmlFile.filename, std::move(unmatchedKey)});
                continue;
            }

            for (const string& indexKey : indexTextKeys_) {
                string it = xpathQueryTextFormatterDict_[indexKey](doc);
                if (it.empty()) {
                    emptyFlag = true;
                    unmatchedKey = indexKey;
                    break;
                }
                indexText.emplace_back(it);
            }

            // skip files that have tags that don't match
            if (emptyFlag) {
                // record files taht don't match
                errorFileQueue.push(XmlErrorFile{xmlFile.filename, std::move(unmatchedKey)});
                continue;
            }
        }
        catch (std::out_of_range& e) {
            cerr << "out of range here " << e.what() << '\n';
            PERROR("out_of_range()");
        }

        /* check if dataRecordFile is full, if full push to queue and init a new one */
        if (!dataRecordFile_->appendRecord(dataText, indexText)) {
            outputData_.push(dataRecordFile_);
            // WARNING: must be released by caller
            dataRecordFile_ = new DataRecordFile;
        }
    }
    /* push to queue if there is remaining data */
    if (!dataRecordFile_->empty())
        outputData_.push(dataRecordFile_);

    /* last thread writes the error files */
    if (--nRunning == 0)
        writeErrorFileToFile();
}

void DataAndIndexGeneratorThread::writeErrorFileToFile() const
{
    errorFileQueue.setQuitSignal();

    constexpr char errorFilename[] = "error.txt";

    ofstream errorFile(errorFilename);
    if (!errorFile.is_open()) {
        fprintf(stderr, "failed to open [%s]\n", errorFilename);
        PERROR("ofstream");
    }

    for (auto res = errorFileQueue.pop(); !res.second; res = errorFileQueue.pop())
    {
        errorFile << res.first.filename << ":\n" << res.first.tagName << "\n\n";
    }

    errorFile.close();
}
