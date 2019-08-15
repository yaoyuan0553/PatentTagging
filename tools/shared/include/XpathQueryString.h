//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_XPATHQUERYSTRING_H
#define TOOLS_XPATHQUERYSTRING_H

#include <string>

/* defines Union and UnionWith for
 * easier changing XmlQuery strings */
class XpathQueryString : public std::string {
public:
    XpathQueryString(const std::string& str) : std::string(str) { }

    static XpathQueryString Union(const XpathQueryString& xqs1,
            const XpathQueryString& xqs2)
    {
        return xqs1 + " | " + xqs2;
    }

    void UnionWith(const XpathQueryString& other)
    {
        *this += " | " + other;
    }
};

#endif //TOOLS_XPATHQUERYSTRING_H
