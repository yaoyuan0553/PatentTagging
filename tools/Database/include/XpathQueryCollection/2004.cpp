//
// Created by yuan on 8/15/19.
//

#include "2004.h"

#include <iostream>

using namespace std;

string XpathIdQuery2004::operator()(pugi::xml_node& root)
{
    pugi::xml_node idRootNode = root.select_node(idRootQuery_.pugiQuery()).node();
    string output = "US";

    for (const XpathQuery& xqs : idNodeQueryList_) {
        auto str = idRootNode.select_node(xqs.pugiQuery()).node().text();
        if (str.empty())
            continue;
        output += str.get();
    }

    return output == "US" ? "" : output;
}

std::string XpathAbstractQuery2004::operator()(pugi::xml_node& root)
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

std::string XpathClaimQuery2004::operator()(pugi::xml_node& root)
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

std::string XpathIpcQuery2004::operator()(pugi::xml_node& root)
{
    pugi::xpath_node_set ipcNodes = root.select_nodes(ipcNodesQuery_.pugiQuery());

    string output;
    /*  <classification-ipc>
     *  <classification-ipc-primary>
     *  <ipc>G04B019/20</ipc>
     *  </classification-ipc-primary>
     *  <classification-ipc-secondary>
     *  <ipc>G04B019/24</ipc>
     *  </classification-ipc-secondary>
     *  <classification-ipc-edition>07</classification-ipc-edition>
     *  </classification-ipc>
     */
    for (const pugi::xpath_node& ipc : ipcNodes) {
        pugi::xpath_node_set subNodes = ipc.node().select_nodes(subNodesQuery_.pugiQuery());

        if (subNodes.size() < 0) continue;

        for(const auto i : subNodes){
            output += string(i.node().text().get()).substr(0,4) + '-' +
                      string(i.node().text().get()).substr(4)+ ',';
        }
    }
    if (!output.empty())
        output.pop_back();

    return output;
}
