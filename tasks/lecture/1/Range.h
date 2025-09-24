#pragma once

#include <limits>

class Range
{
private:
    int _start;
    int _end;

public:
    Range();
    Range(int startValue, int endValue);

    int size() const;
    bool empty() const;
    int start() const;
    int end() const;

    static Range all();
};
