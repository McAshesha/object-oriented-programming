
#include "StrategyFactory.hpp"
#include "strategies/BuiltinStrategies.hpp"
#include <fstream>
#include <iostream>
#include <memory>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace
{
    std::unique_ptr<Strategy> MakeBuiltin(const std::string &name)
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

    bool EndsWith(const std::string &s, const std::string &suffix)
    {
        if (s.size() < suffix.size()) return false;
        return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
    }

    std::string BuildPluginPath(const std::string &pluginsDir, const std::string &name)
    {
        // If path-like or explicit extension is provided, use as-is
        if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos)
            return name;
#if defined(_WIN32)
        if (EndsWith(name, ".dll")) return name;
        return pluginsDir + "/Strategy_" + name + ".dll";
#else
        if (EndsWith(name, ".so")) return name;
        return pluginsDir + "/libStrategy_" + name + ".so";
#endif
    }
}

std::unique_ptr<Strategy> StrategyFactory::create(const std::string &name,
                                                  const std::string &configsDir,
                                                  const std::string &pluginsDir)
{
    // Built-ins first
    if (auto b = MakeBuiltin(name))
    {
        // Apply config if exists
        if (!configsDir.empty())
        {
            std::string cfg = configsDir;
#if defined(_WIN32)
            if (!cfg.empty() && cfg.back() != '\\' && cfg.back() != '/') cfg += "\\";
#else
            if (!cfg.empty() && cfg.back() != '/' ) cfg += "/";
#endif
            cfg += b->id();
            cfg += ".cfg";
            b->configure(cfg);
        }
        return b;
    }

    // Plugin form "plugin:<path or name>" to force plugin loading
    const std::string prefix = "plugin:";
    std::string pluginToken = name;
    if (pluginToken.rfind(prefix, 0) == 0)
    {
        pluginToken = pluginToken.substr(prefix.size());
    }

    std::string libPath = BuildPluginPath(pluginsDir, pluginToken);

#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(libPath.c_str());
    if (!handle)
    {
        std::cerr << "[error] Failed to LoadLibrary: " << libPath << "\n";
        return std::unique_ptr<Strategy>{};
    }
    auto createFn = (Strategy* (__cdecl *)(void))GetProcAddress(handle, "create_strategy");
    auto destroyFn = (void (__cdecl *)(Strategy*))GetProcAddress(handle, "destroy_strategy");
    auto idFn = (const char* (__cdecl *)(void))GetProcAddress(handle, "strategy_id");
    if (!createFn || !destroyFn || !idFn)
    {
        std::cerr << "[error] Plugin symbols missing in: " << libPath << "\n";
        FreeLibrary(handle);
        return std::unique_ptr<Strategy>{};
    }
#else
    void *handle = dlopen(libPath.c_str(), RTLD_NOW);
    if (!handle)
    {
        std::cerr << "[error] Failed to dlopen: " << libPath << " : " << dlerror() << "\n";
        return std::unique_ptr<Strategy>{};
    }
    auto createFn = (Strategy* (*)()) dlsym(handle, "create_strategy");
    auto destroyFn = (void (*)(Strategy*)) dlsym(handle, "destroy_strategy");
    auto idFn = (const char* (*)()) dlsym(handle, "strategy_id");
    if (!createFn || !destroyFn || !idFn)
    {
        std::cerr << "[error] Plugin symbols missing in: " << libPath << "\n";
        dlclose(handle);
        return std::unique_ptr<Strategy>{};
    }
#endif

    Strategy *raw = createFn();
    if (!raw)
    {
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return std::unique_ptr<Strategy>{};
    }

    struct PluginDeleter
    {
        void *libHandle;
#if defined(_WIN32)
        void (*destroy)(Strategy*);
        void operator()(Strategy *p) const
        {
            if (p && destroy) destroy(p);
            if (libHandle) FreeLibrary((HMODULE)libHandle);
        }
#else
        void (*destroy)(Strategy*);
        void operator()(Strategy *p) const
        {
            if (p && destroy) destroy(p);
            if (libHandle) dlclose(libHandle);
        }
#endif
    };

    std::unique_ptr<Strategy, PluginDeleter> ptr(raw, PluginDeleter{handle, destroyFn});

    // Optional: verify id / apply config
    const char *sid = idFn();
    std::string id = sid ? std::string(sid) : std::string("PluginStrategy");
    if (!configsDir.empty())
    {
        std::string cfg = configsDir;
#if defined(_WIN32)
        if (!cfg.empty() && cfg.back() != '\\' && cfg.back() != '/') cfg += "\\";
#else
        if (!cfg.empty() && cfg.back() != '/' ) cfg += "/";
#endif
        cfg += id;
        cfg += ".cfg";
        ptr->configure(cfg);
    }

    return std::unique_ptr<Strategy>(ptr.release());
}
