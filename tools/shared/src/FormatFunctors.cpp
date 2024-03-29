//
// Created by yuan on 7/31/19.
//


#include "FormatFunctors.h"

#include <iostream>

#include "Utility.h"
#include "TagConstants.h"


std::string IdClassAbstractFileOutput::operator()(const TagTextDict& tagTextDict)
{
    std::string output = tagTextDict.at(tags::filename)[0] + '\t';
    for (const std::string& c : tagTextDict.at(tags::classification))
        output += ReplaceDelimiter<'-'>(c) + ',';

    output.back() = '\t';

    for (const std::string& a : tagTextDict.at(tags::abstract))
        output += truncateUnicodeString_(RemoveExtraWhitespace(a), 510);

    output += '\n';

    return output;
}

std::string SplitAbstractFileOutput::operator()(const TagTextDict& tagTextDict)
{
    std::string output;
    for (const std::string& c : tagTextDict.at(tags::abstract)) {
        output += splitParagraph_(RemoveExtraWhitespace(c), '\n');
    }
    if (!output.empty())
        output += "\n\n";

    return output;
}

std::string FirstClassFirstSpaceOutput::operator()(const TagTextDict& tagTextDict)
{
    const auto& classifications = tagTextDict.at(tags::classification);
    if (!classifications.empty()) {
        std::size_t space = classifications[0].find_first_of(' ');
        if (space != std::string::npos)
            return classifications[0].substr(0, space);
    }

    return "";
}

std::string IdSmallClassAbstractFileOutput::operator()(const TagTextDict& tagTextDict)
{
    std::string output = tagTextDict.at(tags::filename)[0] + '\t';
    const auto& classifications = tagTextDict.at(tags::classification);
    if (classifications.empty())
        return "";
    std::size_t space = classifications[0].find_first_of(' ');
    if (space == std::string::npos)
        return "";
    output += classifications[0].substr(0, space);

    output += '\t';

    for (const std::string& a : tagTextDict.at(tags::abstract))
        output += truncateUnicodeString_(RemoveExtraWhitespace(a), 510);

    output += '\n';

    return output;
}

std::string FirstClassOutput::operator()(const TagTextDict& tagTextDict)
{
    const auto& classifications = tagTextDict.at(tags::classification);
    if (!classifications.empty()) {
        return ReplaceDelimiter<'-'>(classifications[0]);
    }

    return "";
}

