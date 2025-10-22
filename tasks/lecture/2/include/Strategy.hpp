
#pragma once
#include <string>
#include <vector>

enum class Move { C, D };

static inline char ToChar(Move m)
{
    return m == Move::C ? 'C' : 'D';
}

class Strategy
{
public:
    virtual ~Strategy() = default;
    virtual std::string id() const = 0;
    virtual void configure(const std::string &configFilePath)
    {
        (void)configFilePath;
    }
    virtual Move decide(const std::vector<Move> &selfHistory,
                        const std::vector<Move> &opponentAHistory,
                        const std::vector<Move> &opponentBHistory) = 0;

    virtual void onRoundEnd(Move self, Move opponentA, Move opponentB)
    {
        (void)self; (void)opponentA; (void)opponentB;
    }
};
