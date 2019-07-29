//
// Created by yuan on 7/26/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTWALKER_H
#define TOOLS_XMLTAGTEXTWALKER_H

#include <unordered_map>

#include <pugixml.hpp>

#include "Utility.h"


using TagCollection = std::unordered_map<std::string, std::string>;


class XmlTagTextWalker : public pugi::xml_tree_walker {
    ParagraphSplit split;

    TagCollection tagCollection;

public:

    bool for_each(pugi::xml_node& node) override;

    const TagCollection& getTagTexts() const
    {
        return tagCollection;
    }

    void reset()
    {
        for (auto& p : tagCollection)
            p.second.clear();
    }
};


#endif //TOOLS_XMLTAGTEXTWALKER_H
