//
// Created by yuan on 8/18/19.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYSELECTOR_H
#define TOOLS_DATABASEQUERYSELECTOR_H

#include <string>

#include "DatabaseQueryManager.h"


extern "C"
{


    IndexValue* IndexValue_new();

    const char* IndexValue_stringify(IndexValue* iv);


    /* DatabaseQueryManager */
    DatabaseQueryManager* DatabaseQueryManager_new(
            char* indexFilename,
            char* dataPath);

    void DatabaseQueryManager_delete(DatabaseQueryManager* dqm);

    bool DatabaseQueryManager_getInfoById(const std::string& id, IndexValue** output);


    void DatabaseQueryManager_getAllId(DatabaseQueryManager* dqm,
            char**& pidList,
            char**& aidList,
            int& size);

    DataRecordCType DatabaseQueryManager_getContentById(DatabaseQueryManager* dqm,
            const char* id/*, DataRecordCType* drct*/);

    /* deallocate functions */
    void delete_arrayPtr(void* arrPtr);
    void delete_ptr(void* ptr);
}



#endif //TOOLS_DATABASEQUERYSELECTOR_H
