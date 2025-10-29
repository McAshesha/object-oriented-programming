
#include "PayoffMatrix.hpp"
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <iostream>

namespace
{
    inline int ToBit(Move m)
    {
        return m == Move::D ? 1 : 0;
    }
}

PayoffMatrix PayoffMatrix::Default()
{
    PayoffMatrix pm;
    auto setScore = [&](Move a, Move b, Move c, int s0, int s1, int s2)
    {
        pm.matrix[ToBit(a)][ToBit(b)][ToBit(c)] = {s0, s1, s2};
    };
    // Default matrix (three-player PD)
    setScore(Move::C, Move::C, Move::C, 7, 7, 7);
    setScore(Move::C, Move::C, Move::D, 3, 3, 9);
    setScore(Move::C, Move::D, Move::C, 3, 9, 3);
    setScore(Move::C, Move::D, Move::D, 0, 5, 5);
    setScore(Move::D, Move::C, Move::C, 9, 3, 3);
    setScore(Move::D, Move::C, Move::D, 5, 0, 5);
    setScore(Move::D, Move::D, Move::C, 5, 5, 0);
    setScore(Move::D, Move::D, Move::D, 1, 1, 1);
    return pm;
}

bool PayoffMatrix::ParseLine(const std::string &line, std::array<int,3> &scores, std::array<Move,3> &moves)
{
    std::string s = line;
    auto posHash = s.find('#');
    if (posHash != std::string::npos) s = s.substr(0, posHash);

    auto NotSpace = [](int ch){ return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), NotSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), NotSpace).base(), s.end());
    if (s.empty()) return false;

    std::string key;
    int a = 0, b = 0, c = 0;
    {
        std::stringstream ss(s);
        if (!(ss >> key >> a >> b >> c)) return false;
    }
    if (key.size() != 3) return false;

    auto ParseMove = [](char ch)->Move
    {
        return (ch == 'D' || ch == 'd') ? Move::D : Move::C;
    };

    moves = { ParseMove(key[0]), ParseMove(key[1]), ParseMove(key[2]) };
    scores = { a, b, c };
    return true;
}

PayoffMatrix PayoffMatrix::FromFile(const std::string &filename)
{
    PayoffMatrix pm = Default();
    if (filename.empty()) return pm;

    try
    {
        std::ifstream in(filename);
        in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        std::string line;
        int applied = 0;
        while (std::getline(in, line))
        {
            std::array<int,3> s;
            std::array<Move,3> mv;
            if (!ParseLine(line, s, mv)) continue;
            pm.matrix[ToBit(mv[0])][ToBit(mv[1])][ToBit(mv[2])] = s;
            ++applied;
        }
        if (applied < 8)
        {
            std::cerr << "[warn] Matrix file '" << filename << "' provided fewer than 8 valid rows; using defaults for the rest.\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[warn] Failed to read matrix: " << e.what() << ". Using default.\n";
        return Default();
    }
    return pm;
}
