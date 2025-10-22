
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

struct CLI
{
    std::vector<std::string> strategyNames;
    std::string mode = "";
    int steps = 50;
    std::string configsDir = "";
    std::string matrixFile = "";
    std::string pluginsDir = "plugins";

    static bool startsWith(const std::string &s, const std::string &p)
    {
        return s.size() >= p.size() && std::equal(p.begin(), p.end(), s.begin());
    }

    void parse(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string a = argv[i];
            if (startsWith(a, "--mode=")) mode = a.substr(7);
            else if (startsWith(a, "--steps=")) steps = std::max(1, std::stoi(a.substr(8)));
            else if (startsWith(a, "--configs=")) configsDir = a.substr(10);
            else if (startsWith(a, "--matrix=")) matrixFile = a.substr(9);
            else if (startsWith(a, "--plugins=")) pluginsDir = a.substr(10);
            else if (!a.empty() && a[0] == '-') { /* ignore */ }
            else strategyNames.push_back(a);
        }
        if (strategyNames.size() < 3)
        {
            std::cerr << "Usage: " << argv[0] << " <S1> <S2> <S3> [S4 ...] "
                      << "[--mode=detailed|fast|tournament] [--steps=N] "
                      << "[--configs=DIR] [--plugins=DIR] [--matrix=FILE]\n";
            std::exit(1);
        }
        if (mode.empty())
        {
            mode = (strategyNames.size() > 3) ? "tournament" : "detailed";
        }
    }
};
