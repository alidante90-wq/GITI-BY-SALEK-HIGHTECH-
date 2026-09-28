# GITI BY SALEK HIGHTECH

Factory presets for SALEK HIGHTECH VST — **GITI 001–050**.

## Quick drop-in

Copy into your SALEK HIGHTECH `Source/` folder:

| File | Action |
|------|--------|
| `PluginProcessorPresetsEngineered.inl` | **Replace** existing (self-contained: core banks + all 50 GITI) |
| `PluginProcessorPresetsGITI.inl` | Optional stub (safe if included) |
| `GitiIdentities.h` | Optional identity metadata |

Your `PluginProcessorPresets.inl` should already end with:

```cpp
#include "PluginProcessorPresetsEngineered.inl"
}
```

Then rebuild the VST.

## Preset names in host

- `GITI/001 The First Breath`
- …
- `GITI/050 The Last Voice`

## Notes

- No nested missing includes (build-safe).
- No Web3 / wallet / mint in this package.
- Original SALEK UI layout unchanged.
