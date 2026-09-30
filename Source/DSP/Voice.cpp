#include "Voice.h"

SalekVoice::SalekVoice (juce::AudioProcessorValueTreeState& vts, ModulationMatrix& matrix)
    : apvts (vts), modMatrix (matrix) {}

bool SalekVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SalekSound*> (sound) != nullptr;
}

void SalekVoice::prepare (double sampleRate, int)
{
    sr = sampleRate;
    osc1.prepare (sampleRate); osc2.prepare (sampleRate); osc3.prepare (sampleRate); subOsc.prepare (sampleRate);
    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    filter.prepare (spec);
    ampEnv.prepare (sampleRate); filterEnv.prepare (sampleRate); modEnv.prepare (sampleRate);
    lfo1.prepare (sampleRate); lfo2.prepare (sampleRate); lfo3.prepare (sampleRate); lfo4.prepare (sampleRate);
    static const float centsTable[maxUnison] = { -18.f, -7.f, 0.f, 7.f, 18.f };
    for (int i = 0; i < maxUnison; ++i)
        unisonDetune[i] = std::pow (2.0f, centsTable[i] / 1200.0f) - 1.0f;
    isPrepared = true; paramsDirty = true;
}

void SalekVoice::startNote (int midiNoteNumber, float vel, juce::SynthesiserSound*, int)
{
    currentNote = midiNoteNumber; velocity = vel;
    noteHz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    osc1.reset(); osc2.reset(); osc3.reset(); subOsc.reset(); filter.reset();
    ampEnv.noteOn(); filterEnv.noteOn(); modEnv.noteOn();
    lfo1.reset(); lfo2.reset(); lfo3.reset(); lfo4.reset();
    paramsDirty = true;
}

void SalekVoice::stopNote (float, bool allowTailOff)
{
    ampEnv.noteOff(); filterEnv.noteOff(); modEnv.noteOff();
    if (!allowTailOff || !ampEnv.isActive()) clearCurrentNote();
}

void SalekVoice::updateParameters()
{
    auto get = [this](const juce::String& id) -> float {
        if (auto* p = apvts.getRawParameterValue (id)) return p->load(); return 0.0f;
    };
    auto calcFreq = [&](const juce::String& pre) -> float {
        return noteHz * std::pow (2.0f, (get(pre+"octave")*12.f + get(pre+"semitone") + get(pre+"detune")) / 12.0f);
    };
    baseOsc1Freq = calcFreq("osc1_"); baseOsc2Freq = calcFreq("osc2_"); baseOsc3Freq = calcFreq("osc3_");
    baseOsc1Level = get("osc1_level"); baseOsc2Level = get("osc2_level"); baseOsc3Level = get("osc3_level");
    baseSubLevel = get("sub_level"); baseNoiseLevel = get("noise_level");
    baseOsc1WT = get("osc1_wtpos"); baseOsc1Morph = get("osc1_morph"); baseOsc1Warp = get("osc1_warp");
    baseOsc1FM = get("osc1_fm"); baseOsc1AM = get("osc1_am"); baseOsc1RM = get("osc1_rm");
    baseOsc2WT = get("osc2_wtpos"); baseOsc2Morph = get("osc2_morph"); baseOsc2Warp = get("osc2_warp");
    baseOsc2FM = get("osc2_fm"); baseOsc2AM = get("osc2_am"); baseOsc2RM = get("osc2_rm");
    baseOsc3WT = get("osc3_wtpos"); baseOsc3Morph = get("osc3_morph"); baseOsc3Warp = get("osc3_warp");
    baseOsc3FM = get("osc3_fm"); baseOsc3AM = get("osc3_am"); baseOsc3RM = get("osc3_rm");
    baseSubFreq = noteHz * std::pow (2.0f, get("sub_octave"));
    baseCutoff = get("filter_cutoff"); baseRes = get("filter_res"); baseDrive = get("filter_drive"); baseKeytrack = get("filter_keytrack");
    filter.setType (static_cast<MultiFilter::Type>((int)get("filter_type"))); filter.setDrive (baseDrive);
    ampEnv.setAttack(get("env1_attack")); ampEnv.setDecay(get("env1_decay")); ampEnv.setSustain(get("env1_sustain")); ampEnv.setRelease(get("env1_release"));
    filterEnv.setAttack(get("env2_attack")); filterEnv.setDecay(get("env2_decay")); filterEnv.setSustain(get("env2_sustain")); filterEnv.setRelease(get("env2_release"));
    modEnv.setAttack(get("env3_attack")); modEnv.setDecay(get("env3_decay")); modEnv.setSustain(get("env3_sustain")); modEnv.setRelease(get("env3_release"));
    lfo1.setRate(get("lfo1_rate")); lfo1.setDepth(get("lfo1_depth")); lfo1.setShape(static_cast<LFO::Shape>((int)get("lfo1_shape"))); lfo1.setOneShot(get("lfo1_oneshot")>0.5f);
    lfo2.setRate(get("lfo2_rate")); lfo2.setDepth(get("lfo2_depth")); lfo2.setShape(static_cast<LFO::Shape>((int)get("lfo2_shape")));
    lfo3.setRate(get("lfo3_rate")); lfo3.setDepth(get("lfo3_depth")); lfo3.setShape(static_cast<LFO::Shape>((int)get("lfo3_shape")));
    lfo4.setRate(get("lfo4_rate")); lfo4.setDepth(get("lfo4_depth")); lfo4.setShape(static_cast<LFO::Shape>((int)get("lfo4_shape")));
    unisonVoices = juce::jlimit(1, maxUnison, (int)get("osc1_unison")); unisonDetuneAmt = get("osc1_udet")*0.01f;
    paramsDirty = false;
}

void SalekVoice::applyModulation (float* destOffsets)
{
    for (int i = 0; i < ModulationMatrix::numDests; ++i) destOffsets[i] = 0.0f;
    modSources[ModulationMatrix::LFO1] = lfo1.process();
    modSources[ModulationMatrix::LFO2] = lfo2.process();
    modSources[ModulationMatrix::LFO3] = lfo3.process();
    modSources[ModulationMatrix::LFO4] = lfo4.process();
    modSources[ModulationMatrix::Env1] = ampEnv.getLevel();
    modSources[ModulationMatrix::Env2] = filterEnv.getLevel();
    modSources[ModulationMatrix::Env3] = modEnv.getLevel();
    modSources[ModulationMatrix::Velocity] = velocity * 2.0f - 1.0f;
    modSources[ModulationMatrix::NoteNumber] = (currentNote - 60) / 48.0f;
    modMatrix.process (modSources, destOffsets);
}

void SalekVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isPrepared || !isVoiceActive()) return;
    updateParameters();
    auto* left = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : left;
    const float noteOffsetBase = (currentNote - 60) * baseKeytrack * 40.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        applyModulation (modDests);
        const float m1L = modDests[ModulationMatrix::Osc1Level], m1W = modDests[ModulationMatrix::Osc1WTPos];
        const float m1M = modDests[ModulationMatrix::Osc1Morph], m1Wp = modDests[ModulationMatrix::Osc1Warp];
        const float m1F = modDests[ModulationMatrix::Osc1FM], m1A = modDests[ModulationMatrix::Osc1AM], m1R = modDests[ModulationMatrix::Osc1RM];
        const float m2L = modDests[ModulationMatrix::Osc2Level], m2W = modDests[ModulationMatrix::Osc2WTPos];
        const float m2M = modDests[ModulationMatrix::Osc2Morph], m2Wp = modDests[ModulationMatrix::Osc2Warp];
        const float m2F = modDests[ModulationMatrix::Osc2FM], m2A = modDests[ModulationMatrix::Osc2AM], m2R = modDests[ModulationMatrix::Osc2RM];
        const float m3L = modDests[ModulationMatrix::Osc3Level], m3W = modDests[ModulationMatrix::Osc3WTPos];
        const float m3M = modDests[ModulationMatrix::Osc3Morph], m3Wp = modDests[ModulationMatrix::Osc3Warp];
        const float m3F = modDests[ModulationMatrix::Osc3FM], m3A = modDests[ModulationMatrix::Osc3AM], m3R = modDests[ModulationMatrix::Osc3RM];
        const float mSub = modDests[ModulationMatrix::SubLevel], mNz = modDests[ModulationMatrix::NoiseLevel];
        const float mCut = modDests[ModulationMatrix::FilterCutoff]*5500.f, mRes = modDests[ModulationMatrix::FilterRes]*0.45f;
        const float mDrv = modDests[ModulationMatrix::FilterDrive]*0.5f, mAmp = modDests[ModulationMatrix::AmpLevel];

        osc1.setFrequency(baseOsc1Freq); osc1.setLevel(juce::jlimit(0.f,1.5f,(baseOsc1Level+m1L)*velocity));
        osc1.setWavetablePos(juce::jlimit(0.f,1.f,baseOsc1WT+m1W)); osc1.setMorph(juce::jlimit(0.f,1.f,baseOsc1Morph+m1M));
        osc1.setWarp(juce::jlimit(0.f,1.f,baseOsc1Warp+m1Wp)); osc1.setFM(baseOsc1FM+m1F);
        osc1.setAM(juce::jlimit(0.f,1.f,baseOsc1AM+m1A)); osc1.setRM(juce::jlimit(0.f,1.f,baseOsc1RM+m1R));
        osc1.setWarpMode(Oscillator::WarpMode::Fold);

        osc2.setFrequency(baseOsc2Freq); osc2.setLevel(juce::jlimit(0.f,1.5f,(baseOsc2Level+m2L)*velocity));
        osc2.setWavetablePos(juce::jlimit(0.f,1.f,baseOsc2WT+m2W)); osc2.setMorph(juce::jlimit(0.f,1.f,baseOsc2Morph+m2M));
        osc2.setWarp(juce::jlimit(0.f,1.f,baseOsc2Warp+m2Wp)); osc2.setFM(baseOsc2FM+m2F);
        osc2.setAM(juce::jlimit(0.f,1.f,baseOsc2AM+m2A)); osc2.setRM(juce::jlimit(0.f,1.f,baseOsc2RM+m2R));
        osc2.setWarpMode(Oscillator::WarpMode::PD);

        osc3.setFrequency(baseOsc3Freq); osc3.setLevel(juce::jlimit(0.f,1.5f,(baseOsc3Level+m3L)*velocity));
        osc3.setWavetablePos(juce::jlimit(0.f,1.f,baseOsc3WT+m3W)); osc3.setMorph(juce::jlimit(0.f,1.f,baseOsc3Morph+m3M));
        osc3.setWarp(juce::jlimit(0.f,1.f,baseOsc3Warp+m3Wp)); osc3.setFM(baseOsc3FM+m3F);
        osc3.setAM(juce::jlimit(0.f,1.f,baseOsc3AM+m3A)); osc3.setRM(juce::jlimit(0.f,1.f,baseOsc3RM+m3R));
        osc3.setWarpMode(Oscillator::WarpMode::Sync);

        subOsc.setFrequency(baseSubFreq); subOsc.setLevel(juce::jlimit(0.f,1.5f,(baseSubLevel+mSub)*velocity)); subOsc.setWavetablePos(0.f);
        const float noiseAmt = juce::jlimit(0.f,1.5f,(baseNoiseLevel+mNz)*velocity);

        filter.setCutoff(juce::jlimit(20.f,20000.f,baseCutoff+mCut+noteOffsetBase));
        filter.setResonance(juce::jlimit(0.f,1.f,baseRes+mRes));
        filter.setDrive(juce::jlimit(0.f,1.f,baseDrive+mDrv));

        const float s2p = osc2.getLastSample(), s3p = osc3.getLastSample();
        osc1.setPhaseMod(s2p*0.7f + s3p*0.3f + modSources[ModulationMatrix::LFO1]*0.35f);
        osc2.setPhaseMod(s3p*0.5f + modSources[ModulationMatrix::LFO2]*0.25f);
        osc3.setPhaseMod(modSources[ModulationMatrix::LFO3]*0.2f);
        osc1.setRingModInput(s2p); osc2.setRingModInput(s3p); osc3.setRingModInput(s2p);

        float mixed = osc1.process() + osc2.process() + osc3.process() + subOsc.process()
                    + (noiseRandom.nextFloat()*2.f - 1.f) * noiseAmt;
        float e2 = filterEnv.process(), e1 = ampEnv.process(); modEnv.process();
        float y = filter.processSample(mixed * (1.f + e2 * 0.3f));
        y *= e1 * juce::jlimit(0.f, 1.5f, 1.f + mAmp);
        y = std::tanh(y * 1.35f);
        left[i] += y; right[i] += y;
        if (!ampEnv.isActive()) { clearCurrentNote(); break; }
    }
}
