//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_STATICQUEUE_H
#define TOOLS_STATICQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "Utility.h"

template <typename T>
class StaticQueue {
protected:
    T* buffer_;

    size_t      maxSize_;                   // maximum size
    size_t      curSize_ = 0;               // current size

    size_t      head_ = 0,
                tail_ = 0;                  // index of head and tail

public:
    explicit StaticQueue(size_t maxSize) : buffer_(new T[maxSize]), maxSize_(maxSize) { }

    virtual ~StaticQueue() { delete[] buffer_; }

    inline size_t size() const { return curSize_; }
    inline size_t capacity() const { return maxSize_; }
    inline bool full() const { return curSize_ == maxSize_; }
    inline bool empty() const { return curSize_ == 0; }

    void push(const T& item)
    {
        if (full())
            PERROR("trying to push into a full queue");
        // copy constructs
        buffer_[tail_] = item;
        curSize_++;
        tail_ = (tail_ + 1) % maxSize_;
    }

    void emplace_push(T&& item)
    {
        if (full())
            PERROR("trying to push into a full queue");
        buffer_[tail_] = std::move(item);
        curSize_++;
        tail_ = (tail_ + 1) % maxSize_;
    }

    T pop()
    {
        if (empty())
            PERROR("trying to pop from an empty queue");

        curSize_--;
        auto oldHead = head_;
        head_ = (head_ + 1) % maxSize_;

        return buffer_[oldHead];
    }
};


#endif //TOOLS_STATICQUEUE_H
