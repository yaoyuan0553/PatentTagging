//
// Created by yuan on 7/19/19.
//
#include <iostream>
#include <string>
#include <utility>
#include <thread>

#include "ThreadJob.h"
#include "ThreadPool.h"

using namespace std;


class SubA : public ThreadJob<string&, int> {
    void internalRun(string& str, int i) override
    {
        cout << "SubA: " << str << " " << i << endl;
    }
public:
    SubA(string& str, int i) : ThreadJob(forward<string&>(str), forward<int>(i)) { }
};

class SubB : public ThreadJob<double> {
    void internalRun(double x) override
    {
        cout << "SubB: " << x << endl;
    }
public:
    explicit SubB(double x) : ThreadJob(forward<double>(x)) { }
};

class SubC : public ThreadJob<string&> {
    void internalRun(string& str) override
    {
        cout << "SubC: " << str << endl;
    }
public:
    explicit SubC(string& str) : ThreadJob(str) { }
};

//class SubD : public ThreadJob<string&&> {
//    void internalRun(string&& str) override
//    {
//        cout << "SubD: " << str << endl;
//    }
//public:
//    explicit SubD(string&& str) : ThreadJob(std::forward<string>(str)) { }
//};

class SubE : public ThreadJob<> {
    void internalRun() override
    {
        cout << "SubE\n";
    }
public:
    SubE() = default;
};


template <typename... T>
void wrapper(ThreadJob<T...>& base, T&&... args)
{
    base.run(forward<T>(args)...);
}

void test()
{
    string hehe = "hehe";
    SubA a(hehe, 3);
    SubB b(3.14);
//    a.run("blah", 3);
//    b.run(3.14);
    string haha = "haha";
    SubC c(haha);

//    SubD d("xxx");
    SubE e;
//    c.run(haha);
    a.run();
    b.run();
    c.run();
//    d.run();
    e.run();

    a.wait();
    b.wait();
    c.wait();
//    d.wait();
    e.wait();
}

// in the normal case, just the identity
template<class T>
struct item_return {
    using type = T;
};

template <class T>
struct item_return<T&> {
    using type = reference_wrapper<T>;
};

template<class T>
typename item_return<T>::type foo() { return T(); }

template<>
struct item_return<float> { using type = int; };

template<>
int foo<float>() { return 42; }

template<class T, typename = void_t<>>
typename item_return<T>::type bar(T val) { return val; }

template<class T, typename = void_t<>>
typename item_return<T&>::type bar(T& val) { return ref(val); }



template<typename T>
struct reference_detector {
    static constexpr T value = T();
    reference_detector()
    {
        cout << "not-reference\n";
    }
};

template<typename T>
struct reference_detector<T&> {
    static const reference_wrapper<T> value = ref(T());
    reference_detector()
    {
        cout << "l-value reference\n";
    }
};

template<typename T>
struct reference_detector<T&&> {
    static constexpr T&& value = T();
    reference_detector()
    {
        cout << "r-value reference\n";
    }
};


class Foo {};




int main()
{
//    int x = 5;
//
//    cout << boolalpha;
//    cout << optional_ref_wrapper<int>::value << '\n';
//    cout << optional_ref_wrapper<int&>::value << '\n';
//
//    auto o1 = optional_ref_wrapper<int>()(x);
//    cout << o1 << endl;
//    cout << x << endl;
//
//    auto o2 = optional_ref_wrapper<int&>()(x);
//    cout << o2 << endl;
//    cout << x << endl;
//
//    auto o3 = optional_ref_wrapper<decltype(x)>()(x);
//    cout << o3 << endl;
//    cout << boolalpha;
//    cout << is_base_of_v<ThreadJob<>, SubA> << endl;
//    cout << is_base_of_template_v<ThreadJob, SubC> << endl;

    test();

    return 0;
}
