//
// Created by yuan on 7/25/19.
//

#pragma once
#ifndef TOOLS_UTILITY_H
#define TOOLS_UTILITY_H

#include <string>
#include <sstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <algorithm>


template <char newDelimiter, char oldDelimiter = ' '>
std::string ReplaceDelimiter(const std::string& str)
{
    using namespace std;

    string newStr;
    newStr.reserve(str.length());
    stringstream ss(str);
    for (string word; getline(ss, word, oldDelimiter);) {
        if (word.empty()) continue;
        newStr += word + newDelimiter;
    }

    newStr.pop_back();  // remove the extra delimiter

    return newStr;
}


template <char = 0, char = 0>
std::string ReplaceDelimiter(const std::string& str, char newDelimiter, char oldDelimiter)
{
    using namespace std;

    string newStr;
    newStr.reserve(str.length());
    stringstream ss(str);
    for (string word; getline(ss, word, oldDelimiter);)
        newStr += word + newDelimiter;

    newStr.pop_back();  // remove the extra delimiter

    return newStr;
}


/* A functor class to split a paragraph into sentences delimited with a given delimiter
 * may throw range_error when encounters unrecognized characters */
class SplitParagraph {
    std::unordered_set<char32_t> separators_;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter_;

    inline bool isSeparator(char32_t c) const
    {
        return separators_.find(c) != separators_.end();
    }

    static inline bool isNotLast(size_t i, size_t n)
    {
        return i < n;
    }

public:

    inline static const std::initializer_list<char32_t> defaultSeparators =
            { u'。', u'？', u'！', u'：', u'；' };

    SplitParagraph(const std::initializer_list<char32_t>& separators = defaultSeparators) :
        separators_(separators) { }

    SplitParagraph(const SplitParagraph& other) : separators_(other.separators_) { }

    SplitParagraph& operator=(const SplitParagraph& other)
    {
        separators_ = other.separators_;
        return *this;
    }

    /* will throw std::range_error if any character is not utf-32 convertible */
    std::string operator()(const std::string& str, char32_t delimiter)
    {
        std::u32string u32paragraph = converter_.from_bytes(str);
        std::string sents;
        auto deli = converter_.to_bytes(delimiter);
        size_t i = 0;
        for (char32_t c : u32paragraph) {
            sents += converter_.to_bytes(c);
            i++;
            if (isSeparator(c) && isNotLast(i, u32paragraph.length()))
                sents += deli;
        }
        return sents;
    }

    /* will throw std::range_error if any character is not utf-32 convertible */
    std::string split(const std::string& str, char32_t delimiter)
    {
        return operator()(str, delimiter);
    }
};


#define DEFINE_DEFAULT_CLONE(cls) \
    cls* clone() const override { return new cls(*this); }

#define DECLARE_ABSTRACT_CLONE(cls) \
    cls* clone() const override = 0


struct Cloneable {
    virtual Cloneable* clone() const = 0;
};


/* name alias for classes */
using TagTextDict = std::unordered_map<std::string, std::vector<std::string>>;


#endif //TOOLS_UTILITY_H
