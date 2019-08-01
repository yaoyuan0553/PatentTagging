//
// Created by yuan on 8/1/19.
//

#pragma once
#ifndef TOOLS_TAGNODEFILTERFUNCTORS_H
#define TOOLS_TAGNODEFILTERFUNCTORS_H

#include <pugixml.hpp>

#include "FunctorDict.h"
#include "TagConstants.h"


class ExhaustiveChildWalker : public pugi::xml_tree_walker {
    std::string innerText_;
public:
    bool for_each(pugi::xml_node& node) override;
    std::string getInnerText() const { return innerText_; }
    void reset() { innerText_.clear(); }
};


struct TagNodeFilter : public Cloneable {
    virtual std::string operator()(pugi::xml_node& node) = 0;
    DECLARE_ABSTRACT_CLONE(TagNodeFilter);

    virtual ~TagNodeFilter() = default;
};


struct ClassificationNodeFilter : public TagNodeFilter {
    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(ClassificationNodeFilter);
};


struct AbstractNodeFilter : public TagNodeFilter {
    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(AbstractNodeFilter);
};


class AbstractGreedyNodeFilter : public TagNodeFilter {
    ExhaustiveChildWalker childWalker_;
public:
    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(AbstractGreedyNodeFilter);
};

using TagNodeFilterDict =
        FunctorDict<TagNodeFilter, std::string, pugi::xml_node&>;


#endif //TOOLS_TAGNODEFILTERFUNCTORS_H
