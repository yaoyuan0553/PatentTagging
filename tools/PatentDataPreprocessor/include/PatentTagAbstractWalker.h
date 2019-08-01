//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_PATENTTAGABSTRACTWALKER_H
#define TOOLS_PATENTTAGABSTRACTWALKER_H

#include <string>
#include <unordered_set>

#include <pugixml.hpp>

#include "Utility.h"


struct PatentTagAbstractWalker : pugi::xml_tree_walker {

    static std::initializer_list<char32_t> separators;

    SplitParagraph split;

    std::unordered_set<std::string> uniqueTags;
    std::string abstract;
    std::string splitAbstract;

    bool isIrregular = false;

    bool for_each(pugi::xml_node& node) override;

    /* resets patent tag to initial stage */
    void reset();

    PatentTagAbstractWalker() : split(separators) { }
};


#endif //TOOLS_PATENTTAGABSTRACTWALKER_H
