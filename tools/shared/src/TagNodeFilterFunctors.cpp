//
// Created by yuan on 8/1/19.
//


#include "TagNodeFilterFunctors.h"

#include <string.h>
#include <iostream>


bool ExhaustiveChildWalker::for_each(pugi::xml_node& node)
{
    if (node.type() == pugi::node_pcdata)
        innerText_ += node.text().get();

    return true;
}

std::string ClassificationNodeFilter::operator()(pugi::xml_node& node)
{
    constexpr char childNodeName[] = "text";
    if (node.find_child([&](pugi::xml_node& childNode) {
        return strcmp(childNode.name(), childNodeName) == 0;
    }))
    {
        const char* textContent = node.child(childNodeName).text().get();
        if (strlen(textContent) > 0)
            return textContent;
    }
    return "";
}

std::string AbstractNodeFilter::operator()(pugi::xml_node& node)
{
    constexpr char childNodeName[] = "p";
    if (node.find_child([&](pugi::xml_node& childNode) {
        return strcmp(childNode.name(), childNodeName) == 0;
    }))
    {
        const char* textContent = node.child(childNodeName).text().get();
        if (strlen(textContent) > 0)
            return textContent;
    }
    return "";
}

std::string AbstractGreedyNodeFilter::operator()(pugi::xml_node& node)
{
    node.traverse(childWalker_);

    std::string content = childWalker_.getInnerText();

    childWalker_.reset();

    return content;
}

