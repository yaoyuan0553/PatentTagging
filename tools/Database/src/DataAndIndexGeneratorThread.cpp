//
// Created by yuan on 8/16/19.
//

#include "DataAndIndexGeneratorThread.h"


using namespace std;

void DataAndIndexGeneratorThread::internalRun()
{
    for (;;)
    {
        auto [bufBytes, quit] = inputData_.pop();

        if (quit) break;

        pugi::xml_document doc;

        // NOTE: this will deallocate buffer in doc automatically
        if (!doc.load_buffer_inplace_own(bufBytes.first, bufBytes.second)) {
            fprintf(stderr, "Error parsing xml file\n");
            continue;
        }

        vector<string> dataText;
        vector<string> indexText;
        try {
            // skip files that have tags that don't match
            // TODO: log which files don't match
            bool emptyFlag = false;

            for (const string& dataKey : dataTextKeys_) {
                string dt = xpathQueryTextFormatterDict_[dataKey](doc);
                if (dt.empty()) {
                    emptyFlag = true;
                    break;
                }
                dataText.emplace_back(dt);
            }
            // skip files that have tags that don't match
            if (emptyFlag) continue;

            for (const string& indexKey : indexTextKeys_) {
                string it = xpathQueryTextFormatterDict_[indexKey](doc);
                if (it.empty()) {
                    emptyFlag = true;
                    break;
                }
                indexText.emplace_back(it);
            }

            // skip files that have tags that don't match
            if (emptyFlag) continue;
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
}
