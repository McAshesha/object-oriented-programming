#include "Range.h"

Range::Range()
    : _start(0), _end(0)
{
}

Range::Range(int startValue, int endValue)
{
    if (startValue < 0 || startValue >= endValue)
    {
        _start = 0;
        _end = 0;
    }
    else
    {
        _start = startValue;
        _end = endValue;
    }
}

int Range::size() const
{
    return _end - _start;
}

bool Range::empty() const
{
    return size() == 0;
}

int Range::start() const
{
    return _start;
}

int Range::end() const
{
    return _end;
}

Range Range::all()
{
    return Range(0, std::numeric_limits<int>::max());
}
