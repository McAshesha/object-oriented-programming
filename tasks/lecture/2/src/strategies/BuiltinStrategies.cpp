#include <algorithm>

#include "strategies/BuiltinStrategies.hpp"
#include <fstream>
#include <sstream>
#include <cctype>

static std::unique_ptr<Strategy> MakeBuiltinByName(const std::string &name)
{
    std::string k = name;
    for (auto &ch : k) ch = (char)std::tolower(ch);
    if (k == "alwaysc" || k == "ac") return std::make_unique<AlwaysC>();
    if (k == "alwaysd" || k == "ad") return std::make_unique<AlwaysD>();
    if (k == "random"  || k == "rnd") return std::make_unique<RandomStrategy>();
    if (k == "titfortat" || k == "tft") return std::make_unique<TitForTat3>();
    if (k == "grim") return std::make_unique<GrimTrigger3>();
    if (k == "twotits" || k == "tt") return std::make_unique<TwoTitsForTat3>();
    if (k == "metamajority" || k == "meta") return std::make_unique<MetaMajority>();
    return nullptr;
}

std::unique_ptr<Strategy> MetaMajority::makeAdvisor(const std::string &name)
{
    auto p = MakeBuiltinByName(name);
    if (p) return p;
    // Fallback
    return std::make_unique<TitForTat3>();
}

void MetaMajority::configure(const std::string &cfg)
{
    advisors.clear();
    if (!cfg.empty())
    {
        try
        {
            std::ifstream in(cfg);
            in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            std::string line;
            while (std::getline(in, line))
            {
                auto poshash = line.find('#');
                if (poshash != std::string::npos) line = line.substr(0, poshash);
                auto pos = line.find("members=");
                if (pos != std::string::npos)
                {
                    std::string members = line.substr(pos + 8);
                    std::stringstream ss(members);
                    std::string token;
                    while (std::getline(ss, token, ','))
                    {
                        auto notspace = [](int ch){ return !std::isspace(ch); };
                        token.erase(token.begin(), std::find_if(token.begin(), token.end(), notspace));
                        token.erase(std::find_if(token.rbegin(), token.rend(), notspace).base(), token.end());
                        if (!token.empty()) advisors.push_back(makeAdvisor(token));
                    }
                }
            }
        }
        catch (...) { }
    }
    if (advisors.empty())
    {
        advisors.push_back(std::make_unique<AlwaysC>());
        advisors.push_back(std::make_unique<TitForTat3>());
        advisors.push_back(std::make_unique<GrimTrigger3>());
    }
}

Move MetaMajority::decide(const std::vector<Move>& selfHistory,
                          const std::vector<Move>& aHist, const std::vector<Move>& bHist)
{
    int votesC = 0, votesD = 0;
    for (auto &adv : advisors)
    {
        Move m = adv->decide(selfHistory, aHist, bHist);
        (m == Move::C ? votesC : votesD)++;
    }
    if (votesC == votesD) return Move::D;
    return (votesC > votesD) ? Move::C : Move::D;
}

void MetaMajority::onRoundEnd(Move s, Move a, Move b)
{
    for (auto &adv : advisors) adv->onRoundEnd(s, a, b);
}
