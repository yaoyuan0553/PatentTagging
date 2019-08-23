//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_2005_H
#define TOOLS_2005_H

#include <vector>

#include <XpathQueryTextFormatter.h>


/* query for publication id and application id */
class XpathIdQuery2005 : public XpathQueryTextFormatter {
    XpathQuery idRootQuery_;
    std::vector<XpathQuery> idNodeQueryList_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIdQuery2005);

    XpathIdQuery2005(const XpathQueryString& rootNodeQueryStr,
            const std::vector<XpathQueryString>& idNodeQueryStrList) :
            idRootQuery_(rootNodeQueryStr)
    {
        idNodeQueryList_.reserve(idNodeQueryStrList.size());
        for (const XpathQueryString& xqs : idNodeQueryStrList)
            idNodeQueryList_.emplace_back(xqs);
    }
};

/* query for publication date and application date */
using XpathDateQuery2005 = XpathIdQuery2005;

/* query for publication id and application id */
class XpathIpcQuery2005 : public XpathQueryTextFormatter {
    XpathQuery ipcNodesQuery_;
    XpathQuery subNodesQuery_;
    inline static constexpr char queryStr[] =
        ".//main-classification | .//further-classification";
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIpcQuery2005);

    explicit XpathIpcQuery2005(const XpathQueryString& rootNodeQueryStr) :
            ipcNodesQuery_(rootNodeQueryStr),
            subNodesQuery_(queryStr) { }
};


/* Title  is just a simple default query */
using XpathTitleQuery2005 = XpathSingleQueryDefaultInnerText;

class XpathAbstractQuery2005 : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery paragraphQuery_;
    XpathQuery abstractQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathAbstractQuery2005);

    explicit XpathAbstractQuery2005(const XpathQueryString& rootNodeQueryStr) :
            paragraphQuery_(".//p"), abstractQuery_(rootNodeQueryStr) { }
};


using XpathDescriptionQuery2005 = XpathAbstractQuery2005;


class XpathClaimQuery2005 : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery claimQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;
    DEFINE_DEFAULT_CLONE(XpathClaimQuery2005);

    explicit XpathClaimQuery2005(const XpathQueryString& claimNodeQueryStr) :
            claimQuery_(claimNodeQueryStr) { }
};


#endif //TOOLS_2005_H
