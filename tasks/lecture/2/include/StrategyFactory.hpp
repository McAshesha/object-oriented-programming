
#pragma once
#include <memory>
#include <string>

#include "Strategy.hpp"

class StrategyFactory
{
public:
    static std::unique_ptr<Strategy> create(const std::string &name,
                                            const std::string &configsDir,
                                            const std::string &pluginsDir);
};
