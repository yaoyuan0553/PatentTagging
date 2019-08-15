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


template <typename Walker>
class TagNodeWalkerFilter : public TagNodeFilter {
    static_assert(std::is_base_of_v<pugi::xml_tree_walker, Walker>,
            "Walker must of subclass of pugi::xml_tree_walker");
    Walker walker_;
public:
    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(TagNodeWalkerFilter);

    ~TagNodeWalkerFilter() override = default;
};

template <typename Walker>
std::string TagNodeWalkerFilter<Walker>::operator()(pugi::xml_node& node)
{
    node.traverse(walker_);

    std::string content = walker_.getInnerText();

    walker_.reset();

    return content;
}

using IdNodeFilter = TagNodeWalkerFilter<IdNodeWalker>;
class IdDateNodeFilter : public TagNodeWalkerFilter<IdNodeWalker> {

};

using ClaimNodeWalker = ExhaustiveChildWalker;

using ClaimNodeFilter = TagNodeWalkerFilter<ClaimNodeWalker>;


class UsClassificationWalker : public pugi::xml_tree_walker {
    std::string innerText_;
public:
    bool for_each(pugi::xml_node& node) override;
    std::string getInnerText() const { return innerText_; }
    void reset() { innerText_.clear(); }
};


class UsClassificationNodeFilter : public TagNodeFilter {
public:
    std::string operator()(pugi::xml_node& node) final;
    DEFINE_DEFAULT_CLONE(UsClassificationNodeFilter);
};

/*
class ClaimNodeFilter : public TagNodeFilter {
    ClaimNodeWalker walker_;
public:
    std::string operator()(pugi::xml_node& node) override;

    DEFINE_DEFAULT_CLONE(ClaimNodeFilter);
};
*/


#endif //TOOLS_NODEFILTERS_H
