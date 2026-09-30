#pragma once
/**
 * GitiSonicDNA — branch → SHAE seed for factory presets and identity select
 * Works with salek::giti::identities (GitiIdentityBank.h)
 */
#include "GITI/GitiIdentityBank.h"
#include <cmath>
#include <cstring>
#include <map>

namespace salek {
namespace giti {

enum class BranchBias : int { Breath = 0, Strings, Voice, Pulse, EarthWater };

inline BranchBias branchBiasFromString (const char* b) noexcept
{
    if (b == nullptr) return BranchBias::Breath;
    if (std::strstr (b, "Breath"))  return BranchBias::Breath;
    if (std::strstr (b, "Strings")) return BranchBias::Strings;
    if (std::strstr (b, "Voice"))   return BranchBias::Voice;
    if (std::strstr (b, "Pulse"))   return BranchBias::Pulse;
    if (std::strstr (b, "Earth"))   return BranchBias::EarthWater;
    return BranchBias::Breath;
}

struct SonicSeed
{
    float osc1Level = 0.85f, osc2Level = 0.25f, osc3Level = 0.0f;
    float subLevel = 0.15f, noiseLevel = 0.05f;
    float osc1Table = 0.12f, osc1Warp = 0.12f, osc1Fold = 0.06f, osc1Drive = 0.08f;
    float osc1Unison = 2.f, osc1Detune = 6.f, osc1Spread = 0.45f;
    float filterCutoff = 4200.f, filterReso = 0.22f, filterEnv = 0.4f, filterDrive = 0.1f;
    float ampAttack = 0.02f, ampDecay = 0.25f, ampSustain = 0.6f, ampRelease = 0.35f;
    float reverbMix = 0.22f, delayMix = 0.12f, chorusMix = 0.1f;
    float masterDrive = 0.08f, masterGain = 0.74f;
    float fm2to1 = 0.1f;
};

/** index0based 0..49 */
inline SonicSeed seedForIndex (int index0based) noexcept
{
    SonicSeed s;
    if (index0based < 0 || index0based >= 50) return s;
    const auto& id = identities[(size_t) index0based];
    const BranchBias bias = branchBiasFromString (id.branch);
    const float t = (float) (index0based + 1) / 50.f;
    const float wobble = 0.03f * std::sin (t * 12.566f);

    switch (bias)
    {
        case BranchBias::Breath:
            s.noiseLevel = 0.08f + 0.06f * t; s.osc1Level = 0.72f; s.osc2Level = 0.22f; s.subLevel = 0.06f;
            s.ampAttack = 0.04f + 0.05f * t; s.ampRelease = 0.4f + 0.3f * t;
            s.filterCutoff = 3600.f + 2000.f * t; s.filterReso = 0.16f;
            s.osc1Warp = 0.1f + wobble; s.reverbMix = 0.28f + 0.12f * t; s.chorusMix = 0.12f;
            s.osc1Unison = 2.f; s.osc1Detune = 5.f + 4.f * t; s.osc1Spread = 0.5f;
            break;
        case BranchBias::Strings:
            s.noiseLevel = 0.02f; s.osc1Level = 0.8f; s.osc2Level = 0.32f;
            s.osc1Unison = 2.f + (float) (index0based % 2); s.osc1Detune = 6.f + 6.f * t; s.osc1Spread = 0.55f + 0.2f * t;
            s.ampAttack = 0.008f + 0.015f * t; s.ampDecay = 0.3f + 0.25f * t; s.ampSustain = 0.4f; s.ampRelease = 0.45f + 0.25f * t;
            s.filterCutoff = 3000.f + 2500.f * t; s.filterReso = 0.24f;
            s.osc1Fold = 0.06f + 0.08f * t; s.osc1Warp = 0.12f + 0.1f * t;
            s.reverbMix = 0.28f + 0.12f * t; s.chorusMix = 0.18f;
            break;
        case BranchBias::Voice:
            s.noiseLevel = 0.04f; s.osc1Level = 0.78f; s.osc2Level = 0.3f;
            s.ampAttack = 0.05f + 0.06f * t; s.ampSustain = 0.7f; s.ampRelease = 0.5f + 0.25f * t;
            s.filterCutoff = 2600.f + 1800.f * t; s.filterReso = 0.28f;
            s.osc1Warp = 0.15f + 0.12f * t; s.reverbMix = 0.32f + 0.12f * t; s.chorusMix = 0.14f;
            s.fm2to1 = 0.08f + 0.1f * t;
            break;
        case BranchBias::Pulse:
            s.noiseLevel = 0.03f; s.osc1Level = 0.88f; s.subLevel = 0.28f + 0.12f * t; s.osc2Level = 0.12f;
            s.ampAttack = 0.002f + 0.006f * t; s.ampDecay = 0.14f + 0.18f * t;
            s.ampSustain = 0.1f + 0.15f * t; s.ampRelease = 0.1f + 0.12f * t;
            s.filterCutoff = 700.f + 1600.f * t; s.filterReso = 0.38f + 0.15f * t; s.filterEnv = 0.65f + 0.2f * t;
            s.osc1Drive = 0.12f + 0.12f * t; s.osc1Fold = 0.1f + 0.12f * t;
            s.masterDrive = 0.1f + 0.08f * t; s.reverbMix = 0.1f; s.osc1Unison = 1.f;
            s.fm2to1 = 0.15f + 0.2f * t;
            break;
        case BranchBias::EarthWater:
            s.noiseLevel = 0.07f + 0.05f * t; s.osc1Level = 0.74f; s.osc2Level = 0.35f; s.subLevel = 0.18f;
            s.ampAttack = 0.04f + 0.08f * t; s.ampDecay = 0.45f + 0.3f * t;
            s.ampSustain = 0.5f + 0.15f * t; s.ampRelease = 0.8f + 0.5f * t;
            s.filterCutoff = 2200.f + 2200.f * t; s.filterReso = 0.3f;
            s.osc1Warp = 0.1f; s.reverbMix = 0.38f + 0.15f * t; s.chorusMix = 0.14f;
            s.osc1Unison = 3.f; s.osc1Detune = 7.f; s.osc1Spread = 0.65f;
            break;
    }

    if (index0based == 49) // 050 collective
    {
        s.osc1Unison = 4.f; s.osc1Detune = 12.f; s.osc1Spread = 0.85f;
        s.osc2Level = 0.45f; s.reverbMix = 0.4f; s.ampSustain = 0.65f; s.filterCutoff = 4000.f;
    }

    s.osc1Table = juce::jlimit (0.f, 0.18f, s.osc1Table + wobble * 0.4f);
    s.filterCutoff = juce::jlimit (80.f, 12000.f, s.filterCutoff * (1.f + wobble * 0.25f));
    return s;
}

inline void applySeedToMap (const SonicSeed& s, std::map<juce::String, float>& v)
{
    v["osc1_level"] = s.osc1Level; v["osc2_level"] = s.osc2Level; v["osc3_level"] = s.osc3Level;
    v["sub_level"] = s.subLevel; v["noise_level"] = s.noiseLevel;
    v["osc1_table"] = s.osc1Table; v["osc1_warp"] = s.osc1Warp; v["osc1_fold"] = s.osc1Fold; v["osc1_drive"] = s.osc1Drive;
    v["osc1_unison"] = s.osc1Unison; v["osc1_udet"] = s.osc1Detune; v["osc1_uspread"] = s.osc1Spread;
    v["filter_cutoff"] = s.filterCutoff; v["filter_reso"] = s.filterReso; v["filter_env"] = s.filterEnv; v["filter_drive"] = s.filterDrive;
    v["amp_attack"] = s.ampAttack; v["amp_decay"] = s.ampDecay; v["amp_sustain"] = s.ampSustain; v["amp_release"] = s.ampRelease;
    v["reverb_mix"] = s.reverbMix; v["delay_mix"] = s.delayMix; v["chorus_mix"] = s.chorusMix;
    v["master_drive"] = s.masterDrive; v["master_gain"] = s.masterGain;
    v["fm_2to1"] = s.fm2to1;
}

} // namespace giti
} // namespace salek
