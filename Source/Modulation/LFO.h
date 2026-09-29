#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>
namespace salek {
class LFO {
public:
    enum class Wave { Sine, Triangle, Saw, Square, SAndH, Custom, SmoothRnd, Chaos, Pulse, Exp, Sine3, SoftSquare };
    static constexpr int TableSize = 32;
    static constexpr int NumShapes = 32;

    void prepare(double sampleRate){ sr=sampleRate>0?sampleRate:44100; phase=0; loadPresetShape(0); }
    void reset() noexcept { phase=0; lastSH=0; }
    void setRate(float hz) noexcept { hz=juce::jlimit(0.01f,40.f,hz); if (std::abs(hz-rate)>0.0001f) { rate=hz; phaseInc=double(rate)/sr; } }
    void setWave(Wave w) noexcept { wave=w; }
    void setAmount(float a) noexcept { amount=juce::jlimit(0.f,1.f,a); }
    void setCustomPoint (int i, float v) noexcept
    {
        if (i >= 0 && i < TableSize)
            customTable[(size_t) i] = juce::jlimit (-1.f, 1.f, v);
    }
    float getCustomPoint (int i) const noexcept
    {
        return (i >= 0 && i < TableSize) ? customTable[(size_t) i] : 0.f;
    }
    std::array<float, TableSize>& getTable() noexcept { return customTable; }
    const std::array<float, TableSize>& getTable() const noexcept { return customTable; }

    static const char* shapeName (int preset) noexcept
    {
        static const char* names[32] = {
            "Sine","Triangle","Saw Up","Saw Down","Square","Pulse 25","Pulse 12","Exp Rise",
            "Exp Fall","Log Rise","Bell","Wobble","3-Step","Gate Sweep","Soft Curve","Spike Train",
            "Sine x3","AM Sine","Rectified","Tent","Soft Sine","Late Ramp","Half Ramp","Dying Wobble",
            "3-State","Harmonic","Folded Saw","Cosine x2","Blip","Saturated","Pattern","FM Curve"
        };
        return names[((preset % 32) + 32) % 32];
    }

    void loadPresetShape (int preset) noexcept
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        preset = ((preset % 32) + 32) % 32;
        for (int i = 0; i < TableSize; ++i)
        {
            float t = (float) i / (float) (TableSize - 1);
            float v = 0.f;
            switch (preset)
            {
                case 0:  v = std::sin (t * twoPi); break;
                case 1:  v = 1.f - 4.f * std::abs (t - 0.5f); break;
                case 2:  v = 2.f * t - 1.f; break;
                case 3:  v = 1.f - 2.f * t; break;
                case 4:  v = t < 0.5f ? 1.f : -1.f; break;
                case 5:  v = t < 0.25f ? 1.f : -1.f; break;
                case 6:  v = t < 0.12f ? 1.f : -1.f; break;
                case 7:  v = 2.f * (1.f - std::exp (-4.f * t)) - 1.f; break;
                case 8:  v = 2.f * std::exp (-4.f * t) - 1.f; break;
                case 9:  v = 2.f * std::log (1.f + 9.f * t) / std::log (10.f) - 1.f; break;
                case 10: v = 0.5f - 0.5f * std::cos (t * twoPi); break;
                case 11: v = std::sin (t * twoPi * 2.f) * (1.f - 0.35f * t); break;
                case 12: v = (i % 3 == 0) ? 1.f : ((i % 3 == 1) ? -0.6f : 0.15f); break;
                case 13: v = t < 0.15f ? (t / 0.15f) * 2.f - 1.f
                         : (t < 0.7f ? 1.f : 1.f - (t - 0.7f) / 0.3f * 2.f); break;
                case 14: v = std::tanh ((t - 0.5f) * 6.f); break;
                case 15: v = (i == 2 || i == 9 || i == 13) ? 1.f : ((i == 5 || i == 11) ? -1.f : 0.f); break;
                case 16: v = std::sin (t * twoPi * 3.f); break;
                case 17: v = std::sin (t * twoPi) * std::sin (t * twoPi * 2.f); break;
                case 18: v = std::abs (std::sin (t * twoPi)) * 2.f - 1.f; break;
                case 19: v = t < 0.5f ? 2.f * t : 2.f - 2.f * t; break;
                case 20: v = std::sin (std::pow (t, 0.4f) * twoPi); break;
                case 21: v = std::sin (std::pow (t, 2.2f) * twoPi); break;
                case 22: v = (t < 0.5f ? t * 2.f : 0.f) * 2.f - 1.f; break;
                case 23: v = std::sin (t * twoPi * 4.f) * (1.f - t); break;
                case 24: v = t < 0.33f ? -1.f : (t < 0.66f ? 0.f : 1.f); break;
                case 25: v = std::sin (t * twoPi) + 0.35f * std::sin (t * twoPi * 5.f); break;
                case 26: v = 2.f * std::fmod (t * 3.f, 1.f) - 1.f; break;
                case 27: v = std::cos (t * twoPi) * std::cos (t * twoPi * 0.5f); break;
                case 28: v = t < 0.08f ? 1.f : (t < 0.16f ? -1.f : 0.f); break;
                case 29: v = std::tanh (std::sin (t * twoPi * 2.f) * 3.f); break;
                case 30: v = (i % 4 == 0) ? 1.f : ((i % 4 == 2) ? -1.f : 0.f); break;
                default: v = std::sin (t * twoPi * 2.f + std::sin (t * twoPi * 3.f)); break;
            }
            customTable[(size_t) i] = juce::jlimit (-1.f, 1.f, v);
        }
    }

    void setRandomSeed (uint32_t seed) noexcept { randomState = seed ? seed : 0xA341316Cu; }
    float nextRandom() noexcept
    {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 17;
        randomState ^= randomState << 5;
        return (float) (randomState & 0x00FFFFFFu) / 16777215.0f;
    }

    float process() noexcept {
        float v=0, p=float(phase);
        switch(wave){
            case Wave::Sine: v=std::sin(p*juce::MathConstants<float>::twoPi); break;
            case Wave::Triangle: v=1-4*std::abs(p-0.5f); break;
            case Wave::Saw: v=2*p-1; break;
            case Wave::Square: v=p<0.5f?1.f:-1.f; break;
            case Wave::SAndH:
                if(phase<phaseInc) lastSH=nextRandom()*2-1;
                v=lastSH; break;
            case Wave::SmoothRnd:
            {
                if (phase < phaseInc)
                {
                    lastSH2 = lastSH;
                    lastSH = nextRandom()*2-1;
                }
                float frac = (float) (phase / juce::jmax (1e-9, phaseInc));
                frac = juce::jlimit (0.f, 1.f, frac);
                frac = frac * frac * (3.f - 2.f * frac);
                v = lastSH2 * (1.f - frac) + lastSH * frac;
                break;
            }
            case Wave::Chaos:
                chaosState = 3.7f * chaosState * (1.f - chaosState);
                v = chaosState * 2.f - 1.f;
                break;
            case Wave::Pulse:
                v = p < 0.18f ? 1.f : -0.35f;
                break;
            case Wave::Exp:
                v = 2.f * (1.f - std::exp (-5.f * p)) - 1.f;
                break;
            case Wave::Sine3:
                v = std::sin (p * juce::MathConstants<float>::twoPi)
                  + 0.35f * std::sin (3.f * p * juce::MathConstants<float>::twoPi);
                v *= 0.75f;
                break;
            case Wave::SoftSquare:
            {
                float s = std::sin (p * juce::MathConstants<float>::twoPi);
                v = std::tanh (s * 4.f);
                break;
            }
            case Wave::Custom:
            {
                float idx = p * (float) (TableSize - 1);
                int i0 = juce::jlimit (0, TableSize - 1, (int) idx);
                int i1 = juce::jmin (TableSize - 1, i0 + 1);
                float f = idx - (float) i0;
                v = customTable[(size_t) i0] * (1.f - f) + customTable[(size_t) i1] * f;
                break;
            }
        }
        phase+=phaseInc; if(phase>=1) phase-=1;
        return v*amount;
    }

    float getPhase() const noexcept { return (float) phase; }

private:
    double sr=44100, phase=0, phaseInc=0; float rate=1, amount=0, lastSH=0, lastSH2=0, chaosState=0.3f;
    uint32_t randomState = 0xA341316Cu;
    Wave wave=Wave::Sine;
    std::array<float, TableSize> customTable {};
};
}
