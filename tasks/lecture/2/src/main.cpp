
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include "CLI.hpp"
#include "PayoffMatrix.hpp"
#include "GameRunner.hpp"
#include "StrategyFactory.hpp"

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    CLI cli;
    cli.parse(argc, argv);

    PayoffMatrix matrix;
    try
    {
        matrix = PayoffMatrix::FromFile(cli.matrixFile);
    }
    catch (...)
    {
        matrix = PayoffMatrix::Default();
    }

    if (cli.mode == "detailed" || cli.mode == "fast")
    {
        std::vector<std::unique_ptr<Strategy>> players;
        for (int i = 0; i < 3; ++i)
        {
            players.push_back(StrategyFactory::create(cli.strategyNames[i], cli.configsDir, cli.pluginsDir));
        }

        std::cout << "Mode: " << cli.mode << ", steps=" << cli.steps << "\n";
        std::cout << "Players: [" << players[0]->id() << ", " << players[1]->id() << ", " << players[2]->id() << "]\n";

        GameRunner runner(matrix);
        bool detailed = (cli.mode == "detailed");
        runner.run(players, cli.steps, detailed, std::cin, std::cout);
    }
    else if (cli.mode == "tournament")
    {
        std::cout << "Mode: tournament, steps=" << cli.steps << "\n";
        const int n = (int)cli.strategyNames.size();

        std::unordered_map<std::string, long long> grand;
        struct MatchResult { std::array<std::string,3> names; std::array<long long,3> totals; };
        std::vector<MatchResult> results;

        for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
        for (int k = j+1; k < n; ++k)
        {
            std::array<std::string,3> names = {cli.strategyNames[i], cli.strategyNames[j], cli.strategyNames[k]};
            std::vector<std::unique_ptr<Strategy>> players;
            for (int t = 0; t < 3; ++t)
            {
                players.push_back(StrategyFactory::create(names[t], cli.configsDir, cli.pluginsDir));
            }

            GameRunner runner(matrix);
            auto log = runner.run(players, cli.steps, /*detailed*/false, std::cin, std::cout);

            std::array<long long,3> totals{0,0,0};
            for (auto &ri : log)
            {
                totals[0] += ri.scores[0];
                totals[1] += ri.scores[1];
                totals[2] += ri.scores[2];
            }

            std::cout << "Match: [" << names[0] << ", " << names[1] << ", " << names[2]
                    << "] -> totals: [" << totals[0] << " " << totals[1] << " " << totals[2] << "]\n";

            results.push_back({names, totals});
            for (int t = 0; t < 3; ++t) grand[names[t]] += totals[t];
        }

        std::vector<std::pair<std::string,long long>> lb(grand.begin(), grand.end());
        std::sort(lb.begin(), lb.end(), [](auto &a, auto &b){ return a.second > b.second; });

        std::cout << "\n=== Leaderboard (sum over all matches) ===\n";
        for (size_t r = 0; r < lb.size(); ++r)
        {
            std::cout << (r+1) << ". " << lb[r].first << " : " << lb[r].second << "\n";
        }
    }
    else
    {
        std::cerr << "Unknown mode: " << cli.mode << "\n";
        return 2;
    }

    return 0;
}
