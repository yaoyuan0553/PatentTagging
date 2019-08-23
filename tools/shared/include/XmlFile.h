//
// Created by yuan on 2019/8/23.
//

#pragma once
#ifndef TOOLS_XMLFILE_H
#define TOOLS_XMLFILE_H

#include <string>

struct XmlFile {
    std::string filename;
    char* buffer;
    uint64_t bufferSize;
};

#endif //TOOLS_XMLFILE_H
