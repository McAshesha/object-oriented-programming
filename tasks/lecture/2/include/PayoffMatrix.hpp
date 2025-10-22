
#pragma once
#include <array>
#include <string>

#include "Strategy.hpp"

struct PayoffMatrix
{
    std::array<std::array<std::array<std::array<int,3>,2>,2>,2> matrix{};

    static PayoffMatrix Default();
    static PayoffMatrix FromFile(const std::string &filename);

    static bool ParseLine(const std::string &line, std::array<int,3> &scores, std::array<Move,3> &moves);

    std::array<int,3> scores(Move a, Move b, Move c) const
    {
        auto ToBit = [](Move m){ return m == Move::D ? 1 : 0; };
        return matrix[ToBit(a)][ToBit(b)][ToBit(c)];
    }
};
