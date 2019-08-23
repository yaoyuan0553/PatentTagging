//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_2019_H
#define TOOLS_2019_H

#include <vector>

#include <XpathQueryTextFormatter.h>


/* query for publication id and application id */
class XpathIdQuery2019 : public XpathQueryTextFormatter {
    XpathQuery idRootQuery_;
    std::vector<XpathQuery> idNodeQueryList_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIdQuery2019);

    XpathIdQuery2019(const XpathQueryString& rootNodeQueryStr,
            const std::vector<XpathQueryString>& idNodeQueryStrList) :
            idRootQuery_(rootNodeQueryStr)
    {
        idNodeQueryList_.reserve(idNodeQueryStrList.size());
        for (const XpathQueryString& xqs : idNodeQueryStrList)
            idNodeQueryList_.emplace_back(xqs);
    }
};

/* query for publication date and application date */
using XpathDateQuery2019 = XpathIdQuery2019;

/* query for publication id and application id */
class XpathIpcQuery2019 : public XpathQueryTextFormatter {
    XpathQuery ipcNodesQuery_;
    XpathQuery subNodesQuery_;
    inline static constexpr char queryStr[] =
        ".//section | .//class | .//subclass | .//main-group | "
        ".//subgroup";
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathIpcQuery2019);

    explicit XpathIpcQuery2019(const XpathQueryString& rootNodeQueryStr) :
            ipcNodesQuery_(rootNodeQueryStr),
            subNodesQuery_(queryStr) { }
};


/* Title  is just a simple default query */
using XpathTitleQuery = XpathSingleQueryDefaultInnerText;

class XpathAbstractQuery2019 : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery paragraphQuery_;
    XpathQuery abstractQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;

    DEFINE_DEFAULT_CLONE(XpathAbstractQuery2019);

    explicit XpathAbstractQuery2019(const XpathQueryString& rootNodeQueryStr) :
            paragraphQuery_(".//p"), abstractQuery_(rootNodeQueryStr) { }
};


using XpathDescriptionQuery2019 = XpathAbstractQuery2019;


class XpathClaimQuery2019 : public XpathQueryTextFormatter {
    ExhaustiveChildWalker walker_;
    XpathQuery claimQuery_;
public:
    std::string operator()(pugi::xml_node& root) override;
    DEFINE_DEFAULT_CLONE(XpathClaimQuery2019);

    explicit XpathClaimQuery2019(const XpathQueryString& claimNodeQueryStr) :
            claimQuery_(claimNodeQueryStr) { }
};


#endif //TOOLS_2019_H
