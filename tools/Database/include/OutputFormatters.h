//
// Created by yuan on 8/13/19.
//

#pragma once
#ifndef TOOLS_OUTPUTFORMATTERS_H
#define TOOLS_OUTPUTFORMATTERS_H

#include <vector>
#include <string>

#include <Utility.h>
#include <FunctorDict.h>
#include <FormatFunctors.h>

#include "DataTypes.h"


constexpr char APP_DATE[] = "";

using TextVec = std::vector<std::string>;

struct DatabaseOutputFormatter : public Cloneable {
    virtual std::string operator()(const TextVec& texts) = 0;

    DECLARE_ABSTRACT_CLONE(DatabaseOutputFormatter);

    virtual ~DatabaseOutputFormatter() = default;
};


/*
class ClaimDataFormatter : public DatabaseOutputFormatter {
    SplitParagraph splitParagraph_;
public:

    virtual std::string operator()(const TextVec& texts) final;

    DEFINE_DEFAULT_CLONE(ClaimDataFormatter);
};
*/

class ClaimDataFormatter : public TagTextOutputFormatter {
public:

    std::string operator()(const TagTextDict& tagTextDict) final;

    DEFINE_DEFAULT_CLONE(ClaimDataFormatter);
};


class AbstractDataFormatter : public TagTextOutputFormatter {
public:
    std::string operator()(const TagTextDict& tagTextDict) final;

    DEFINE_DEFAULT_CLONE(AbstractDataFormatter);
};


using DatabaseOutputFormatterDict = FunctorDict<
        DatabaseOutputFormatter, std::string, const TextVec&>;



#endif //TOOLS_OUTPUTFORMATTERS_H
