//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_PATENTTAGABSTRACTWALKER_H
#define TOOLS_PATENTTAGABSTRACTWALKER_H

#include <string>
#include <unordered_set>

#include <pugixml.hpp>


struct PatentTagAbstractWalker : pugi::xml_tree_walker {

    std::unordered_set<std::string> uniqueTags;
    std::string abstract = "";

    bool isIrregular = false;

    bool for_each(pugi::xml_node& node) override;

    /* resets patent tag to initial stage */
    void reset();
};


#endif //TOOLS_PATENTTAGABSTRACTWALKER_H
