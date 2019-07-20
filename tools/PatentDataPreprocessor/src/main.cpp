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

class SubC : public ThreadJob<const string&> {
    void internalRun(const string& str) override
    {
        cout << "SubC: " << str << endl;
    }
};


template <typename... T>
void test(ThreadJob<T...>& base, T&&... args)
{
    base.run(forward<T>(args)...);
}


int main()
{
    SubA a;
    SubB b;
    SubC c;
    a.run("blah", 3);
    b.run(3.14);
    string haha = "haha";
    c.run(haha);

    a.wait();
    b.wait();
    c.wait();

    return 0;
}
