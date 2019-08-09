//
// Created by yuan on 7/31/19.
//

#pragma once
#ifndef TOOLS_FUNCTORDICT_H
#define TOOLS_FUNCTORDICT_H

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "Utility.h"


template <typename Functor, typename Ret, typename... Args>
class FunctorDict {
    static_assert(std::is_class_v<Functor>, "Functor must be a class, not a function");
    static_assert(std::is_base_of_v<Cloneable, Functor>, "Functor must be Cloneable");
    static_assert(std::is_invocable_r_v<Ret, Functor, Args...>,
    "Functor must be of callable of type string(*)(const string&)");

    std::map<std::string, Functor*> functorDict_;
    std::set<std::string> keys_;

public:
    auto& operator[](const std::string& str) { return *functorDict_[str]; }

    template <typename FunctorSub, typename... ConstructArgs>
    bool add(const std::string& name, ConstructArgs&&... args)
    {
        if (!keys_.insert(name).second)
            return false;
        functorDict_[name] = new FunctorSub(std::forward<ConstructArgs>(args)...);
        return true;
    }

    const std::set<std::string>& getKeys() const { return keys_; }

    FunctorDict() = default;

    /* copy constructor for deep copy of Functor */
    FunctorDict(const FunctorDict& other) : keys_(other.keys_)
    {
        for (const auto& [name, functor] : other.functorDict_)
            functorDict_[name] = functor->clone();
    }

    FunctorDict& operator=(const FunctorDict& other)
    {
        keys_ = other.keys_;
        for (const auto& [name, functor] : other.functorDict_)
            functorDict_[name] = functor->clone();
        return *this;
    }

    virtual ~FunctorDict()
    {
        for (auto& [_, functor] : functorDict_)
            delete functor;
    }
};


#endif //TOOLS_FUNCTORDICT_H
