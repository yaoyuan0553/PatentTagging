//
// Created by yuan on 7/19/19.
//
#include <iostream>
#include <string>
#include <utility>
#include <thread>

using namespace std;


template <typename... RunArgs>
class ThreadJob {
    void runWrapper(RunArgs... runArgs)
    {
        cout << "ThreadJob: " << this_thread::get_id() << endl;
        internalRun(runArgs...);
    }

protected:
    thread thread_;
    virtual void internalRun(RunArgs...) = 0;

    ThreadJob() = default;

public:
    virtual void run(RunArgs... runArgs)
    {
        thread_ = thread(&ThreadJob::runWrapper, this, runArgs...);
    }

    virtual void wait()
    {
        thread_.join();
    }
};


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


template <typename... T>
void test(ThreadJob<T...>& base, T&&... args)
{
    base.run(forward<T>(args)...);
}


int main()
{
    SubA a;
    SubB b;
    a.run("blah", 3);
    b.run(3.14);

    a.wait();
    b.wait();

    return 0;
}
