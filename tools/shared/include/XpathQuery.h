//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XPATHQUERY_H
#define TOOLS_XPATHQUERY_H


#include <string>
#include <pugixml.hpp>

#include "XpathQueryString.h"


/* wrapper for pugi::XpathQuery
 * for copying and cloning */
class XpathQuery {
    XpathQueryString queryStr_;
    pugi::xpath_query* query_;
public:
    explicit XpathQuery(const XpathQueryString& queryStr) : queryStr_(queryStr),
        query_(new pugi::xpath_query(queryStr.c_str())) { }

    XpathQuery(const XpathQuery& other) : queryStr_(other.queryStr_),
        query_(new pugi::xpath_query(other.queryStr_.c_str())) { }

    XpathQuery& operator=(const XpathQuery& other)
    {
        delete query_;
        queryStr_ = other.queryStr_;
        query_ = new pugi::xpath_query(queryStr_.c_str());

        return *this;
    }

    ~XpathQuery()
    {
        delete query_;
    }

    /* retrieve pugi::xpath_query */
    pugi::xpath_query& pugiQuery() { return *query_; }
    const pugi::xpath_query& pugiQuery() const { return *query_; }
};


#endif //TOOLS_XPATHQUERY_H
