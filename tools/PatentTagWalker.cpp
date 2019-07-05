//
// Created by yuan on 7/4/19.
//

#include "PatentTagWalker.h"
#include <string>
#include <string.h>
#include <stdio.h>

bool PatentTagWalker::for_each(pugi::xml_node& node)
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
                if (auto space = strchr(patentTag, ' '); space)
                    tagLen = space - patentTag;
                std::string tag(patentTag, patentTag + tagLen);
                uniqueTags.insert(tag);
            }
        }
        else {
            isIrregular = true;
        }
    }
    return true;
}
