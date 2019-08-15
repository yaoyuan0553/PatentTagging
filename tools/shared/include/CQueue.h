//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_CQUEUE_H
#define TOOLS_CQUEUE_H

#include "ConcurrentQueue.h"
#include "ConcurrentStaticQueue.h"

#define USE_CONCURRENT_STATIC_QUEUE
//#define USE_CONCURRENT_QUEUE


#ifdef USE_CONCURRENT_QUEUE

/* name alias for shortening class name */
template <typename T, typename Container = std::deque<T>>
using CQueue = ConcurrentQueue<T, Container>;

#endif

#ifdef USE_CONCURRENT_STATIC_QUEUE

template <typename T> using CQueue = ConcurrentStaticQueue<T>;

#endif


#endif //TOOLS_CQUEUE_H
