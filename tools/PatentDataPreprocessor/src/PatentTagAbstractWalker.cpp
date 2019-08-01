//
// Created by yuan on 7/24/19.
//

#include "PatentTagAbstractWalker.h"

#include <string>
#include <sstream>
#include <iostream>
#include <string.h>

#include "Utility.h"

std::initializer_list<char32_t> PatentTagAbstractWalker::separators = {
        u'。'
};

bool PatentTagAbstractWalker::for_each(pugi::xml_node &node)
{
    if (strcmp(node.name(), "classification-ipcr") == 0) {
        if (node.find_child([](pugi::xml_node& childNode) {
            return strcmp(childNode.name(), "text") == 0;
        }))
        {
            const char* patentTag = node.child("text").text().get();
            int tagLen = strlen(patentTag);
            if (tagLen == 0)
                isIrregular = true;
            else {
//                if (auto space = strchr(patentTag, ' '); space)
//                    tagLen = space - patentTag;
                std::string processedTag =
                        ReplaceDelimiter<'-'>(std::string(patentTag, patentTag + tagLen));
                uniqueTags.insert(processedTag);
            }
        }
        else {
            isIrregular = true;
        }
    }

    if (strcmp(node.name(), "abstract") == 0) {
        if (node.find_child([](pugi::xml_node& childNode) {
            return strcmp(childNode.name(), "p") == 0;
        }))
        {
            abstract = node.child("p").text().get();
            splitAbstract = split(abstract, '\n');
        }
        else
            isIrregular = true;
    }

    return true;
}

void PatentTagAbstractWalker::reset()
{
    uniqueTags.clear();
    abstract.clear();
    splitAbstract.clear();
    isIrregular = false;
}
