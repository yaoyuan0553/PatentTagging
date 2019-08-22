//
// Created by yuan on 8/15/19.
//

#include "2004.h"

#include <iostream>

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

        if (walker_.getInnerText().empty()) continue;
//        output += RemoveExtraWhitespace(walker_.getInnerText()) + '\n';
        output += walker_.getInnerText() + '\n';
    }

    if (!output.empty())
        output.pop_back();

    return output;
}

std::string XpathIpcQuery::operator()(pugi::xml_node& root)
{
    pugi::xpath_node_set ipcNodes = root.select_nodes(ipcNodesQuery_.pugiQuery());

    string output;
    /* size 5: 0      1             2             3             4
     * .//section | .//class | .//subclass | .//main-group | .//subgroup */
    for (const pugi::xpath_node& ipc : ipcNodes) {
        pugi::xpath_node_set subNodes = ipc.node().select_nodes(subNodesQuery_.pugiQuery());

        if (subNodes.size() != 5) continue;

        subNodes.sort();

        output += string(subNodes[0].node().text().get()) +
                subNodes[1].node().text().get() +
                subNodes[2].node().text().get() + '-' +
                subNodes[3].node().text().get() + '/' +
                subNodes[4].node().text().get() + ',';
    }
    if (!output.empty())
        output.pop_back();

    return output;
}
