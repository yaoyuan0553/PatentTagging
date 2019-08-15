//
// Created by yuan on 8/15/19.
//

#include "XpathQueryCollection.h"

using namespace std;


string XpathIdQuery::operator()(pugi::xml_node& root)
{
    pugi::xml_node idRootNode = root.select_node(idRootQuery_.pugiQuery()).node();

    string output;

    for (const XpathQuery& xqs : idNodeQueryList_) {
        auto str = idRootNode.select_node(xqs.pugiQuery()).node().text();
        if (str.empty())
            continue;
        output += str.get();
    }

    return output;
}

std::string XpathAbstractQuery::operator()(pugi::xml_node& root)
{
    pugi::xml_node idRootNode = root.select_node(abstractQuery_.pugiQuery()).node();

    string output;

    for (const pugi::xpath_node& pNode : idRootNode.select_nodes(paragraphQuery_.pugiQuery())) {

        walker_.reset();
        pNode.node().traverse(walker_);

        if (walker_.getInnerText().empty()) continue;

//        output += RemoveExtraWhitespace(walker_.getInnerText()) + '\n';
        output += walker_.getInnerText() + '\n';
    }
    /* pop the last new line*/
    if (!output.empty())
        output.pop_back();

    return output;
}

std::string XpathClaimQuery::operator()(pugi::xml_node& root)
{
    pugi::xpath_node_set claimNodes = root.select_nodes(claimQuery_.pugiQuery());

    claimNodes.sort();

    string output;
    for (const pugi::xpath_node& claimNode : claimNodes) {
        walker_.reset();
        claimNode.node().traverse(walker_);

//        output += RemoveExtraWhitespace(walker_.getInnerText()) + '\n';
        output += walker_.getInnerText() + '\n';
    }

    if (!output.empty())
        output.pop_back();

    return output;
}
