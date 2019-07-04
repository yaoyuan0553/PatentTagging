//
// Created by yuan on 7/4/19.
//

#pragma once
#ifndef TOOLS_PATENTTAGWALKER_H
#define TOOLS_PATENTTAGWALKER_H

#include <string>
#include <unordered_set>

#include <pugixml.hpp>

struct PatentTagWalker : pugi::xml_tree_walker {
    const char* curFilename = "";
    std::unordered_set<std::string> uniqueTags;

    bool for_each(pugi::xml_node& node) override;
};


#endif //TOOLS_PATENTTAGWALKER_H
