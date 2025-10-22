
# PD3 Dynamic (3-Player Prisoner's Dilemma)

## Build

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j
```

This produces:
- `pd3` — the main executable
- `libStrategy_AdaptiveGrim.so` (Linux/macOS) or `Strategy_AdaptiveGrim.dll` (Windows) — sample plugin

## Run (examples)

```bash
# Built-ins
./pd3 AlwaysC TitForTat Grim --mode=fast --steps=20

# Tournament with a plugin by name (resolved in ./plugins)
./pd3 AlwaysD TitForTat AdaptiveGrim --plugins=./build # when running from the build dir
./pd3 plugin:AdaptiveGrim AlwaysC Random --steps=40 --mode=fast

# Explicit path to a .so/.dll
./pd3 plugin:./build/libStrategy_AdaptiveGrim.so AlwaysC Random --mode=fast

# With configs
./pd3 Random MetaMajority AdaptiveGrim --configs=./configs --mode=detailed
```

## Plugin API

A plugin must define the following C symbols:
```cpp
extern "C" {
    const char* strategy_id();
    Strategy*   create_strategy();
    void        destroy_strategy(Strategy*);
}
```

Compile the plugin against the same `include/Strategy.hpp` to ensure ABI compatibility.

## Matrix file format

Optional `--matrix=matrix.txt`, 8 rows like:
```
CCC 7 7 7
CCD 3 3 9
CDC 3 9 3
DCC 9 3 3
CDD 0 5 5
DCD 5 0 5
DDC 5 5 0
DDD 1 1 1
```
