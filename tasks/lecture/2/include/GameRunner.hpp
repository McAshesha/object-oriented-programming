
#pragma once
#include <vector>
#include <array>
#include <iostream>
#include <memory>

#include "Strategy.hpp"
#include "PayoffMatrix.hpp"

struct RoundInfo
{
    std::array<Move,3> decisions;
    std::array<int,3> scores;
};

class GameRunner
{
public:
    explicit GameRunner(PayoffMatrix pm)
        : payoff(std::move(pm))
    {
    }

    std::vector<RoundInfo> run(std::vector<std::unique_ptr<Strategy>> &players,
                               int steps,
                               bool detailed,
                               std::istream &in,
                               std::ostream &out);

private:
    PayoffMatrix payoff;
};
