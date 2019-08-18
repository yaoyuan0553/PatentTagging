//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XPATHQUERYTEXTFORMATTER_H
#define TOOLS_XPATHQUERYTEXTFORMATTER_H

#include <pugixml.hpp>

#include "FunctorDict.h"
#include "XpathQuery.h"
#include "TagNodeFilterFunctors.h"


struct XpathQueryTextFormatter : public Cloneable {
    virtual std::string operator()(pugi::xml_node& root) = 0;

    DECLARE_ABSTRACT_CLONE(XpathQueryTextFormatter);

    virtual ~XpathQueryTextFormatter() = default;
};

using XpathQueryTextFormatterDict = FunctorDict<XpathQueryTextFormatter,
        std::string, pugi::xml_node&>;


/* returns inner text of only the designated node */
class XpathSingleQueryDefaultInnerText : public XpathQueryTextFormatter {
protected:
    XpathQuery query_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathSingleQueryDefaultInnerText);

    explicit XpathSingleQueryDefaultInnerText(const XpathQueryString& queryStr) :
        query_(queryStr) { }
};


/* returns text of designated node and that of its children */
class XpathSingleQueryGreedyInnerText : public XpathQueryTextFormatter {
protected:
    ExhaustiveChildWalker walker_;
    XpathQuery query_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathSingleQueryGreedyInnerText);

    explicit XpathSingleQueryGreedyInnerText(const XpathQueryString& queryStr) :
            query_(queryStr) { }
};

/* returns all text without extra space */
class XpathSingleQueryGreedyNoExtraSpaceInnerText : public XpathSingleQueryGreedyInnerText {
    using Base = XpathSingleQueryGreedyInnerText;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathSingleQueryGreedyNoExtraSpaceInnerText);

    explicit XpathSingleQueryGreedyNoExtraSpaceInnerText(const XpathQueryString& queryStr) :
            XpathSingleQueryGreedyInnerText(queryStr) { }
};


#endif //TOOLS_XPATHQUERYTEXTFORMATTER_H
