#pragma once
#include "GitiIdentityBank.h"
#include "../GitiSonicDNA.h"
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace salek::giti
{
struct Preset { juce::String name; std::map<juce::String, float> values; };

inline std::uint32_t hash32 (const char* text) noexcept
{
    std::uint32_t h = 2166136261u;
    while (*text != '\0') { h ^= static_cast<unsigned char> (*text++); h *= 16777619u; }
    return h;
}
struct Random
{
    explicit Random (std::uint32_t s) : state (s ? s : 0x9e3779b9u) {}
    std::uint32_t next() noexcept { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
    float unit() noexcept { return static_cast<float> (next() & 0x00ffffffu) / 16777216.0f; }
    float range (float a, float b) noexcept { return a + (b - a) * unit(); }
    int pick (int n) noexcept { return static_cast<int> (next() % static_cast<std::uint32_t> (juce::jmax (1, n))); }
    std::uint32_t state;
};

inline std::vector<Preset> makePresets()
{
    static constexpr const char* categories[] = {
        "ATMOSPHERE", "LEAD", "BASS", "PULSE", "TEXTURE",
        "SEQUENCE", "FM", "FX", "PLUCK", "DRONE"
    };
    static constexpr const char* roles[] = {
        "Origin", "Signal", "Motion", "Shadow", "Halo",
        "Memory", "Vector", "Ritual", "Bloom", "Terminal"
    };
    static constexpr const char* bankNames[5][10] = {
        { "Air", "Reed", "Inhale", "Horizon", "Wind", "Lung", "Whisper", "Silver", "Dawn", "Sky" },
        { "Bow", "String", "Cedar", "Resonance", "Pluck", "Thread", "Tremolo", "Harp", "Lantern", "Cathedral" },
        { "Vowel", "Throat", "Choir", "Hum", "Name", "Lullaby", "Call", "Bloom", "Tongue", "Overtone" },
        { "Heart", "Frame", "Drum", "Impact", "Step", "Ember", "Thunder", "Ritual", "Sub", "Meter" },
        { "Rain", "Stone", "River", "Root", "Glass", "Cave", "Tide", "Seed", "Monsoon", "Earth" }
    };

    std::vector<Preset> result;
    result.reserve (identities.size() * 50);

    for (std::size_t gi = 0; gi < identities.size(); ++gi)
    {
        const auto& id = identities[gi];
        const SonicSeed base = seedForIndex ((int) gi);
        int branch = 0;
        if (std::string (id.branch) == "Strings") branch = 1;
        else if (std::string (id.branch) == "Voice") branch = 2;
        else if (std::string (id.branch) == "Pulse") branch = 3;
        else if (std::string (id.branch) == "Earth & Water") branch = 4;

        for (int pi = 0; pi < 50; ++pi)
        {
            const auto seed = hash32 ((std::string (id.number) + "|" + std::to_string (pi) + "|GITI-DNA-v2").c_str());
            Random rng (seed);
            const char* bank = bankNames[branch][rng.pick (10)];
            const char* role = roles[rng.pick (10)];
            const int categoryIndex = pi % 10;
            const char* category = categories[categoryIndex];
            const auto suffix = juce::String (pi + 1).paddedLeft ('0', 2);
            const auto displayName = juce::String ("GITI ") + id.number + "/" + id.name
                + " · " + bank + " " + role + " — " + category + " " + suffix;

            std::map<juce::String, float> v;
            applySeedToMap (base, v); // DNA base first

            auto r = [&] (float lo, float hi) { return rng.range (lo, hi); };
            auto nudge = [&] (const char* key, float lo, float hi)
            {
                auto it = v.find (key);
                float cur = (it != v.end()) ? it->second : 0.f;
                v[key] = juce::jlimit (lo, hi, cur + r (-0.08f, 0.12f));
            };

            // Light variation from DNA
            v["osc2_level"] = juce::jlimit (0.f, 0.7f, base.osc2Level + r (-0.05f, 0.15f));
            v["osc3_level"] = juce::jlimit (0.f, 0.45f, r (0.0f, 0.25f));
            v["osc1_table"] = juce::jlimit (0.f, 0.18f, base.osc1Table + r (-0.04f, 0.06f));
            v["osc2_table"] = juce::jlimit (0.f, 0.18f, r (0.02f, 0.14f));
            v["osc3_table"] = juce::jlimit (0.f, 0.18f, r (0.0f, 0.12f));
            v["osc1_semi"] = r (-5.f, 5.f); v["osc2_semi"] = r (-7.f, 12.f); v["osc3_semi"] = r (-5.f, 7.f);
            v["osc2_octave"] = (float) (rng.pick (3) - 1);
            v["fm_2to1"] = juce::jlimit (0.f, 0.35f, base.fm2to1 + r (-0.05f, 0.12f));
            v["fm_3to1"] = r (0.f, 0.18f);
            v["lfo_rate"] = r (0.15f, 8.f); v["lfo_amount"] = r (0.05f, 0.28f);
            v["lfo2_rate"] = r (0.1f, 4.f); v["lfo2_amount"] = r (0.0f, 0.2f);
            v["comp_mix"] = r (0.15f, 0.4f); v["comp_threshold"] = r (-20.f, -10.f);
            v["seq_on"] = 0.f; v["arp_on"] = 0.f;

            // Category morph on top of DNA
            switch (categoryIndex)
            {
                case 0: // ATMOSPHERE
                    v["amp_attack"] = r (0.35f, 1.1f); v["amp_release"] = r (0.9f, 2.8f);
                    v["reverb_mix"] = r (0.35f, 0.55f); v["chorus_mix"] = r (0.15f, 0.35f);
                    v["osc1_unison"] = juce::jmax (v["osc1_unison"], 3.f);
                    break;
                case 1: // LEAD
                    v["amp_attack"] = r (0.002f, 0.02f); v["filter_cutoff"] = r (2800.f, 7000.f);
                    v["delay_mix"] = r (0.15f, 0.32f); v["amp_sustain"] = r (0.4f, 0.7f);
                    break;
                case 2: // BASS
                    v["osc1_octave"] = -1.f; v["osc1_unison"] = 1.f;
                    v["filter_cutoff"] = r (120.f, 900.f); v["sub_level"] = r (0.25f, 0.5f);
                    v["amp_attack"] = r (0.002f, 0.02f); v["reverb_mix"] = r (0.05f, 0.15f);
                    break;
                case 3: // PULSE
                    v["amp_attack"] = r (0.001f, 0.012f); v["amp_decay"] = r (0.1f, 0.28f);
                    v["amp_sustain"] = r (0.05f, 0.25f); v["filter_env"] = r (0.5f, 0.9f);
                    break;
                case 4: // TEXTURE
                    v["noise_level"] = r (0.05f, 0.18f); v["osc1_warp"] = r (0.1f, 0.25f);
                    v["chorus_mix"] = r (0.15f, 0.35f); v["lfo_amount"] = r (0.1f, 0.3f);
                    break;
                case 5: // SEQUENCE (playable without auto-seq)
                    v["amp_attack"] = r (0.002f, 0.03f); v["amp_decay"] = r (0.12f, 0.35f);
                    v["amp_sustain"] = r (0.15f, 0.45f); v["delay_mix"] = r (0.12f, 0.28f);
                    break;
                case 6: // FM
                    v["fm_2to1"] = r (0.22f, 0.4f); v["fm_3to1"] = r (0.08f, 0.22f);
                    v["osc2_level"] = r (0.25f, 0.55f); v["osc2_octave"] = (float) (1 + rng.pick (2));
                    break;
                case 7: // FX
                    v["dist_mix"] = r (0.05f, 0.18f); v["filter_drive"] = r (0.08f, 0.22f);
                    v["phaser_mix"] = r (0.1f, 0.3f);
                    break;
                case 8: // PLUCK
                    v["amp_attack"] = r (0.001f, 0.006f); v["amp_sustain"] = r (0.0f, 0.12f);
                    v["amp_decay"] = r (0.12f, 0.4f); v["amp_release"] = r (0.1f, 0.35f);
                    v["filter_env"] = r (0.35f, 0.7f);
                    break;
                case 9: // DRONE
                    v["amp_attack"] = r (0.4f, 1.4f); v["amp_sustain"] = r (0.7f, 0.95f);
                    v["amp_release"] = r (1.2f, 3.5f); v["reverb_mix"] = r (0.35f, 0.55f);
                    v["lfo_rate"] = r (0.08f, 0.4f); v["lfo_amount"] = r (0.08f, 0.22f);
                    break;
                default: break;
            }

            // Safety caps (playable default levels)
            auto cap = [&] (const char* key, float maximum)
            {
                auto it = v.find (key);
                if (it != v.end()) it->second = juce::jmin (it->second, maximum);
            };
            cap ("osc1_table", 0.18f); cap ("osc2_table", 0.18f); cap ("osc3_table", 0.18f);
            cap ("osc1_warp", 0.25f); cap ("osc1_fold", 0.15f); cap ("osc1_drive", 0.15f);
            cap ("osc1_unison", 5.f); cap ("osc1_udet", 18.f);
            cap ("fm_2to1", 0.4f); cap ("filter_reso", 0.55f); cap ("filter_drive", 0.25f);
            cap ("lfo_amount", 0.35f); cap ("delay_mix", 0.35f); cap ("chorus_mix", 0.35f);
            cap ("reverb_mix", 0.55f); cap ("dist_mix", 0.2f); cap ("master_drive", 0.15f);
            cap ("master_gain", 0.78f);

            result.push_back ({ displayName, std::move (v) });
        }
    }
    return result;
}
} // namespace salek::giti
