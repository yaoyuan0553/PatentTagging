//
// Created by yuan on 8/13/19.
//

#pragma once
#ifndef TOOLS_NODEFILTERS_H
#define TOOLS_NODEFILTERS_H

#include <TagNodeFilterFunctors.h>


class IdNodeWalker : public pugi::xml_tree_walker {
    std::string innerText_;
public:
    bool for_each(pugi::xml_node& node) override;
    std::string getInnerText() const { return innerText_; }
    void reset() { innerText_.clear(); }
};


class IdNodeFilter : public TagNodeFilter {
    IdNodeWalker walker_;
public:

    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(IdNodeFilter);
};


#endif //TOOLS_NODEFILTERS_H
