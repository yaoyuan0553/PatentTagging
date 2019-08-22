//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XPATHQUERYCOLLECTION_H
#define TOOLS_XPATHQUERYCOLLECTION_H

#include <vector>

#include <XpathQueryTextFormatter.h>


/* query for publication id and application id */
class XpathIdQuery : public XpathQueryTextFormatter {
    XpathQuery idRootQuery_;
    std::vector<XpathQuery> idNodeQueryList_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIdQuery);

    XpathIdQuery(const XpathQueryString& rootNodeQueryStr,
            const std::vector<XpathQueryString>& idNodeQueryStrList) :
            idRootQuery_(rootNodeQueryStr)
    {
        idNodeQueryList_.reserve(idNodeQueryStrList.size());
        for (const XpathQueryString& xqs : idNodeQueryStrList)
            idNodeQueryList_.emplace_back(xqs);
    }
};

/* query for publication date and application date */
using XpathDateQuery = XpathIdQuery;

/* query for publication id and application id */
class XpathIpcQuery : public XpathQueryTextFormatter {
    XpathQuery ipcNodesQuery_;
    XpathQuery subNodesQuery_;
    inline static constexpr char queryStr[] =
        ".//section | .//class | .//subclass | .//main-group | "
        ".//subgroup";
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIpcQuery);

    explicit XpathIpcQuery(const XpathQueryString& rootNodeQueryStr) :
            ipcNodesQuery_(rootNodeQueryStr),
            subNodesQuery_(queryStr) { }
};


/* Title  is just a simple default query */
using XpathTitleQuery = XpathSingleQueryDefaultInnerText;

class XpathAbstractQuery : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery paragraphQuery_;
    XpathQuery abstractQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathAbstractQuery);

    explicit XpathAbstractQuery(const XpathQueryString& rootNodeQueryStr) :
            paragraphQuery_(".//p"), abstractQuery_(rootNodeQueryStr) { }
};


using XpathDescriptionQuery = XpathAbstractQuery;


class XpathClaimQuery : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery claimQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;
    DEFINE_DEFAULT_CLONE(XpathClaimQuery);

    explicit XpathClaimQuery(const XpathQueryString& claimNodeQueryStr) :
            claimQuery_(claimNodeQueryStr) { }
};


#endif //TOOLS_XPATHQUERYCOLLECTION_H
