//
// Created by yuan on 7/24/19.
//

#include <iostream>
#include <string>
#include <utility>

#include "ThreadJob.h"
#include "ThreadPool.h"

using namespace std;

class SubA : public ThreadJob<string&, int> {
    void internalRun(string& str, int i) override
    {
        cout << "SubA: " << str << " " << i << endl;
        str = "okay";
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

/*
class SubD : public ThreadJob<string&&> {
    void internalRun(string&& str) override
    {
        cout << "SubD: " << str << endl;
    }
public:
    explicit SubD(string&& str) : ThreadJob(std::forward<string>(str)) { }
};
*/

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
    string haha = "haha";
    SubC c(haha);
    SubE e;

    a.runOnMain();
    b.runOnMain();
    c.runOnMain();
    e.runOnMain();

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

void test2()
{
    int n = 10;
    double x = 3.14;
//    vector<ThreadInterface*> ts;
//    ts.push_back(new SubB(i));
    ThreadPool threadPool;
    for (int i = 0; i < n; i++)
        threadPool.add(new SubB(x++));

    threadPool.runAll();
    threadPool.waitAll();

    PCThreadPool<> pcThreadPool;
    int nP = 15, nC = 10;
    for (int i = 0; i < nP; i++)
        pcThreadPool.addProducer(new SubB(i++));
    for (int i = 0; i < nC; i++)
        pcThreadPool.addConsumer(new SubE);

    pcThreadPool.runAll();
    pcThreadPool.waitAll();
}
