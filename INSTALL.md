# GITI Factory Presets — Drop into SALEK HIGHTECH VST

## Files in this repo (`Source/`)

| File | What to do |
|------|------------|
| `PluginProcessorPresetsEngineered.inl` | **Replace** your existing file (contains GITI/001–050 + core banks) |
| `PluginProcessorPresets.inl` | Optional reference — must end with `#include "PluginProcessorPresetsEngineered.inl"` |
| `PluginProcessorPresetsGITI.inl` | Safe stub |
| `GitiIdentities.h` | Optional |
| `GitiSonicDNA.h` | Optional (included by presets.inl) |

## Steps

1. Open your SALEK HIGHTECH project (the full JUCE source you already build).
2. Copy these files into `Source/`.
3. Confirm `PluginProcessorPresets.inl` ends with:
   ```cpp
   #include "PluginProcessorPresetsEngineered.inl"
   }
   ```
4. Rebuild (Visual Studio / CMake / local — **no GitHub Actions required**).
5. In the host, open factory presets → `GITI/001 The First Breath` … `GITI/050 The Last Voice`.

## Why not full VST source here?

The complete JUCE tree is large and private on the original account.
This package is the **complete GITI preset integration** — drop-in, build-safe, no missing includes.

## Billing note

If the original repo Actions fails with spending limit, build **locally** — the code does not need Actions to work.
