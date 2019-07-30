//
// Created by yuan on 7/26/19.
//

#pragma once
#ifndef TOOLS_XMLTAGTEXTWALKER_H
#define TOOLS_XMLTAGTEXTWALKER_H

#include <unordered_map>
#include <vector>

#include <pugixml.hpp>

#include "Utility.h"


using TagTextDict = std::unordered_map<std::string, std::vector<std::string>>;


class XmlTagTextWalker : public pugi::xml_tree_walker {

    TagTextDict tagTextDict_;

public:

    XmlTagTextWalker(std::initializer_list<std::string>&& tags);

    bool for_each(pugi::xml_node& node) override;

    const TagTextDict& getTagTexts() const { return tagTextDict_; }

    void reset();
};


#endif //TOOLS_XMLTAGTEXTWALKER_H
