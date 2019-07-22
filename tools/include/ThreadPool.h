//
// Created by yuan on 7/22/19.
//

#pragma once
#ifndef TOOLS_THREADPOOL_H
#define TOOLS_THREADPOOL_H

#include <vector>
#include <array>
#include <type_traits>

#include "ThreadJob.h"


template <template <typename...> class Base, typename Derived>
struct is_base_of_template_impl {
private:
    template<typename... Ts>
    static constexpr std::true_type test(const Base<Ts...> *);
    static constexpr std::false_type test(...);
public:
    using type = decltype(test(std::declval<Derived*>()));
};

template <template <typename...> class Base, typename Derived>
using is_base_of_template = typename is_base_of_template_impl<Base, Derived>::type;

template <template <typename...> class Base, typename Derived>
constexpr bool is_base_of_template_v = is_base_of_template<Base, Derived>::value;


template <typename... ThreadJobSubClass>
class ThreadPool {
    static_assert(std::conjunction_v<is_base_of_template<ThreadJob, ThreadJobSubClass>...>,
            "Argument(s) of ThreadPool must be subclass(es) of ThreadJob!");
protected:
    ThreadPool() = default;

public:
    void waitAll()
    {
    }
};



#endif //TOOLS_THREADPOOL_H
