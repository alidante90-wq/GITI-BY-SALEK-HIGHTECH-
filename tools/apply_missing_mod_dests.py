#!/usr/bin/env python3
from pathlib import Path
import sys
path = Path("Source/PluginProcessor_full_p2.inl")
if not path.exists():
    print("ERROR: run from repo root"); sys.exit(1)
text = path.read_text(encoding="utf-8", errors="replace")
if "Dest::Osc1Fold" in text and "Dest::Bassify" in text and "Dest::Amp" in text:
    print("Already patched."); sys.exit(0)

reps = [
(
'    synthEngine.setOsc1Fold(g("osc1_fold")); synthEngine.setOsc2Fold(g("osc2_fold")); synthEngine.setOsc3Fold(g("osc3_fold"));',
'''    synthEngine.setOsc1Fold(juce::jlimit(0.f,1.f, g("osc1_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Fold)*0.5f));
    synthEngine.setOsc2Fold(juce::jlimit(0.f,1.f, g("osc2_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Fold)*0.5f));
    synthEngine.setOsc3Fold(juce::jlimit(0.f,1.f, g("osc3_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Fold)*0.5f));'''
),
(
'    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));\n    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));',
'''    synthEngine.setFm2to1(juce::jlimit(0.f,1.f, g("fm_2to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Fm2to1)*0.5f));
    synthEngine.setFm3to1(juce::jlimit(0.f,1.f, g("fm_3to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Fm3to1)*0.5f));
    synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(juce::jlimit(0.f,1.f, g("pm_2to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Pitch)*0.35f));
    synthEngine.setRm2to1(g("rm_2to1"));
    synthEngine.setAm2to1(g("am_2to1"));'''
),
(
'    synthEngine.setNoiseLevel(g("noise_level"));\n    synthEngine.setSubLevel(g("sub_level"));\n    synthEngine.setGlide(g("glide"));',
'''    synthEngine.setNoiseLevel(juce::jlimit(0.f,1.f, g("noise_level")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Level)*0.25f));
    synthEngine.setSubLevel(juce::jlimit(0.f,1.f, g("sub_level")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Level)*0.25f));
    synthEngine.setGlide(juce::jlimit(0.f,1.f, g("glide")+std::abs(modMatrix.getModulation(salek::ModMatrix::Dest::Pitch))*0.15f));'''
),
(
'    const float bassify = apvts.getRawParameterValue("bassify")->load();',
'    const float bassify = juce::jlimit(0.f,1.f, apvts.getRawParameterValue("bassify")->load() + modMatrix.getModulation(salek::ModMatrix::Dest::Bassify)*0.5f);'
),
(
'    float gain = apvts.getRawParameterValue("master_gain")->load();\n    const float drive = apvts.getRawParameterValue("master_drive")->load();',
'''    float gain = juce::jlimit(0.f, 2.f, apvts.getRawParameterValue("master_gain")->load()
        + modMatrix.getModulation(salek::ModMatrix::Dest::Amp)*0.35f);
    const float drive = juce::jlimit(0.f, 1.f, apvts.getRawParameterValue("master_drive")->load()
        + modMatrix.getModulation(salek::ModMatrix::Dest::DistDrive)*0.25f);'''
),
]
for old, new in reps:
    if old not in text:
        print("ERROR missing block:\n", old[:100]); sys.exit(1)
    text = text.replace(old, new)
path.write_text(text, encoding="utf-8")
print("OK patched", path, "size", len(text))
print('git add Source/PluginProcessor_full_p2.inl')
print('git commit -m "SHAE: missing LFO dests Fold/FM/Pitch/Sub/Noise/Bassify/Amp"')
print('git push origin giti-by-salek-hightech')
