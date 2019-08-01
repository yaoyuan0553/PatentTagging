//
// Created by yuan on 7/31/19.
//


#include "TagTextFormatter.h"

#include "Utility.h"


const std::unordered_set<std::string>& FileOutputFormatterDict::getTags()
{
    tags_.clear();
    for (const auto&[_, formatter] : *this)
        tags_.insert(formatter.getKeys().begin(), formatter.getKeys().end());

    return tags_;
}

std::string AbstractFormatter::operator()(const std::string &text)
{

}
