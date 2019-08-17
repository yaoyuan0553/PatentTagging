//
// Created by yuan on 8/17/19.
//

#pragma once
#ifndef TOOLS_DATABASEQUERY_H
#define TOOLS_DATABASEQUERY_H


#include <filesystem>
#include <string>
#include "DataTypes.h"


class DatabaseQuery {
    std::filesystem::path indexFilename_;
    std::filesystem::path dataPath_;
    std::filesystem::path dataFilePrefix_;

    IndexTable indexTable_;
public:
    DatabaseQuery(std::string_view indexFilename, std::string_view dataPath,
            std::string_view dataFilePrefix){ }
};


#endif //TOOLS_DATABASEQUERY_H
