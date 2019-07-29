//
// Created by yuan on 7/26/19.
//

#include "XmlTagTextWalker.h"

#include <string>
#include <string.h>

bool XmlTagTextWalker::for_each(pugi::xml_node &node)
{
    if (auto tagNode = tagCollection.find(std::string(node.name()));
        tagNode != tagCollection.end())
    {
        if (node.find_child([](pugi::xml_node& childNode) {
            return strcmp(childNode.name(), "text") == 0;
        }))
        {
            const char* textTag = node.child("text").text().get();
            int tagLen = strlen(textTag);
            if (tagLen != 0)
            {
                tagNode->second =
                        ReplaceDelimiter<'-'>(std::string(textTag, textTag + tagLen));

            }
        }
    }

    return true;
}

