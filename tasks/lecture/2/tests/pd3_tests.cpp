
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <random>

#include "Strategy.hpp"
#include "PayoffMatrix.hpp"
#include "GameRunner.hpp"
#include "StrategyFactory.hpp"

using std::vector;
using std::string;

namespace fs = std::filesystem;

// Helpers
static inline std::array<int,3> S(const PayoffMatrix &pm, Move a, Move b, Move c)
{
    return pm.scores(a,b,c);
}

TEST(PayoffMatrixTest, DefaultValues)
{
    PayoffMatrix pm = PayoffMatrix::Default();
    EXPECT_EQ(S(pm, Move::C, Move::C, Move::C), (std::array<int,3>{7,7,7}));
    EXPECT_EQ(S(pm, Move::C, Move::C, Move::D), (std::array<int,3>{3,3,9}));
    EXPECT_EQ(S(pm, Move::C, Move::D, Move::C), (std::array<int,3>{3,9,3}));
    EXPECT_EQ(S(pm, Move::D, Move::C, Move::C), (std::array<int,3>{9,3,3}));
    EXPECT_EQ(S(pm, Move::C, Move::D, Move::D), (std::array<int,3>{0,5,5}));
    EXPECT_EQ(S(pm, Move::D, Move::C, Move::D), (std::array<int,3>{5,0,5}));
    EXPECT_EQ(S(pm, Move::D, Move::D, Move::C), (std::array<int,3>{5,5,0}));
    EXPECT_EQ(S(pm, Move::D, Move::D, Move::D), (std::array<int,3>{1,1,1}));
}

TEST(PayoffMatrixTest, ParseLineVariants)
{
    std::array<int,3> scores;
    std::array<Move,3> moves;

    EXPECT_TRUE(PayoffMatrix::ParseLine("CCD 3 3 9", scores, moves));
    EXPECT_EQ(scores, (std::array<int,3>{3,3,9}));
    EXPECT_EQ(moves[0], Move::C);
    EXPECT_EQ(moves[1], Move::C);
    EXPECT_EQ(moves[2], Move::D);

    EXPECT_TRUE(PayoffMatrix::ParseLine(" ddc   5 5 0 # comment", scores, moves));
    EXPECT_EQ(scores, (std::array<int,3>{5,5,0}));
    EXPECT_EQ(moves[0], Move::D);
    EXPECT_EQ(moves[1], Move::D);
    EXPECT_EQ(moves[2], Move::C);

    EXPECT_FALSE(PayoffMatrix::ParseLine("invalid", scores, moves));
    EXPECT_FALSE(PayoffMatrix::ParseLine("", scores, moves));
}

TEST(FactoryTest, BuiltinStrategiesBasic)
{
    auto sC = StrategyFactory::create("AlwaysC", "", "");
    auto sD = StrategyFactory::create("AlwaysD", "", "");
    ASSERT_TRUE(sC);
    ASSERT_TRUE(sD);
    EXPECT_EQ(sC->id(), "AlwaysC");
    EXPECT_EQ(sD->id(), "AlwaysD");
    EXPECT_EQ(sC->decide({}, {}, {}), Move::C);
    EXPECT_EQ(sD->decide({}, {}, {}), Move::D);
}

TEST(FactoryTest, RandomConfiguredToAlwaysC)
{
    // Create a temp configs dir with Random.cfg => prob=1.0
    fs::path tmp = fs::temp_directory_path() / fs::path("pd3_test_cfgs");
    fs::create_directories(tmp);
    std::ofstream(tmp / "Random.cfg") << "prob=1.0\n";

    auto rnd = StrategyFactory::create("Random", tmp.string(), "");
    ASSERT_TRUE(rnd);
    for (int i = 0; i < 50; ++i)
    {
        EXPECT_EQ(rnd->decide({}, {}, {}), Move::C);
    }
}

TEST(StrategyBehaviorTest, GrimTriggerAndMetaMajority)
{
    auto grim = StrategyFactory::create("Grim", "", "");
    ASSERT_TRUE(grim);
    // Initially cooperates
    EXPECT_EQ(grim->decide({}, {}, {}), Move::C);
    // One opponent defects -> set grim state via onRoundEnd, then next decide is D
    grim->onRoundEnd(Move::C, Move::D, Move::C);
    EXPECT_EQ(grim->decide({Move::C}, {Move::D}, {Move::C}), Move::D);

    auto meta = StrategyFactory::create("MetaMajority", "", "");
    ASSERT_TRUE(meta);
    // Before any history: advisors default (AlwaysC, TitForTat, Grim)
    // Simulate previous round with one D -> TFT votes D, Grim now grim (votes D), AlwaysC votes C -> majority D
    meta->onRoundEnd(Move::C, Move::D, Move::C);
    EXPECT_EQ(meta->decide({Move::C}, {Move::D}, {Move::C}), Move::D);
}

TEST(GameRunnerTest, SimpleTotals)
{
    PayoffMatrix pm = PayoffMatrix::Default();
    GameRunner runner(pm);

    // All cooperate
    {
        std::vector<std::unique_ptr<Strategy>> ps;
        ps.emplace_back(StrategyFactory::create("AlwaysC", "", ""));
        ps.emplace_back(StrategyFactory::create("AlwaysC", "", ""));
        ps.emplace_back(StrategyFactory::create("AlwaysC", "", ""));

        std::stringstream dummyIn; std::stringstream dummyOut;
        auto log = runner.run(ps, 5, false, dummyIn, dummyOut);

        long long t0=0,t1=0,t2=0;
        for (auto &r : log) { t0+=r.scores[0]; t1+=r.scores[1]; t2+=r.scores[2]; }
        EXPECT_EQ(t0, 5*7);
        EXPECT_EQ(t1, 5*7);
        EXPECT_EQ(t2, 5*7);
    }

    // D vs C vs C
    {
        std::vector<std::unique_ptr<Strategy>> ps;
        ps.emplace_back(StrategyFactory::create("AlwaysD", "", ""));
        ps.emplace_back(StrategyFactory::create("AlwaysC", "", ""));
        ps.emplace_back(StrategyFactory::create("AlwaysC", "", ""));

        std::stringstream dummyIn; std::stringstream dummyOut;
        auto log = GameRunner(pm).run(ps, 4, false, dummyIn, dummyOut);

        long long t0=0,t1=0,t2=0;
        for (auto &r : log) { t0+=r.scores[0]; t1+=r.scores[1]; t2+=r.scores[2]; }
        // DCC -> 9,3,3 each round
        EXPECT_EQ(t0, 4*9);
        EXPECT_EQ(t1, 4*3);
        EXPECT_EQ(t2, 4*3);
    }
}

TEST(PluginTest, LoadAdaptiveGrim)
{
#ifndef PLUGINS_DIR
#define STR2(x) #x
#define STR(x) STR2(x)
#pragma message("PLUGINS_DIR is not defined, plugin test will be skipped.")
    GTEST_SKIP() << "PLUGINS_DIR not defined";
#else
    std::string pluginsDir = PLUGINS_DIR;
    auto p = StrategyFactory::create("AdaptiveGrim", "", pluginsDir);
    ASSERT_TRUE(p) << "Failed to load plugin from: " << pluginsDir;
    EXPECT_EQ(p->id(), "AdaptiveGrim");

    // First round: no history -> cooperate
    EXPECT_EQ(p->decide({}, {}, {}), Move::C);

    // If someone defected, expect a few Ds (cooldown default = 3)
    p->onRoundEnd(Move::C, Move::D, Move::C);
    EXPECT_EQ(p->decide({Move::C}, {Move::D}, {Move::C}), Move::D);
    p->onRoundEnd(Move::D, Move::C, Move::C);
    EXPECT_EQ(p->decide({Move::C,Move::D}, {Move::D,Move::C}, {Move::C,Move::C}), Move::D);
#endif
}
