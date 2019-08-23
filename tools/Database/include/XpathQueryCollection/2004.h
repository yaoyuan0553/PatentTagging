//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_2004_H
#define TOOLS_2004_H

#include <vector>

#include <XpathQueryTextFormatter.h>


/* query for publication id and application id */
class XpathIdQuery2004: public XpathQueryTextFormatter {
    XpathQuery idRootQuery_;
    std::vector<XpathQuery> idNodeQueryList_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIdQuery2004);

    XpathIdQuery2004(const XpathQueryString& rootNodeQueryStr,
            const std::vector<XpathQueryString>& idNodeQueryStrList) :
            idRootQuery_(rootNodeQueryStr)
    {
        idNodeQueryList_.reserve(idNodeQueryStrList.size());
        for (const XpathQueryString& xqs : idNodeQueryStrList)
            idNodeQueryList_.emplace_back(xqs);
    }
};

/* query for publication date and application date */
using XpathDateQuery2004 = XpathIdQuery2004;

/* query for publication id and application id */
class XpathIpcQuery2004: public XpathQueryTextFormatter {
    XpathQuery ipcNodesQuery_;
    XpathQuery subNodesQuery_;
    inline static constexpr char queryStr[] =
        ".//ipc";
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIpcQuery2004);

    explicit XpathIpcQuery2004(const XpathQueryString& rootNodeQueryStr) :
            ipcNodesQuery_(rootNodeQueryStr),
            subNodesQuery_(queryStr) { }
};


/* Title  is just a simple default query */
using XpathTitleQuery2004 = XpathSingleQueryDefaultInnerText;

class XpathAbstractQuery2004: public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery paragraphQuery_;
    XpathQuery abstractQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathAbstractQuery2004);

    explicit XpathAbstractQuery2004(const XpathQueryString& rootNodeQueryStr) :
            paragraphQuery_(".//heading | .//paragraph"), abstractQuery_(rootNodeQueryStr) { }
};


using XpathDescriptionQuery2004 = XpathAbstractQuery2004;


class XpathClaimQuery2004: public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery claimQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;
    DEFINE_DEFAULT_CLONE(XpathClaimQuery2004);

    explicit XpathClaimQuery2004(const XpathQueryString& claimNodeQueryStr) :
            claimQuery_(claimNodeQueryStr) { }
};


#endif //TOOLS_2004_H
