//
// Created by yuan on 8/13/19.
//

#include "OutputFormatters.h"

#include "TagConstants.h"

using namespace std;

/*
string ClaimDataFormatter::operator()(const TextVec& texts)
{
    string output;
    for (const string& claim : texts)
        output += splitParagraph_(RemoveExtraWhitespace(claim), '\n');

    return output;
}
*/

string ClaimDataFormatter::operator()(const TagTextDict& tagTextDict)
{
    string output;
    for (const string& claim : tagTextDict.at(tags::claim))
        output += RemoveExtraWhitespace(claim) + '\n';
    output.pop_back();

    return output;
}

std::string AbstractDataFormatter::operator()(const TagTextDict& tagTextDict)
{
    string output;
    for (const string& abstract : tagTextDict.at(tags::abstract))
        output += RemoveExtraWhitespace(abstract);

    return output;
}
