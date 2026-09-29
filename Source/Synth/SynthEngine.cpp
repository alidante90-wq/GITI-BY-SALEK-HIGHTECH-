#include "SynthEngine.h"

namespace salek
{

SynthEngine::SynthEngine()
{
    lastFloatParameters.fill (std::numeric_limits<float>::quiet_NaN());
    lastIntParameters.fill (std::numeric_limits<int>::min());
    for (int i = 0; i < maxVoices; ++i)
        synth.addVoice (new SynthVoice());
    synth.addSound (new SynthSound());
}

void SynthEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    forEachVoice ([&] (SynthVoice& v) { v.prepareToPlay (sampleRate, samplesPerBlock); });
}

void SynthEngine::releaseResources() {}

void SynthEngine::allNotesOff()
{
    synth.allNotesOff (0, true);
}

void SynthEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
}

#define SALEK_CACHED_FLOAT(method, index, voiceMethod) \
    void SynthEngine::method (float v) { \
        if (! updateIfChanged (lastFloatParameters, index, v)) return; \
        forEachVoice ([&] (SynthVoice& voice) { voice.voiceMethod (v); }); \
    }
#define SALEK_CACHED_INT(method, index, voiceMethod) \
    void SynthEngine::method (int v) { \
        if (! updateIfChanged (lastIntParameters, index, v)) return; \
        forEachVoice ([&] (SynthVoice& voice) { voice.voiceMethod (v); }); \
    }

SALEK_CACHED_FLOAT (setOsc1TablePos, 0, setOsc1TablePos)
SALEK_CACHED_FLOAT (setOsc2TablePos, 1, setOsc2TablePos)
SALEK_CACHED_FLOAT (setOsc3TablePos, 2, setOsc3TablePos)
SALEK_CACHED_FLOAT (setOsc1Level, 3, setOsc1Level)
SALEK_CACHED_FLOAT (setOsc2Level, 4, setOsc2Level)
SALEK_CACHED_FLOAT (setOsc3Level, 5, setOsc3Level)
SALEK_CACHED_FLOAT (setOsc1Pan, 6, setOsc1Pan)
SALEK_CACHED_FLOAT (setOsc2Pan, 7, setOsc2Pan)
SALEK_CACHED_FLOAT (setOsc3Pan, 8, setOsc3Pan)
SALEK_CACHED_INT (setOsc1Octave, 0, setOsc1Octave)
SALEK_CACHED_INT (setOsc2Octave, 1, setOsc2Octave)
SALEK_CACHED_INT (setOsc3Octave, 2, setOsc3Octave)
SALEK_CACHED_INT (setOsc1Semi, 3, setOsc1Semi)
SALEK_CACHED_INT (setOsc2Semi, 4, setOsc2Semi)
SALEK_CACHED_INT (setOsc3Semi, 5, setOsc3Semi)
SALEK_CACHED_INT (setOsc1CoarsePitch, 6, setOsc1CoarsePitch)
SALEK_CACHED_INT (setOsc2CoarsePitch, 7, setOsc2CoarsePitch)
SALEK_CACHED_INT (setOsc3CoarsePitch, 8, setOsc3CoarsePitch)
SALEK_CACHED_FLOAT (setOsc1Fine, 9, setOsc1Fine)
SALEK_CACHED_FLOAT (setOsc2Fine, 10, setOsc2Fine)
SALEK_CACHED_FLOAT (setOsc3Fine, 11, setOsc3Fine)
SALEK_CACHED_FLOAT (setOsc1Detune, 12, setOsc1Detune)
SALEK_CACHED_FLOAT (setOsc2Detune, 13, setOsc2Detune)
SALEK_CACHED_FLOAT (setOsc3Detune, 14, setOsc3Detune)
SALEK_CACHED_FLOAT (setOsc1Warp, 15, setOsc1Warp)
SALEK_CACHED_FLOAT (setOsc2Warp, 16, setOsc2Warp)
SALEK_CACHED_FLOAT (setOsc3Warp, 17, setOsc3Warp)
SALEK_CACHED_FLOAT (setOsc1Fold, 18, setOsc1Fold)
SALEK_CACHED_FLOAT (setOsc2Fold, 19, setOsc2Fold)
SALEK_CACHED_FLOAT (setOsc3Fold, 20, setOsc3Fold)
SALEK_CACHED_FLOAT (setOsc1Drive, 21, setOsc1Drive)
SALEK_CACHED_FLOAT (setOsc2Drive, 22, setOsc2Drive)
SALEK_CACHED_FLOAT (setOsc3Drive, 23, setOsc3Drive)
SALEK_CACHED_FLOAT (setOsc1Phase, 24, setOsc1Phase)
SALEK_CACHED_FLOAT (setOsc2Phase, 25, setOsc2Phase)
SALEK_CACHED_FLOAT (setOsc3Phase, 26, setOsc3Phase)
SALEK_CACHED_FLOAT (setOsc1Rand, 27, setOsc1Rand)
SALEK_CACHED_FLOAT (setOsc2Rand, 28, setOsc2Rand)
SALEK_CACHED_FLOAT (setOsc3Rand, 29, setOsc3Rand)
SALEK_CACHED_FLOAT (setFm2to1, 30, setFm2to1)
SALEK_CACHED_FLOAT (setFm3to1, 31, setFm3to1)
SALEK_CACHED_FLOAT (setFm3to2, 32, setFm3to2)
SALEK_CACHED_FLOAT (setPm2to1, 33, setPm2to1)
SALEK_CACHED_FLOAT (setPm3to1, 34, setPm3to1)
SALEK_CACHED_FLOAT (setAm2to1, 35, setAm2to1)
SALEK_CACHED_FLOAT (setRm2to1, 36, setRm2to1)
SALEK_CACHED_FLOAT (setFilterCutoff, 37, setFilterBaseCutoff)
SALEK_CACHED_FLOAT (setFilterResonance, 38, setFilterResonance)
SALEK_CACHED_FLOAT (setFilterDrive, 39, setFilterDrive)
SALEK_CACHED_INT (setFilterMode, 9, setFilterMode)
SALEK_CACHED_FLOAT (setFilterEnvAmt, 40, setFilterEnvAmt)
SALEK_CACHED_FLOAT (setNoiseLevel, 41, setNoiseLevel)
SALEK_CACHED_FLOAT (setSubLevel, 42, setSubLevel)
SALEK_CACHED_INT (setFilterRoute, 10, setFilterRoute)
SALEK_CACHED_FLOAT (setGlide, 43, setGlide)
SALEK_CACHED_FLOAT (setAmpAttack, 44, setAmpAttack)
SALEK_CACHED_FLOAT (setAmpDecay, 45, setAmpDecay)
SALEK_CACHED_FLOAT (setAmpSustain, 46, setAmpSustain)
SALEK_CACHED_FLOAT (setAmpRelease, 47, setAmpRelease)
SALEK_CACHED_FLOAT (setLfoRate, 48, setLfoRate)
SALEK_CACHED_FLOAT (setLfoAmount, 49, setLfoAmount)
SALEK_CACHED_INT (setLfoWave, 11, setLfoWave)
SALEK_CACHED_INT (setUnison, 12, setUnison)
SALEK_CACHED_FLOAT (setUnisonDetune, 50, setUnisonDetune)
SALEK_CACHED_FLOAT (setUnisonSpread, 51, setUnisonSpread)
SALEK_CACHED_INT (setScaleMode, 13, setScaleMode)
SALEK_CACHED_FLOAT (setKoronCents, 52, setKoronCents)

void SynthEngine::labRegenerate() { forEachVoice ([&] (SynthVoice& v) { v.labRegenerate(); }); }
void SynthEngine::labProcessFrames (float fold, float drive) { forEachVoice ([&] (SynthVoice& v) { v.labProcessFrames (fold, drive); }); }
void SynthEngine::labMorph (float t) { forEachVoice ([&] (SynthVoice& v) { v.labMorph (t); }); }

void SynthEngine::setOsc1Unison (int v, float d, float s)
{
    const bool changed = updateIfChanged (lastIntParameters, 16, v)
                      | updateIfChanged (lastFloatParameters, 53, d)
                      | updateIfChanged (lastFloatParameters, 54, s);
    if (changed) forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Unison (v, d, s); });
}
void SynthEngine::setOsc2Unison (int v, float d, float s)
{
    const bool changed = updateIfChanged (lastIntParameters, 17, v)
                      | updateIfChanged (lastFloatParameters, 55, d)
                      | updateIfChanged (lastFloatParameters, 56, s);
    if (changed) forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Unison (v, d, s); });
}
void SynthEngine::setOsc3Unison (int v, float d, float s)
{
    const bool changed = updateIfChanged (lastIntParameters, 18, v)
                      | updateIfChanged (lastFloatParameters, 57, d)
                      | updateIfChanged (lastFloatParameters, 58, s);
    if (changed) forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Unison (v, d, s); });
}

#undef SALEK_CACHED_FLOAT
#undef SALEK_CACHED_INT

} // namespace salek
