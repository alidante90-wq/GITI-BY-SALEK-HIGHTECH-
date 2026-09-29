#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"
#include <array>
#include <limits>

namespace salek
{

class SynthEngine
{
public:
    SynthEngine();
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);
    void allNotesOff();

    void setOsc1TablePos (float v); void setOsc2TablePos (float v); void setOsc3TablePos (float v);
    void setOsc1Level (float v); void setOsc2Level (float v); void setOsc3Level (float v);
    void setOsc1Pan (float v); void setOsc2Pan (float v); void setOsc3Pan (float v);
    void setOsc1Octave (int v); void setOsc2Octave (int v); void setOsc3Octave (int v);
    void setOsc1Semi (int v); void setOsc2Semi (int v); void setOsc3Semi (int v);
    void setOsc1CoarsePitch (int v); void setOsc2CoarsePitch (int v); void setOsc3CoarsePitch (int v);
    void setOsc1Fine (float v); void setOsc2Fine (float v); void setOsc3Fine (float v);
    void setOsc1Detune (float v); void setOsc2Detune (float v); void setOsc3Detune (float v);
    void setOsc1Phase (float v); void setOsc2Phase (float v); void setOsc3Phase (float v);
    void setOsc1Rand (float v); void setOsc2Rand (float v); void setOsc3Rand (float v);
    void setOsc1Warp (float v); void setOsc2Warp (float v); void setOsc3Warp (float v);
    void setOsc1Fold (float v); void setOsc2Fold (float v); void setOsc3Fold (float v);
    void setOsc1Drive (float v); void setOsc2Drive (float v); void setOsc3Drive (float v);

    void setFilterCutoff (float v); void setFilterResonance (float v);
    void setFilterDrive (float v); void setFilterMode (int v); void setFilterEnvAmt (float v);
    void setNoiseLevel (float v); void setSubLevel (float v); void setFilterRoute (int v); void setGlide (float v);
    void setOsc1Unison (int v, float d, float s);
    void setOsc2Unison (int v, float d, float s);
    void setOsc3Unison (int v, float d, float s);

    void setAmpAttack (float v); void setAmpDecay (float v);
    void setAmpSustain (float v); void setAmpRelease (float v);

    void setLfoRate (float v); void setLfoAmount (float v); void setLfoWave (int v);

    void setFm2to1 (float v); void setFm3to1 (float v); void setFm3to2 (float v);
    void setPm2to1 (float v); void setPm3to1 (float v);
    void setRm2to1 (float v); void setAm2to1 (float v);

    void setUnison (int v); void setUnisonDetune (float v); void setUnisonSpread (float v);

    void setScaleMode (int m); void setKoronCents (float c);

    void labRegenerate();
    void labProcessFrames (float fold, float drive);
    void labMorph (float t);

private:
    static constexpr int maxVoices = 16;
    juce::Synthesiser synth;
    // Most synth controls stay fixed for many audio blocks. Avoid pushing the
    // same values through every voice at every block boundary.
    std::array<float, 64> lastFloatParameters;
    std::array<int, 24> lastIntParameters;

    bool updateIfChanged (std::array<float, 64>& values, size_t index, float value) noexcept
    {
        if (values[index] == value) return false;
        values[index] = value;
        return true;
    }
    bool updateIfChanged (std::array<int, 24>& values, size_t index, int value) noexcept
    {
        if (values[index] == value) return false;
        values[index] = value;
        return true;
    }

    template <typename Fn>
    void forEachVoice (Fn&& fn)
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
            // SynthEngine exclusively installs SynthVoice instances above.
            fn (static_cast<SynthVoice&> (*synth.getVoice (i)));
    }
};

} // namespace salek
