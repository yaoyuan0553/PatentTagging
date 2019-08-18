//
// Created by yuan on 7/26/19.
//

#include "XmlTagTextWalker.h"

#include <string>
#include <string.h>


XmlTagTextWalker::XmlTagTextWalker(const TagNodeFilterDict& tagNodeFilterDict) :
    tagNodeFilterDict_(tagNodeFilterDict)
{
    for (const std::string& tag : tagNodeFilterDict_.getKeys())
        tagTextDict_[tag] = std::vector<std::string>();
}

bool XmlTagTextWalker::for_each(pugi::xml_node &node)
{
    if (auto tagNode = tagTextDict_.find(std::string(node.name()));
        tagNode != tagTextDict_.end())
    {
        std::string filteredText = tagNodeFilterDict_[tagNode->first](node);
        if (!filteredText.empty())
            tagNode->second.emplace_back(filteredText);
/*
        if (node.find_child([](pugi::xml_node& childNode) {
            return strcmp(childNode.name(), "text") == 0;
        }))
        {
            const char* textTag = node.child("text").text().get();
            int tagLen = strlen(textTag);
            if (tagLen != 0)
            {
                tagNode->second.emplace_back(textTag);
            }
        }
*/
    }

    return true;
}


void XmlTagTextWalker::reset()
{
    for (auto& p : tagTextDict_)
        p.second.clear();
}
