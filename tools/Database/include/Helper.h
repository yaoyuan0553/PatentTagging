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


/* hides implementation of helper templates */
namespace Details
{
    template <typename T, typename... Ts>
    using AllType =
            std::enable_if_t<std::conjunction_v<std::is_convertible<Ts, T>...>>;

    template <typename... Ts>
    using AllString = AllType<std::string, Ts...>;


    template <typename... Args> struct PrintStringsRecursive;

    template <typename Arg, typename... Args>
    struct PrintStringsRecursive<Arg, Args...> {
        static void print(Arg&& arg, Args&&... args)
        {
            std::cout << std::forward<Arg>(arg) << ',';
            PrintStringsRecursive<Args...>::print(std::forward<Args>(args)...);
        }
    };

    template <typename Arg>
    struct PrintStringsRecursive<Arg> {
        static void print(Arg&& arg)
        {
            std::cout << std::forward<Arg>(arg) << '\n';
        }
    };

    template <>
    struct PrintStringsRecursive<> {
        static void print()
        {
            std::cout << '\n';
        }
    };

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

    template <typename...> struct ConcatStringWithDelimiterImpl;

    template <typename Arg1, typename Arg2, typename... Args>
    struct ConcatStringWithDelimiterImpl<Arg1, Arg2, Args...> {
        static std::string concat(const char* delimiter,
                Arg1&& arg1, Arg2&& arg2, Args&&... args)
        {
            if constexpr (sizeof...(args) == 0)
                return std::string(std::forward<Arg1>(arg1)) + delimiter +
                        std::forward<Arg2>(arg2);
            else
                return std::string(std::forward<Arg1>(arg1)) + delimiter +
                       std::forward<Arg2>(arg2) + delimiter +
                       ConcatStringWithDelimiterImpl<Args...>::concat(delimiter,
                               std::forward<Args>(args)...);
        }
    };

    template <typename Arg>
    struct ConcatStringWithDelimiterImpl<Arg> {
        static std::string concat(const char*, Arg&& arg)
        {
            return std::forward<Arg>(arg);
        }
    };
}

/********************************/
/*    abstract interface types  */
/********************************/

struct Stringifiable {
    virtual std::string stringify() const = 0;
    virtual ~Stringifiable() = default;
};

struct FileReadable {
    virtual void readFromFile(const char* filename) = 0;
    virtual ~FileReadable() = default;
};

struct FileWritable {
    virtual void writeToFile(const char* filename) = 0;
    virtual ~FileWritable() = default;
};

struct FileReadWritable : public FileReadable, public FileWritable {
    ~FileReadWritable() override = default;
};

/**
 * @brief
 * @details  concatenate given strings delimiters
 *      function takes at lease 3 arguments,
 *      with the first argument being the delimiter,
 *      and the rest being strings to be concatenated
 * @tparam Args
 * @param delimiter
 * @param str1
 * @param str2
 * @param otherStrs
 * @return
 */
template <typename... Args, typename = Details::AllString<Args...>>
std::string ConcatStringWithDelimiter(const char* delimiter,
        const std::string& str1, const std::string& str2, const Args&... otherStrs)
{
    return Details::ConcatStringWithDelimiterImpl<
            const std::string&, const std::string&, const Args&...>::
            concat(delimiter, str1, str2, otherStrs...);
}



#endif //TOOLS_HELPER_H
