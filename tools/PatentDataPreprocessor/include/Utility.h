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
#include <functional>


/* type alias - one string per file output */
using FileOutput = std::vector<std::string>;


template <char newDelimiter, char oldDelimiter = ' '>
std::string ReplaceDelimiter(std::string&& str)
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
std::string ReplaceDelimiter(std::string&& str, char newDelimiter, char oldDelimiter)
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


class ParagraphSplit {
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
    ParagraphSplit(const std::initializer_list<char32_t>& separators) : separators_(separators) { }

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


template <typename FuncSignature>
class FunctionDict {
protected:
    std::unordered_map<std::string, std::function<FuncSignature>> funcDict_;
public:
    FunctionDict() = default;

    auto& operator[](const std::string& str)
    {
        return funcDict_[str];
    }

    virtual bool add(const std::string& name, std::function<FuncSignature> function)
    {
        return funcDict_.insert({name, function});
    }
};


/* A functor storing customized function to format inner text of a tag */
class TagTextFormat : public FunctionDict<std::string(const std::string&)> {
    std::vector<std::string> tags_;
public:
    const std::vector<std::string>& getTags()
    {
        if (tags_.size() != funcDict_.size())
        {
            tags_.reserve(funcDict_.size());
            tags_.clear();
            for (const auto& tf : funcDict_)
                tags_.push_back(tf.first);
        }

        return tags_;
    }
};


#endif //TOOLS_UTILITY_H
