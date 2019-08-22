//
// Created by yuan on 2019/8/22.
//

#include "DatabaseQueryWrapper.h"

#include <iostream>
#include <stdio.h>

using namespace std;

void hello()
{
    std::cout << "Hello, World!" << std::endl;
}

void printStr(const char* str)
{
    printf("%s\n", str);
}

std::string makeStr()
{
    return "made this string";
}

void takeStr(std::string str)
{
    std::cout << str << '\n';
}

void changeStr(char* str, int length)
{
    cout << str << " " << length << '\n';

    for (int i = 0; i < length; i++) {
        str[i] = 'a';
    }

    printf("changed str: %s\n", str);
}

void fillArray(int arr[], int n)
{
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

void changeIntPointerArray(int* array, int n)
{
    for (int i = 0; i < n; i++)
        array[i] = i * 2;
}
