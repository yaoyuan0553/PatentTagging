//
// Created by yuan on 7/19/19.
//
#include <iostream>
#include <string>
#include <utility>
#include <thread>

#include "ThreadJob.h"

using namespace std;


class SubA : public ThreadJob<string, int> {
    void internalRun(string str, int i) override
    {
        cout << "SubA: " << str << " " << i << endl;
    }
};

class SubB : public ThreadJob<double> {
    void internalRun(double x) override
    {
        cout << "SubB: " << x << endl;
    }
};

class SubC : public ThreadJob<string&> {
    void internalRun(string& str) override
    {
        cout << "SubC: " << str << endl;
    }
};


template <typename... T>
void wrapper(ThreadJob<T...>& base, T&&... args)
{
    base.run(forward<T>(args)...);
}

void test()
{
    SubA a;
    SubB b;
//    SubC c;
    a.run("blah", 3);
    b.run(3.14);
    string haha = "haha";
//    c.run(haha);

    a.wait();
    b.wait();
//    c.wait();

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


template <typename T>
void reference_detector(T) { cout << "base: " << is_reference_v<T> << endl; }

template <typename T>
enable_if_t<is_reference_v<T>> reference_detector(T&) { cout << "reference!\n"; }




int main()
{
    reference_detector(3);
    int x = 2;
    reference_detector(x);
    int& y = x;
    reference_detector(y);


    return 0;
}
