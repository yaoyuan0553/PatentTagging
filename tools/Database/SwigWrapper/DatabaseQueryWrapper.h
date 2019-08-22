//
// Created by yuan on 2019/8/22.
//

#pragma once
#ifndef TOOLS_DATABASEQUERYSELECTORSWIG_H
#define TOOLS_DATABASEQUERYSELECTORSWIG_H

#include <string>

void hello();

template <typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
};

void printStr(const char* str);

std::string makeStr();

void takeStr(std::string str);

void changeStr(char* str, int length);

void fillArray(int arr[], int n);

void changeIntPointerArray(int* array, int n);

#endif //TOOLS_DATABASEQUERYSELECTORSWIG_H
