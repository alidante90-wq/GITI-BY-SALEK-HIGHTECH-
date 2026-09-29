#pragma once
#include "GitiIdentityBank.h"
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
    int pick (int n) noexcept { return static_cast<int> (next() % static_cast<std::uint32_t> (n)); }
    std::uint32_t state;
};

inline std::vector<Preset> makePresets()
{
    static constexpr const char* categories[] = { "ATMOSPHERE", "LEAD", "BASS", "PULSE", "TEXTURE", "SEQUENCE", "FM", "FX", "PLUCK", "DRONE" };
    static constexpr const char* roles[] = { "Origin", "Signal", "Motion", "Shadow", "Halo", "Memory", "Vector", "Ritual", "Bloom", "Terminal" };
    static constexpr const char* bankNames[5][10] = {
        { "Air", "Reed", "Inhale", "Horizon", "Wind", "Lung", "Whisper", "Silver", "Dawn", "Sky" },
        { "Bow", "String", "Cedar", "Resonance", "Pluck", "Thread", "Tremolo", "Harp", "Lantern", "Cathedral" },
        { "Vowel", "Throat", "Choir", "Hum", "Name", "Lullaby", "Call", "Bloom", "Tongue", "Overtone" },
        { "Heart", "Frame", "Drum", "Impact", "Step", "Ember", "Thunder", "Ritual", "Sub", "Meter" },
        { "Rain", "Stone", "River", "Root", "Glass", "Cave", "Tide", "Seed", "Monsoon", "Earth" }
    };
    static constexpr int waveFrames[] = { 0, 1, 2, 3, 6, 8, 11, 13, 16, 19, 22, 29, 40, 64, 96 };
    std::vector<Preset> result;
    result.reserve (identities.size() * 50);

    for (std::size_t gi = 0; gi < identities.size(); ++gi)
    {
        const auto& id = identities[gi];
        int branch = 0;
        if (std::string (id.branch) == "Strings") branch = 1;
        else if (std::string (id.branch) == "Voice") branch = 2;
        else if (std::string (id.branch) == "Pulse") branch = 3;
        else if (std::string (id.branch) == "Earth & Water") branch = 4;
        const float identityBias = 0.35f + static_cast<float> (hash32 (id.voice) % 100u) / 200.0f;
        for (int pi = 0; pi < 50; ++pi)
        {
            const auto seed = hash32 ((std::string (id.number) + "|" + std::to_string (pi) + "|GITI-SONIC-CORE").c_str());
            Random rng (seed);
            const char* bank = bankNames[branch][rng.pick (10)];
            const char* role = roles[rng.pick (10)];
            const int categoryIndex = pi % 10;
            const char* category = categories[categoryIndex];
            const auto suffix = juce::String (pi + 1).paddedLeft ('0', 2);
            const auto displayName = juce::String ("GITI ") + id.number + "/" + id.name + " · " + bank + " " + role + " — " + category + " " + suffix;
            std::map<juce::String, float> v;
            auto put = [&] (const char* key, float value) { v.emplace (key, value); };
            auto r = [&] (float lo, float hi) { return rng.range (lo, hi); };
            auto level = [&] (float lo, float hi) { return juce::jlimit (0.0f, 1.0f, r (lo, hi)); };
            const int wf1 = waveFrames[rng.pick (15)], wf2 = waveFrames[rng.pick (15)], wf3 = waveFrames[rng.pick (15)];
            const float timbreBias = (static_cast<float> (branch) - 2.0f) * 0.045f;

            // Identity-weighted, complete native SHAE patch state. Every preset
            // varies oscillator, modulation, envelope, filter, motion and effects.
            put ("osc1_level", juce::jlimit (0.25f, 0.96f, r (0.48f, 0.88f) * (0.78f + identityBias * 0.35f)));
            put ("osc2_level", level (0.12f, 0.66f)); put ("osc3_level", level (0.06f, 0.48f));
            put ("osc1_table", wf1 / 127.0f); put ("osc2_table", wf2 / 127.0f); put ("osc3_table", wf3 / 127.0f);
            put ("osc1_octave", branch == 3 ? -1.0f : 0.0f); put ("osc2_octave", r (-1.0f, 1.01f)); put ("osc3_octave", branch == 3 ? -2.0f : r (-1.0f, 1.01f));
            put ("osc1_semi", r (-7.0f, 7.01f)); put ("osc2_semi", r (-12.0f, 12.01f)); put ("osc3_semi", r (-5.0f, 5.01f));
            put ("osc1_fine", r (-18.0f, 18.0f)); put ("osc2_fine", r (-24.0f, 24.0f)); put ("osc3_fine", r (-32.0f, 32.0f));
            put ("osc1_phase", r (0.0f, 1.0f)); put ("osc2_phase", r (0.0f, 1.0f)); put ("osc3_phase", r (0.0f, 1.0f));
            put ("osc1_rand", r (0.0f, 0.35f)); put ("osc2_rand", r (0.0f, 0.5f)); put ("osc3_rand", r (0.0f, 0.6f));
            put ("osc1_warp", level (0.04f, 0.8f)); put ("osc2_warp", level (0.0f, 0.85f)); put ("osc3_warp", level (0.0f, 0.75f));
            put ("osc1_fold", level (0.0f, 0.48f)); put ("osc2_fold", level (0.0f, 0.55f)); put ("osc3_fold", level (0.0f, 0.5f));
            put ("osc1_drive", level (0.0f, 0.48f)); put ("osc2_drive", level (0.0f, 0.4f)); put ("osc3_drive", level (0.0f, 0.35f));
            put ("osc1_unison", static_cast<float> (1 + rng.pick (7))); put ("osc2_unison", static_cast<float> (1 + rng.pick (5))); put ("osc3_unison", static_cast<float> (1 + rng.pick (4)));
            put ("osc1_udet", r (2.0f, 24.0f)); put ("osc2_udet", r (1.0f, 18.0f)); put ("osc3_udet", r (0.0f, 14.0f));
            put ("osc1_uspread", r (0.25f, 1.0f)); put ("osc2_uspread", r (0.15f, 0.95f)); put ("osc3_uspread", r (0.05f, 0.85f));
            put ("fm_2to1", level (0.0f, 0.65f)); put ("fm_3to1", level (0.0f, 0.5f)); put ("fm_3to2", level (0.0f, 0.45f));
            put ("pm_2to1", level (0.0f, 0.55f)); put ("am_2to1", level (0.0f, 0.55f)); put ("rm_2to1", level (0.0f, 0.4f));
            const float cutoffNorm = juce::jlimit (0.18f, 0.95f, r (0.24f, 0.88f) + timbreBias);
            put ("filter_cutoff", 25.0f * std::pow (720.0f, cutoffNorm)); put ("filter_reso", level (0.04f, 0.76f)); put ("filter_drive", level (0.0f, 0.55f)); put ("filter_env", level (0.0f, 0.9f));
            put ("filter_mode", static_cast<float> (rng.pick (categoryIndex == 2 ? 4 : 12)));
            put ("amp_attack", branch == 3 ? r (0.001f, 0.08f) : r (0.005f, 0.42f)); put ("amp_decay", r (0.08f, 0.85f)); put ("amp_sustain", r (0.28f, 0.95f)); put ("amp_release", branch == 3 ? r (0.06f, 0.6f) : r (0.18f, 2.2f));
            put ("lfo_rate", r (0.12f, 14.0f)); put ("lfo_amount", level (0.0f, 0.72f)); put ("lfo_wave", static_cast<float> (rng.pick (12)));
            put ("lfo2_rate", r (0.08f, 9.0f)); put ("lfo2_amount", level (0.0f, 0.62f)); put ("lfo2_wave", static_cast<float> (rng.pick (12)));
            put ("lfo3_rate", r (0.05f, 5.0f)); put ("lfo3_amount", level (0.0f, 0.42f)); put ("lfo3_wave", static_cast<float> (rng.pick (12)));
            put ("delay_mix", level (0.0f, 0.68f)); put ("delay_time", r (80.0f, 720.0f)); put ("delay_fb", r (0.08f, 0.68f)); put ("delay_tone", r (0.3f, 0.95f)); put ("delay_mode", static_cast<float> (rng.pick (3)));
            put ("chorus_mix", level (0.0f, 0.68f)); put ("chorus_rate", r (0.08f, 2.8f)); put ("chorus_depth", r (0.15f, 0.9f));
            put ("reverb_mix", level (0.0f, branch == 4 ? 0.85f : 0.68f)); put ("reverb_size", r (0.2f, 0.95f)); put ("reverb_decay", r (0.25f, 0.92f)); put ("reverb_damp", r (0.15f, 0.85f)); put ("reverb_mode", static_cast<float> (rng.pick (5)));
            put ("dist_mix", level (0.0f, 0.62f)); put ("dist_drive", level (0.0f, 0.68f)); put ("dist_crush", level (0.0f, 0.36f)); put ("dist_mode", static_cast<float> (rng.pick (6)));
            put ("master_drive", level (0.0f, 0.45f)); put ("master_gain", r (0.68f, 0.9f)); put ("comp_mix", r (0.15f, 0.72f)); put ("comp_threshold", r (-24.0f, -8.0f)); put ("comp_ratio", r (2.0f, 8.0f));
            put ("spatial_size", r (0.25f, 0.9f)); put ("spatial_azim", r (-45.0f, 45.0f));
            put ("macro1", r (0.2f, 0.95f)); put ("macro2", r (0.1f, 0.95f)); put ("macro3", r (0.15f, 0.92f)); put ("macro4", r (0.2f, 0.95f));
            put ("seq_on", (categoryIndex == 5 || categoryIndex == 3) && rng.unit() > 0.35f ? 1.0f : 0.0f); put ("seq_rate", static_cast<float> (1 + rng.pick (8))); put ("seq_length", static_cast<float> (8 + rng.pick (9))); put ("seq_swing", r (0.0f, 0.3f)); put ("seq_gate", r (0.35f, 0.95f));
            put ("arp_on", (categoryIndex == 5 || categoryIndex == 1) && rng.unit() > 0.55f ? 1.0f : 0.0f); put ("arp_rate", static_cast<float> (1 + rng.pick (16))); put ("arp_octaves", static_cast<float> (1 + rng.pick (3))); put ("arp_gate", r (0.35f, 0.95f)); put ("arp_swing", r (0.0f, 0.25f));

            if (categoryIndex == 0 || categoryIndex == 9) { v["amp_attack"] = r (0.28f, 1.2f); v["amp_release"] = r (0.7f, 3.2f); v["reverb_mix"] = r (0.38f, 0.78f); }
            if (categoryIndex == 1) { v["amp_attack"] = r (0.001f, 0.025f); v["filter_cutoff"] = r (2800.f, 9500.f); v["delay_mix"] = r (0.18f, 0.55f); }
            if (categoryIndex == 2) { v["osc1_octave"] = branch == 3 ? -2.f : -1.f; v["osc1_unison"] = 1.f; v["filter_cutoff"] = r (80.f, 900.f); v["sub_level"] = r (0.22f, 0.55f); }
            if (categoryIndex == 3 || categoryIndex == 5) { v["arp_on"] = 0.f; v["seq_on"] = 1.f; v["amp_attack"] = r (0.001f, 0.04f); }
            if (categoryIndex == 6) { v["fm_2to1"] = r (0.35f, 0.92f); v["fm_3to1"] = r (0.12f, 0.65f); }
            if (categoryIndex == 7) { v["dist_mix"] = r (0.25f, 0.72f); v["phaser_mix"] = r (0.18f, 0.65f); }
            if (categoryIndex == 8) { v["amp_attack"] = r (0.001f, 0.008f); v["amp_sustain"] = r (0.0f, 0.18f); v["amp_decay"] = r (0.12f, 0.4f); }
            result.push_back ({ displayName, std::move (v) });
        }
    }
    return result;
}
} // namespace salek::giti