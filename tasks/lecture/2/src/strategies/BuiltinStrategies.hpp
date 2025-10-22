
#pragma once
#include <memory>
#include <random>
#include <deque>
#include <fstream>
#include <array>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "Strategy.hpp"

class AlwaysC final : public Strategy
{
public:
    std::string id() const override { return "AlwaysC"; }
    Move decide(const std::vector<Move>&, const std::vector<Move>&, const std::vector<Move>&) override
    {
        return Move::C;
    }
};

class AlwaysD final : public Strategy
{
public:
    std::string id() const override { return "AlwaysD"; }
    Move decide(const std::vector<Move>&, const std::vector<Move>&, const std::vector<Move>&) override
    {
        return Move::D;
    }
};

class RandomStrategy final : public Strategy
{
    double cooperateProb = 0.5;
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist{0.0, 1.0};
public:
    std::string id() const override { return "Random"; }

    void configure(const std::string &cfg) override
    {
        if (cfg.empty()) return;
        try
        {
            std::ifstream in(cfg);
            in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            std::string line;
            while (std::getline(in, line))
            {
                auto poshash = line.find('#');
                if (poshash != std::string::npos) line = line.substr(0, poshash);
                if (line.find("prob=") != std::string::npos)
                {
                    double p = std::stod(line.substr(line.find("prob=")+5));
                    if (p >= 0.0 && p <= 1.0) cooperateProb = p;
                }
            }
        }
        catch (...) { /* silent */ }
    }

    Move decide(const std::vector<Move>&, const std::vector<Move>&, const std::vector<Move>&) override
    {
        return (dist(rng) < cooperateProb) ? Move::C : Move::D;
    }
};

class TitForTat3 final : public Strategy
{
public:
    std::string id() const override { return "TitForTat"; }
    Move decide(const std::vector<Move>& selfHistory,
                const std::vector<Move>& oppA, const std::vector<Move>& oppB) override
    {
        if (selfHistory.empty()) return Move::C;
        return (oppA.back() == Move::C && oppB.back() == Move::C) ? Move::C : Move::D;
    }
};

class GrimTrigger3 final : public Strategy
{
    bool grim = false;
public:
    std::string id() const override { return "Grim"; }
    Move decide(const std::vector<Move>&, const std::vector<Move>&, const std::vector<Move>&) override
    {
        return grim ? Move::D : Move::C;
    }
    void onRoundEnd(Move, Move a, Move b) override
    {
        if (a == Move::D || b == Move::D) grim = true;
    }
};

class TwoTitsForTat3 final : public Strategy
{
    std::deque<std::array<Move,2>> recent;
public:
    std::string id() const override { return "TwoTits"; }
    Move decide(const std::vector<Move>& selfHistory,
                const std::vector<Move>&, const std::vector<Move>&) override
    {
        if (selfHistory.size() < 2) return Move::C;
        for (auto &p : recent)
        {
            if (p[0] == Move::D || p[1] == Move::D) return Move::D;
        }
        return Move::C;
    }
    void onRoundEnd(Move, Move a, Move b) override
    {
        if (recent.size() == 2) recent.pop_front();
        recent.push_back(std::array<Move,2>{a,b});
    }
};

class MetaMajority final : public Strategy
{
    std::vector<std::unique_ptr<Strategy>> advisors;

    static std::unique_ptr<Strategy> makeAdvisor(const std::string &name);
public:
    std::string id() const override { return "MetaMajority"; }
    void configure(const std::string &cfg) override;
    Move decide(const std::vector<Move>& selfHistory,
                const std::vector<Move>& aHist, const std::vector<Move>& bHist) override;
    void onRoundEnd(Move s, Move a, Move b) override;
};
