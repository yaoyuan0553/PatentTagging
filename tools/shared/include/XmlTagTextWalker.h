//
// Created by yuan on 7/26/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTWALKER_H
#define TOOLS_XMLTAGTEXTWALKER_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pugixml.hpp>

#include "Utility.h"
#include "TagNodeFilterFunctors.h"


class XmlTagTextWalker : public pugi::xml_tree_walker {

    TagTextDict tagTextDict_;
    TagNodeFilterDict tagNodeFilterDict_;

public:

    explicit XmlTagTextWalker(const TagNodeFilterDict& tagNodeFilterDict);

    bool for_each(pugi::xml_node& node) override;

    TagTextDict& getTagTexts() { return tagTextDict_; }

    const TagTextDict& getTagTexts() const { return tagTextDict_; }

    void reset();
};


#endif //TOOLS_XMLTAGTEXTWALKER_H
