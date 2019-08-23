//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_DATABASEGENERATOR_H
#define TOOLS_DATABASEGENERATOR_H

#include "XmlXpathIPOGenericInterface.h"
#include "XmlPathFileReader.h"
#include "DataTypes.h"

#include "XmlFile.h"

#include "DataAndIndexGeneratorThread.h"
#include "DatabaseFileWriterThread.h"

//#include "XpathQueryCollection.h"
#include "XpathQueryCollection/2019.h"
#include "XpathQueryCollection/2004.h"
#include "XpathQueryCollection/2005.h"

#define ISC inline static constexpr
#define ISC_STRING(name) ISC char name[]


class DatabaseGenerator : public XmlXpathIPOGenericInterface {

    std::filesystem::path dataPrefixName_ = DATA_FILE_PREFIX_NAME;

    std::filesystem::path pathFilename_;
    std::filesystem::path dataStorageDir_;
    std::filesystem::path indexFilname_;

    ConcurrentQueue<std::string> filenameQueue_;
//    CQueue<std::pair<char*, size_t>> xmlDocQueue_;
    CQueue<XmlFile> xmlDocQueue_;
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

    void initializeQuery2019()
    {
        using namespace std;

        xpathQueryTextFormatterDict_.add<XpathIdQuery2019>(
                PID, "//publication-reference",
                vector<XpathQueryString>{".//country", ".//doc-number", ".//kind"}
        );

        xpathQueryTextFormatterDict_.add<XpathIdQuery2019>(
                AID, "//application-reference",
                vector<XpathQueryString>{".//country", ".//doc-number"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2019>(
                PUB_DATE, "//publication-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2019>(
                APP_DATE, "//application-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathIpcQuery2019>(
                IPC, ".//classification-ipcr"
        );

        xpathQueryTextFormatterDict_.add<XpathTitleQuery2019>(
                TITLE, "//invention-title"
        );

        xpathQueryTextFormatterDict_.add<XpathAbstractQuery2019>(
                ABSTRACT, "//abstract"
        );

        xpathQueryTextFormatterDict_.add<XpathClaimQuery2019>(
                CLAIM, "//claim"
        );

        xpathQueryTextFormatterDict_.add<XpathDescriptionQuery2019>(
                DESCRIPTION, "//description"
        );
    }

    void initializeQuery2005()
    {
        //initializeQuery  for xml in [2005,2005]
//        xpathQueryTextFormatterDict_.add<XpathSingleQueryGreedyNoExtraSpaceInnerText>(
//                "claimText", XpathQueryString("//claim-text")
//                );
        using namespace std;
        xpathQueryTextFormatterDict_.add<XpathIdQuery2005>(
                PID, "//publication-reference",
                vector<XpathQueryString>{".//country", ".//doc-number", ".//kind"}
        );

        xpathQueryTextFormatterDict_.add<XpathIdQuery2005>(
                AID, "//application-reference",
                vector<XpathQueryString>{".//country", ".//doc-number"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2005>(
                PUB_DATE, "//publication-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2005>(
                APP_DATE, "//application-reference",
                vector<XpathQueryString>{".//date"}
        );

        xpathQueryTextFormatterDict_.add<XpathIpcQuery2005>(
                IPC, ".//classification-ipc"
        );

        xpathQueryTextFormatterDict_.add<XpathTitleQuery2005>(
                TITLE, "//invention-title"
        );

        xpathQueryTextFormatterDict_.add<XpathAbstractQuery2005>(
                ABSTRACT, "//abstract"
        );

        xpathQueryTextFormatterDict_.add<XpathClaimQuery2005>(
                CLAIM, "//claim"
        );

        xpathQueryTextFormatterDict_.add<XpathDescriptionQuery2005>(
                DESCRIPTION, "//description"
        );
    }

    void initializeQuery2004()
    {
        //initializeQuery  for xml in [:,2004]
//        xpathQueryTextFormatterDict_.add<XpathSingleQueryGreedyNoExtraSpaceInnerText>(
//                "claimText", XpathQueryString("//claim-text")
//                );
        using namespace std;
        xpathQueryTextFormatterDict_.add<XpathIdQuery2004>(
                PID, "//subdoc-bibliographic-information",
                vector<XpathQueryString>{"./document-id/doc-number", ".//kind-code"}
        );

        xpathQueryTextFormatterDict_.add<XpathIdQuery2004>(
                AID, "//subdoc-bibliographic-information",
                vector<XpathQueryString>{"./domestic-filing-data/application-number/doc-number"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2004>(
                PUB_DATE, "//subdoc-bibliographic-information",
                vector<XpathQueryString>{"./document-id/document-date"}
        );

        xpathQueryTextFormatterDict_.add<XpathDateQuery2004>(
                APP_DATE, "//subdoc-bibliographic-information",
                vector<XpathQueryString>{".//filing-date"}
        );

        xpathQueryTextFormatterDict_.add<XpathIpcQuery2004>(
                IPC, ".//subdoc-bibliographic-information"
        );

        xpathQueryTextFormatterDict_.add<XpathTitleQuery2004>(
                TITLE, "//title-of-invention"
        );

        xpathQueryTextFormatterDict_.add<XpathAbstractQuery2004>(
                ABSTRACT, "//subdoc-abstract"
        );

        xpathQueryTextFormatterDict_.add<XpathClaimQuery2004>(
                CLAIM, "//subdoc-claims"
        );

        xpathQueryTextFormatterDict_.add<XpathDescriptionQuery2004>(
                DESCRIPTION, "//subdoc-description"
        );
    }

    void initializeQuery() final
    {
        initializeQuery2004();
//        initializeQuery2005();
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
        StatsThread<XmlFile, true> processedStats(
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
            indexFilname_(indexFilename),
            dataRecordFileQueue_(nReaders * 2)
    {
        std::filesystem::create_directory(dataStorageDir);
    }
};


#endif //TOOLS_DATABASEGENERATOR_H
