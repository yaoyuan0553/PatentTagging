//
// Created by yuan on 8/13/19.
//

#include "NodeFilters.h"
#include "TagConstants.h"


using namespace std;

bool IdNodeWalker::for_each(pugi::xml_node& node)
{
    if (node.type() == pugi::node_element) {
        if (strcmp(node.name(), tags::date) == 0)
            innerText_ += '-' + string(node.text().get());
        else
            innerText_ += node.text().get();
    }

    return true;
}

/*
std::string ClaimNodeFilter::operator()(pugi::xml_node& node)
{
    node.traverse(walker_);

    std::string content = walker_.getInnerText();

    walker_.reset();

    return content;
}

*/
