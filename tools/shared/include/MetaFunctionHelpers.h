//
// Created by yuan on 7/24/19.
//

#pragma once
#ifndef TOOLS_METAFUNCTIONHELPERS_H
#define TOOLS_METAFUNCTIONHELPERS_H

#endif //TOOLS_METAFUNCTIONHELPERS_H

#include <type_traits>

template <typename T>
struct optional_ref_wrapper {
    static constexpr bool value = false;
    using type = T;
    type operator()(T val) {
        return val;
    }

};

template <typename T>
struct optional_ref_wrapper<T&> {
    static constexpr bool value = true;
    using type = std::reference_wrapper<T>;
    type operator()(T& val) {
        return std::ref(val);
    }
};

template <typename T>
struct optional_ref_wrapper<T&&> {
    static constexpr bool value = true;
    using type = T&&;
    type operator()(T&& val) {
        return std::forward<T>(val);
    }
};

template <typename T>
using optional_ref_wrapper_t = typename optional_ref_wrapper<T>::type;

template <typename T>
constexpr bool optional_ref_wrapper_v = optional_ref_wrapper<T>::value;


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
