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
#include <regex>


/* replace multiple occurrences of the oldDelimiter (2nd template argument)
 * with a single newDelimiter (1st template argument)
 * the default oldDelimiter is a space character ' ' */
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


inline std::string RemoveExtraWhitespace(const std::string& str)
{
    static const std::regex e("\\s+");

    return std::regex_replace(str, e, " ");
}



/* A functor class to split a paragraph into sentences delimited with a given delimiter
 * may throw range_error when encounters unrecognized characters
 * made as a function emulating class (aka. functor) to save initialization time of
 * helper variables, i.e. separators_ and converter_ */
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


class TruncateUnicodeString {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter_;
public:
    TruncateUnicodeString() = default;

    TruncateUnicodeString(const TruncateUnicodeString&) { }

    std::string operator()(const std::string& str, int length)
    {
        std::u32string u32str = converter_.from_bytes(str);

        return converter_.to_bytes(u32str.substr(0, length));
    }

    std::string truncate(const std::string& str, int length)
    {
        return operator()(str, length);
    }
};


/* Cloneable Macros to shorten repetitive code */

/* Define a default clone member override function */
#define DEFINE_DEFAULT_CLONE(cls) \
    cls* clone() const override { return new cls(*this); }

/* Declare a pure virtual clone member function */
#define DECLARE_ABSTRACT_CLONE(cls) \
    cls* clone() const override = 0


/* Interface - subclasses of this class must implement clone() function */
struct Cloneable {
    virtual Cloneable* clone() const = 0;
};


/* name alias for classes */

/* For storing Tag as key and vector of Texts as value */
using TagTextDict = std::unordered_map<std::string, std::vector<std::string>>;


#endif //TOOLS_UTILITY_H
