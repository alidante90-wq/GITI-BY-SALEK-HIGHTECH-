#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"
#include "ModulationMatrix.h"

class SalekSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SalekVoice : public juce::SynthesiserVoice
{
public:
    SalekVoice (juce::AudioProcessorValueTreeState& apvts, ModulationMatrix& matrix);
    ~SalekVoice() override = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock);
    void markParamsDirty() noexcept { paramsDirty = true; }

private:
    void updateParameters();
    void applyModulation (float* destOffsets);

    float noteHz = 440.0f;
    float velocity = 0.0f;
    int currentNote = 60;

    juce::AudioProcessorValueTreeState& apvts;
    ModulationMatrix& modMatrix;

    Oscillator osc1, osc2, osc3;
    Oscillator subOsc;
    // Real unison: dedicated oscillators for extra osc1 voices (center is osc1)
    static constexpr int maxUnison = 5;
    std::array<Oscillator, maxUnison - 1> unisonOscs {};

    MultiFilter filter;
    Envelope ampEnv, filterEnv, modEnv;
    LFO lfo1, lfo2, lfo3, lfo4;

    juce::Random noiseRandom;
    float noiseLevel = 0.0f;

    std::array<float, maxUnison> unisonDetune {};
    int unisonVoices = 1;
    float unisonDetuneAmt = 0.12f;
    float unisonSpread = 0.5f; // 0 mono .. 1 full L/R

    double sr = 44100.0;
    bool isPrepared = false;
    bool paramsDirty = true;

    float baseOsc1Freq = 440.f, baseOsc2Freq = 440.f, baseOsc3Freq = 440.f, baseSubFreq = 220.f;
    float baseOsc1Level = 0.8f, baseOsc2Level = 0.f, baseOsc3Level = 0.f;
    float baseSubLevel = 0.f, baseNoiseLevel = 0.f;
    float baseOsc1WT = 0.f, baseOsc1Morph = 0.f, baseOsc1Warp = 0.f, baseOsc1FM = 0.f, baseOsc1AM = 0.f, baseOsc1RM = 0.f;
    float baseOsc2WT = 0.f, baseOsc2Morph = 0.f, baseOsc2Warp = 0.f, baseOsc2FM = 0.f, baseOsc2AM = 0.f, baseOsc2RM = 0.f;
    float baseOsc3WT = 0.f, baseOsc3Morph = 0.f, baseOsc3Warp = 0.f, baseOsc3FM = 0.f, baseOsc3AM = 0.f, baseOsc3RM = 0.f;
    float baseCutoff = 8000.f, baseRes = 0.2f, baseDrive = 0.f, baseKeytrack = 0.f;

    float modSources[ModulationMatrix::numSources] {};
    float modDests[ModulationMatrix::numDests] {};
};
