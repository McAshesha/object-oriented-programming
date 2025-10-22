
#include "GameRunner.hpp"

std::vector<RoundInfo> GameRunner::run(std::vector<std::unique_ptr<Strategy>> &players,
                                       int steps,
                                       bool detailed,
                                       std::istream &in,
                                       std::ostream &out)
{
    const int P = 3;
    std::vector<std::vector<Move>> histories(P);
    std::vector<long long> totals(P, 0);
    std::vector<RoundInfo> log;
    log.reserve(steps);

    for (int t = 1; t <= steps; ++t)
    {
        if (detailed)
        {
            out << "\n[step " << t << "] Press Enter for next, or 'q' to quit: " << std::flush;
            std::string line;
            if (!std::getline(in, line)) line.clear();
            if (!line.empty() && (line[0]=='q' || line[0]=='Q')) break;
        }

        std::array<Move,3> decision{};
        for (int i = 0; i < P; ++i)
        {
            const std::vector<Move> &selfHistory = histories[i];
            const std::vector<Move> &opponentA = histories[(i+1)%P];
            const std::vector<Move> &opponentB = histories[(i+2)%P];
            decision[i] = players[i]->decide(selfHistory, opponentA, opponentB);
        }

        auto sc = payoff.scores(decision[0], decision[1], decision[2]);
        for (int i = 0; i < P; ++i)
        {
            histories[i].push_back(decision[i]);
            totals[i] += sc[i];
        }
        for (int i = 0; i < P; ++i)
        {
            players[i]->onRoundEnd(decision[i], decision[(i+1)%P], decision[(i+2)%P]);
        }

        RoundInfo info{decision, sc};
        log.push_back(info);

        if (detailed)
        {
            out << " moves: [" << ToChar(decision[0]) << " "
                << ToChar(decision[1]) << " "
                << ToChar(decision[2]) << "]  "
                << "scores: [" << sc[0] << " " << sc[1] << " " << sc[2] << "]  "
                << "totals: [" << totals[0] << " " << totals[1] << " " << totals[2] << "]\n";
        }
    }

    if (!detailed)
    {
        long long s0=0,s1=0,s2=0;
        for (auto &ri : log) { s0+=ri.scores[0]; s1+=ri.scores[1]; s2+=ri.scores[2]; }
        out << "Final totals: [" << s0 << " " << s1 << " " << s2 << "]\n";
    }
    return log;
}
