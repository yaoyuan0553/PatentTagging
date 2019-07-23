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
#include "ConcurrentQueue.h"


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


struct ThreadPoolInterface {
    virtual void runAll() = 0;
    virtual void waitAll() = 0;
};


template <typename DataType, typename ProducerThread, typename ConsumerThread>
class PCThreadPool : public ThreadPoolInterface {
    static_assert(std::conjunction_v<
            is_base_of_template<ThreadJob, ProducerThread>,
            is_base_of_template<ThreadJob, ConsumerThread>>,
        "Template argument(s) of PCThreadPool must be subclass(es) of ThreadJob!");

    ConcurrentQueue<DataType> dataQueue_;
    std::vector<ProducerThread> producers_;
    std::vector<ConsumerThread> consumers_;

protected:

    template <typename ProducerInitFn, typename ConsumerInitFn>
    PCThreadPool(int pN, ProducerInitFn producerInitFn, int cN, ConsumerInitFn consumerInitFn)
    {
        static_assert(std::conjunction_v<
                std::is_invocable<ProducerInitFn, decltype(producers_)&>,
                std::is_invocable<ConsumerInitFn, decltype(consumers_)&>>,
            "Expects function of type with arguments ProducerInitFn(vector<ProducerThread>&), "
            "and ConsumerInitFn(vector<ConsumerThread>&>)");
        producers_.reserve(pN);
        consumers_.reserve(cN);
        ProducerInitFn(this->producers_);
        ConsumerInitFn(this->consumers_);
    }


public:
    void runAll() override
    {
        for (auto& pt : producers_)
            pt.run();
    }

    void waitAll() override
    {
    }
};



#endif //TOOLS_THREADPOOL_H
