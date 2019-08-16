//
// Created by yuan on 8/16/19.
//

#pragma once
#ifndef TOOLS_HELPER_H
#define TOOLS_HELPER_H

#include <string>
#include <type_traits>
#include <iostream>
#include <tuple>

namespace Details
{
    template <typename T, typename... Ts>
    using AllType =
            std::enable_if_t<std::conjunction_v<std::is_convertible<Ts, T>...>>;

    template <typename... Ts>
    using AllString = AllType<std::string, Ts...>;

//    template <typename... Args> struct PrintStrings;
//
//    template <typename Arg, typename... Args>
//    struct PrintStrings<Arg, Args...> {
//        static void print(Arg&& arg, Args&&... args)
//        {
//            std::cout << std::forward<Arg>(arg) << ',';
//            PrintStrings<Args...>::print(std::forward<Args>(args)...);
//        }
//    };
//
//    template <typename Arg>
//    struct PrintStrings<Arg> {
//        static void print(Arg&& arg)
//        {
//            std::cout << std::forward<Arg>(arg) << '\n';
//        }
//    };
//
//    template <>
//    struct PrintStrings<> {
//        static void print()
//        {
//            std::cout << '\n';
//        }
//    };

    template <size_t N, typename... Args>
    decltype(auto) GetArg(Args&&... args) noexcept
    {
        return std::get<N>(std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template <typename Arg, typename... Args>
    struct PrintStrings {
        static void print(Arg&& arg, Args&&... args)
        {
            if constexpr (sizeof...(args) == 0) {
                std::cout <<  std::forward<Arg>(arg) << '\n';
            }
            else {
                std::cout << std::forward<Arg>(arg) << ',';
                PrintStrings<Args...>::print(std::forward<Args>(args)...);
            }
        }
    };

    template <typename... Args, typename = AllString<Args...>>
    void PrintStringRec(const Args&... args)
    {
        PrintStrings<const Args&...>::print(args...);
    }
}



//template <typename T = std::string>
//std::string ConcatenateStringsWithDelimiter()


#endif //TOOLS_HELPER_H
