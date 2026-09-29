#pragma once
#include <JuceHeader.h>
#include "Synth/SynthEngine.h"
#include "FX/SimpleDelay.h"
#include "FX/SimpleChorus.h"
#include "FX/SimpleReverb.h"
#include "FX/SimpleCompressor.h"
#include "FX/SimpleEQ.h"
#include "FX/SimpleSpatial.h"
#include "FX/SimplePhaser.h"
#include "FX/SimpleDistortion.h"
#include "FX/MagicEngine.h"
#include "Oscillators/GranularOscillator.h"
#include "Sequencer/Arpeggiator.h"
#include "Sequencer/StepSequencer.h"
#include "Modulation/ModMatrix.h"
#include "Modulation/LFO.h"
#include "UI/VisualFifo.h"
#include "SHAE/SHAECore.h"
#include <map>
#include <vector>
#include <atomic>
#include <chrono>

class SalekHightechAudioProcessor : public juce::AudioProcessor
{
public:
    SalekHightechAudioProcessor();
    ~SalekHightechAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return (int) factoryPresets.size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    juce::String getProgramDescription (int index) const;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    float getOutputPeak() const { return outputPeak.load(); }
    float getAudioLoadPercent() const noexcept { return audioLoadPercent.load (std::memory_order_relaxed); }
    float getAudioPeakLoadPercent() const noexcept { return audioPeakLoadPercent.load (std::memory_order_relaxed); }
    float getAudioCallbackMs() const noexcept { return audioCallbackMs.load (std::memory_order_relaxed); }
    VisualFifo& getVisualFifo() { return visualFifo; }
    salek::StepSequencer& getStepSequencer() { return stepSequencer; }
    salek::Arpeggiator& getArpeggiator() { return arpeggiator; }
    salek::ModMatrix& getModMatrix() { return modMatrix; }
    salek::LFO& getLfo1() { return lfo1; }
    salek::LFO& getLfo2() { return lfo2; }
    salek::LFO& getLfo3() { return lfo3; }
    salek::MagicEngine& getMagic() { return magic; }
    salek::SimpleCompressor& getCompressor() { return compressor; }
    salek::SimpleReverb& getReverb() { return reverb; }
    salek::GranularOscillator& getGranular() { return granular; }

    juce::StringArray getPresetNames() const;
    int saveCurrentAsUserPreset (const juce::String& name);
    void loadUserPresetsFromDisk();
    void saveUserPresetsToDisk();
    bool exportCurrentPresetToFile (const juce::File& file, const juce::String& displayName);
    int importPresetFromFile (const juce::File& file);
    bool exportUserBankToFile (const juce::File& file);
    int importUserBankFromFile (const juce::File& file);
    bool deleteUserPreset (int programIndex);
    int getNumUserPresets() const;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyParamsToEngine();
    void initFactoryPresets();
    void loadFactoryPreset (int index);

    juce::AudioProcessorValueTreeState apvts;
    salek::SynthEngine synthEngine;
    salek::SimpleDelay delay;
    salek::SimpleChorus chorus;
    salek::SimpleReverb reverb;
    salek::SimpleCompressor compressor;
    salek::SimpleEQ eq;
    salek::SimpleSpatial spatial;
    salek::SimplePhaser phaser;
    salek::SimpleDistortion distortion;
    salek::MagicEngine magic;
    salek::GranularOscillator granular;
    salek::Arpeggiator arpeggiator;
    salek::StepSequencer stepSequencer;
    salek::ModMatrix modMatrix;
    salek::LFO lfo1, lfo2, lfo3;
    juce::MidiKeyboardState keyboardState;
    VisualFifo visualFifo;
    std::atomic<float> outputPeak { 0.f };
    std::atomic<float> audioLoadPercent { 0.f }, audioPeakLoadPercent { 0.f }, audioCallbackMs { 0.f };
    shae::SafetyStage shaeSafety;

    struct FactoryPreset { juce::String name; std::map<juce::String, float> values; };
    std::vector<FactoryPreset> factoryPresets;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessor)
};
