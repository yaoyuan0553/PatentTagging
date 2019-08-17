//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_DATABASEGENERATOR_H
#define TOOLS_DATABASEGENERATOR_H

#include "XmlXpathIPOGenericInterface.h"
#include "XmlPathFileReader.h"
#include "DataTypes.h"

#include "DataAndIndexGeneratorThread.h"
#include "DatabaseFileWriterThread.h"


#define ISC inline static constexpr
#define ISC_STRING(name) ISC char name[]


class DatabaseGenerator : public XmlXpathIPOGenericInterface {

    std::filesystem::path dataPrefixName_ = DATA_FILE_PREFIX_NAME;

    std::filesystem::path pathFilename_;
    std::filesystem::path dataStorageDir_;
    std::filesystem::path indexFilname_;

    ConcurrentQueue<std::string> filenameQueue_;
    CQueue<std::pair<char*, size_t>> xmlDocQueue_;
    CQueue<DataRecordFile*> dataRecordFileQueue_;

    XpathQueryTextFormatterDict xpathQueryTextFormatterDict_;


    /* index file fields */
    ISC_STRING(PID) = "pid";
    ISC_STRING(AID) = "aid";
    ISC_STRING(PUB_DATE) = "pubDate";
    ISC_STRING(APP_DATE) = "appDate";
    ISC_STRING(IPC) = "ipc";                // TODO

    /* data file fields */
    ISC_STRING(TITLE) = "title";            // TODO
    ISC_STRING(ABSTRACT) = "abstract";
    ISC_STRING(CLAIM) = "claim";
    ISC_STRING(DESCRIPTION) = "description";

    std::vector<std::string> dataTextKeys_ = { TITLE, ABSTRACT, CLAIM, DESCRIPTION };
    std::vector<std::string> indexTextKeys_ = { PID, AID, APP_DATE, IPC };

    void initializeQuery() final
    {
        using namespace std;

        xpathQueryTextFormatterDict_.add<XpathIdQuery>(
                PID, "//publication-reference",
                vector<XpathQueryString>{".//country", ".//doc-number", ".//kind"}
        );

        xpathQueryTextFormatterDict_.add<XpathIdQuery>(
                AID, "//application-reference",
                vector<XpathQueryString>{".//country", ".//doc-number"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery>(
                PUB_DATE, "//publication-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery>(
                APP_DATE, "//application-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathIpcQuery>(
                IPC, ".//classification-ipcr"
        );

        xpathQueryTextFormatterDict_.add<XpathTitleQuery>(
                TITLE, "//invention-title"
        );

        xpathQueryTextFormatterDict_.add<XpathAbstractQuery>(
                ABSTRACT, "//abstract"
        );

        xpathQueryTextFormatterDict_.add<XpathClaimQuery>(
                CLAIM, "//claim"
        );

        xpathQueryTextFormatterDict_.add<XpathDescriptionQuery>(
                DESCRIPTION, "//description"
        );
    }

    void initializeData() final
    {
        XmlPathFileReader xmlPathFileReader(pathFilename_, filenameQueue_);
        xmlPathFileReader.runOnMain();
    }

    void initializeThreads() final
    {
        readerPool_.add<FileReaderThread>(nReaders_, filenameQueue_, xmlDocQueue_, 128);

        // TODO: test processor
        processorPool_.add<DataAndIndexGeneratorThread>(nProcessors_,
                xmlDocQueue_, dataRecordFileQueue_,
                xpathQueryTextFormatterDict_,
                dataTextKeys_,
                indexTextKeys_
        );
        // TODO: test writer thread
        writerPool_.add<DatabaseFileWriterThread>(nWriters_,
                dataRecordFileQueue_, dataStorageDir_,
                dataPrefixName_, indexFilname_
        );
    }

    void executeThreads() final
    {
        StatsThread<std::pair<char*, size_t>, true> processedStats(
                xmlDocQueue_,
                filenameQueue_.totalPushedItems(),
                "xmls parsed");
        StatsThread<DataRecordFile*, false> writtenStats(dataRecordFileQueue_,
                "data files written");

        readerPool_.runAll();
        processorPool_.runAll();
        writerPool_.runAll();
        processedStats.run();

        readerPool_.waitAll();
        xmlDocQueue_.setQuitSignal();

        processorPool_.waitAll();
        dataRecordFileQueue_.setQuitSignal();
        processedStats.wait();

        writtenStats.run();

        writerPool_.waitAll();
        writtenStats.wait();
    }

public:
    DatabaseGenerator(std::string_view pathFilename,
            std::string_view dataStorageDir,
            std::string_view indexFilename,
            int nReaders, int nProcessors) :
            XmlXpathIPOGenericInterface(nReaders, nProcessors),
            pathFilename_(pathFilename), dataStorageDir_(dataStorageDir),
            indexFilname_(indexFilename)
    {
        std::filesystem::create_directory(dataStorageDir);
    }
};


#endif //TOOLS_DATABASEGENERATOR_H
