//
// Created by yuan on 8/15/19.
//

#include "XpathQueryTextFormatter.h"

#include <iostream>


using namespace std;


std::string XpathSingleQueryDefaultInnerText::operator()(pugi::xml_node& root)
{
    pugi::xml_node resNode = root.select_node(query_.pugiQuery()).node();

//    printf("%s: %s\n", resNode.name(), resNode.text().get());

    return resNode.text().get();
}


std::string XpathSingleQueryGreedyInnerText::operator()(pugi::xml_node& root)
{
    pugi::xml_node resNode = root.select_node(query_.pugiQuery()).node();

    walker_.reset();

    resNode.traverse(walker_);

    return walker_.getInnerText();
}

std::string XpathSingleQueryGreedyNoExtraSpaceInnerText::operator()(pugi::xml_node& root)
{
    return RemoveExtraWhitespace(Base::operator()(root));
}
