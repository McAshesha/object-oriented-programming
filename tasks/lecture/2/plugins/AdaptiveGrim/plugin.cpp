#include "../../include/Strategy.hpp"
#include <fstream>
#include <string>

// An adaptive grim strategy: cooperate until someone defects,
// then defect for 3 rounds; if both cooperate for 2 consecutive rounds, forgive.
class AdaptiveGrim final : public Strategy
{
public:
    std::string id() const override { return "AdaptiveGrim"; }

    Move decide(const std::vector<Move> &selfHistory,
                const std::vector<Move> &opponentAHistory,
                const std::vector<Move> &opponentBHistory) override
    {
        (void)selfHistory;
        int punish = cooldown;
        if (punish > 0) return Move::D;

        // If last round any defected, start cooldown
        if (!opponentAHistory.empty() && !opponentBHistory.empty())
        {
            if (opponentAHistory.back() == Move::D || opponentBHistory.back() == Move::D)
            {
                cooldown = punishmentLength;
                return Move::D;
            }
        }
        return Move::C;
    }

    void onRoundEnd(Move, Move a, Move b) override
    {
        // Decrease cooldown
        if (cooldown > 0) cooldown--;

        // Track forgiveness window: if both cooperated, increase streak, else reset.
        if (a == Move::C && b == Move::C) forgiveStreak++;
        else forgiveStreak = 0;

        // If cooperating for forgiveThreshold, reset cooldown early.
        if (forgiveStreak >= forgiveThreshold)
        {
            cooldown = 0;
            forgiveStreak = 0;
        }
    }

    void configure(const std::string &cfg) override
    {
        if (cfg.empty()) return;
        try
        {
            std::ifstream in(cfg);
            std::string line;
            while (std::getline(in, line))
            {
                auto poshash = line.find('#');
                if (poshash != std::string::npos) line = line.substr(0, poshash);
                auto findKV = [&](const std::string &k)->std::string
                {
                    auto p = line.find(k + "=");
                    if (p == std::string::npos) return "";
                    return line.substr(p + k.size() + 1);
                };
                auto s1 = findKV("punishment");
                if (!s1.empty()) punishmentLength = std::max(1, std::stoi(s1));
                auto s2 = findKV("forgive");
                if (!s2.empty()) forgiveThreshold = std::max(1, std::stoi(s2));
            }
        }
        catch (...) { }
    }

private:
    int cooldown = 0;
    int forgiveStreak = 0;
    int punishmentLength = 3;
    int forgiveThreshold = 2;
};

extern "C"
{
    const char* strategy_id()
    {
        return "AdaptiveGrim";
    }
    Strategy* create_strategy()
    {
        return new AdaptiveGrim();
    }
    void destroy_strategy(Strategy *p)
    {
        delete p;
    }
}
