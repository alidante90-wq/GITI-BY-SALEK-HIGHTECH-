#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>

namespace salek
{

class Arpeggiator
{
public:
    enum class Direction { Up, Down, UpDown, Random };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
        sampleCounter = 0;
        stepIndex = 0;
    }

    void setBpm (double b) noexcept
    {
        bpm = juce::jlimit (20.0, 300.0, b);
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
    }

    void setRateDivisor (int div) noexcept
    {
        rateDivisor = juce::jlimit (1, 16, div);
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
    }

    void setDirection (Direction d) noexcept { direction = d; }
    void setOctaves (int o) noexcept { octaves = juce::jlimit (1, 4, o); }
    void setGate (float g) noexcept { gate = juce::jlimit (0.05f, 1.0f, g); }
    void setSwing (float s) noexcept { swing = juce::jlimit (0.0f, 0.75f, s); }
    void setEnabled (bool e) noexcept { enabled = e; if (!e) heldNotes.clear(); }

    void noteOn (int note, float velocity)
    {
        heldNotes.push_back ({ note, velocity });
        std::sort (heldNotes.begin(), heldNotes.end(),
                   [] (const Note& a, const Note& b) { return a.note < b.note; });
        rebuildPattern();
        stepIndex = 0;
    }

    void noteOff (int note)
    {
        heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
            [note] (const Note& n) { return n.note == note; }), heldNotes.end());
        rebuildPattern();
        if (heldNotes.empty()) stepIndex = 0;
    }

    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        if (! enabled || pattern.empty())
        {
            if (currentPlaying >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, currentPlaying), 0);
                currentPlaying = -1;
            }
            return;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            if (sampleCounter <= 0)
            {
                if (currentPlaying >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentPlaying), i);
                    currentPlaying = -1;
                }
                int intervalSamples = samplesPerStep;
                if (! pattern.empty())
                {
                    const int phase = stepIndex & 1;
                    const float swingScale = phase == 0 ? (1.0f + swing) : (1.0f - swing);
                    intervalSamples = juce::jmax (1, static_cast<int> (std::round (samplesPerStep * swingScale)));
                    auto& step = pattern[static_cast<size_t> (stepIndex % (int) pattern.size())];
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, step.note, step.velocity), i);
                    currentPlaying = step.note;
                    gateSamplesLeft = juce::jmax (1, static_cast<int> (intervalSamples * gate));
                    stepIndex++;
                }
                sampleCounter = intervalSamples;
            }

            if (currentPlaying >= 0 && gateSamplesLeft > 0)
            {
                --gateSamplesLeft;
                if (gateSamplesLeft <= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentPlaying), i);
                    currentPlaying = -1;
                }
            }

            --sampleCounter;
        }
    }

private:
    struct Note { int note; float velocity; };

    void rebuildPattern()
    {
        pattern.clear();
        if (heldNotes.empty()) return;

        std::vector<Note> base = heldNotes;
        if (direction == Direction::Down)
            std::reverse (base.begin(), base.end());

        for (int o = 0; o < octaves; ++o)
        {
            for (auto n : base)
            {
                int nn = n.note + o * 12;
                if (nn >= 0 && nn <= 127)
                    pattern.push_back ({ nn, n.velocity });
            }
        }

        if (direction == Direction::UpDown && pattern.size() > 1)
        {
            for (int i = static_cast<int> (pattern.size()) - 2; i > 0; --i)
                pattern.push_back (pattern[static_cast<size_t> (i)]);
        }
        if (direction == Direction::Random && pattern.size() > 1)
        {
            juce::Random rng;
            for (int i = (int) pattern.size() - 1; i > 0; --i)
            {
                int j = rng.nextInt (i + 1);
                std::swap (pattern[(size_t) i], pattern[(size_t) j]);
            }
        }
    }

    double sr = 44100.0;
    double bpm = 175.0;
    int rateDivisor = 4;
    int samplesPerStep = 1000;
    int sampleCounter = 0;
    int stepIndex = 0;
    int gateSamplesLeft = 0;
    int currentPlaying = -1;
    int octaves = 1;
    float gate = 0.6f;
    float swing = 0.0f;
    bool enabled = false;
    Direction direction = Direction::Up;
    std::vector<Note> heldNotes;
    std::vector<Note> pattern;
};

} // namespace salek
