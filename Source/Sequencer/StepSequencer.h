#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

class StepSequencer
{
public:
    static constexpr int NumSteps = 16;

    struct Step
    {
        int   noteOffset = 0;
        float velocity   = 0.8f;
        float gate       = 0.7f;
        float probability = 1.0f;
        float modValue   = 0.0f;
        bool  active     = false;
        bool  accent     = false;
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        updateTiming();
    }

    void setBpm (double b) noexcept
    {
        bpm = juce::jlimit (20.0, 300.0, b);
        updateTiming();
    }

    void setRateDivisor (int div) noexcept
    {
        rateDivisor = juce::jlimit (1, 16, div);
        updateTiming();
    }

    void setPatternLength (int length) noexcept { patternLength = juce::jlimit (1, NumSteps, length); }
    void setSwing (float amount) noexcept { swing = juce::jlimit (0.0f, 0.75f, amount); }
    void setGate (float amount) noexcept { gateAmount = juce::jlimit (0.05f, 1.0f, amount); }

    bool isEnabled() const noexcept { return enabled; }

    void setEnabled (bool e) noexcept
    {
        enabled = e;
        if (! e) { hasRoot = false; currentNote = -1; pendingNoteOff = -1; }
    }

    void setRootNote (int n) noexcept { rootNote = juce::jlimit (0, 127, n); hasRoot = true; }

    Step& getStep (int i) noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }
    const Step& getStep (int i) const noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }

    int getCurrentStep() const noexcept { return currentStep; }
    float getCurrentMod() const noexcept { return currentMod; }

    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        if (! enabled)
        {
            if (pendingNoteOff >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, pendingNoteOff), 0);
                pendingNoteOff = -1;
                currentNote = -1;
            }
            return;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            if (gateSamplesLeft > 0)
            {
                --gateSamplesLeft;
                if (gateSamplesLeft == 0 && currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    pendingNoteOff = -1;
                    currentNote = -1;
                }
            }

            samplesUntilNext -= 1.0;
            if (samplesUntilNext <= 0.0)
            {
                if (currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    currentNote = -1;
                }

                currentStep = (currentStep + 1) % patternLength;
                const bool oddStep = (currentStep & 1) != 0;
                samplesUntilNext += samplesPerStep * (oddStep ? (1.0 - swing) : (1.0 + swing));

                auto& st = steps[static_cast<size_t> (currentStep)];
                currentMod = st.modValue;

                if (hasRoot && st.active && juce::Random::getSystemRandom().nextFloat() <= st.probability)
                {
                    int note = juce::jlimit (0, 127, rootNote + st.noteOffset);
                    float vel = juce::jlimit (0.0f, 1.0f, st.accent ? juce::jmin (1.0f, st.velocity * 1.35f) : st.velocity);
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, note, vel), i);
                    currentNote = note;
                    pendingNoteOff = note;
                    gateSamplesLeft = (int) (samplesPerStep * juce::jlimit (0.05f, 1.0f, st.gate * gateAmount));
                }
            }
        }
    }

    void initDefaultPattern() { reset(); }

    void reset()
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            steps[static_cast<size_t>(i)].active = false;
            steps[static_cast<size_t>(i)].noteOffset = 0;
            steps[static_cast<size_t>(i)].velocity = 0.8f;
            steps[static_cast<size_t>(i)].accent = false;
            steps[static_cast<size_t>(i)].gate = 0.7f;
            steps[static_cast<size_t>(i)].probability = 1.0f;
            steps[static_cast<size_t>(i)].modValue = (float) i / (float) juce::jmax (1, NumSteps - 1);
        }
        currentStep = 0;
        currentMod = steps[0].modValue;
        samplesUntilNext = 0.0;
        currentNote = -1;
        pendingNoteOff = -1;
        gateSamplesLeft = 0;
    }

private:
    void updateTiming()
    {
        // rateDivisor: 1 → 16th (4 steps/beat), 2 → 32nd (8 steps/beat), ...
        const double beatsPerSec = juce::jmax (1.0, bpm) / 60.0;
        const double stepsPerBeat = 4.0 * (double) juce::jmax (1, rateDivisor);
        samplesPerStep = sr / (beatsPerSec * stepsPerBeat);
        samplesPerStep = juce::jmax (1.0, samplesPerStep);
    }

    std::array<Step, NumSteps> steps {};
    double sr = 44100.0;
    double bpm = 120.0;
    int rateDivisor = 1;
    int patternLength = NumSteps;
    double swing = 0.0;
    float gateAmount = 1.0f;
    double samplesPerStep = 5512.5;
    double samplesUntilNext = 0.0;
    int currentStep = 0;
    int currentNote = -1;
    int pendingNoteOff = -1;
    int gateSamplesLeft = 0;
    int rootNote = 60;
    bool hasRoot = false;
    bool enabled = false;
    float currentMod = 0.0f;
};

} // namespace salek
