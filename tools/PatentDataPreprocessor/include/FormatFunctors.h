//
// Created by yuan on 7/31/19.
//

#pragma once
#ifndef TOOLS_TAGTEXTFORMATTER_H
#define TOOLS_TAGTEXTFORMATTER_H

#include <string>
#include <vector>

#include "FunctorDict.h"
#include "Utility.h"


struct TagTextFormatterFunctor : public Cloneable {
    virtual std::string operator()(const std::string& text) = 0;
};


struct ClassificationFormatter : public TagTextFormatterFunctor {
    std::string operator()(const std::string& classification) override
    {
        return ReplaceDelimiter<'-'>(std::string(classification));
    }

    DEFINE_DEFAULT_CLONE(ClassificationFormatter);
};


class AbstractSeparateFormatter : public TagTextFormatterFunctor {
    SplitParagraph splitParagraph;
public:
    std::string operator()(const std::string& abstract) override
    {
        return splitParagraph(abstract, '\n');
    }

    DEFINE_DEFAULT_CLONE(AbstractSeparateFormatter);
};


/* A mapped functor storing customized function to format inner text of based on a given tag */
class TagTextFormatterDict : public FunctorDict<TagTextFormatterFunctor, std::string, const std::string&> {

};

class FileOutputFormatterDict : public std::unordered_map<std::string, TagTextFormatterDict> {
    std::unordered_set<std::string> tags_;

public:
    FileOutputFormatterDict() = default;

    const std::unordered_set<std::string>& getTags();
};

class FileOutputFormatter : public std::unordered_map<std::string, TagTextFormatterDict> {
    using Filename = std::string;
    std::unordered_set<std::string> tags_;
public:
    FileOutputFormatter() = default;

    const std::unordered_set<std::string>& getTags()
    {
        for (const auto& [_, formatter] : *this)
            tags_.insert(formatter.getKeys().begin(), formatter.getKeys().end());

        return tags_;
    }
};

#endif //TOOLS_TAGTEXTFORMATTER_H
