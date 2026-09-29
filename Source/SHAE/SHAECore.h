#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <limits>

namespace shae
{
enum class Quality : int { Eco = 1, Normal = 2, High = 4, Ultra = 8 };

inline float softClip (float x, float drive = 1.0f) noexcept
{
    const float d = juce::jmax (0.001f, drive);
    return std::tanh (x * d) / std::tanh (d);
}

inline float safeSample (float x, float ceiling = 0.995f) noexcept
{
    if (!std::isfinite (x)) return 0.0f;
    return juce::jlimit (-ceiling, ceiling, x);
}

inline float onePoleLowpass (float x, float& state, float cutoffHz, float sampleRate) noexcept
{
    const float fc = juce::jlimit (20.0f, sampleRate * 0.45f, cutoffHz);
    const float a = std::exp (-juce::MathConstants<float>::twoPi * fc / sampleRate);
    state += (1.0f - a) * (x - state);
    return state;
}

inline float antiAliasCutoff (float frequencyHz, float sampleRate, bool nonlinear) noexcept
{
    const float ny = sampleRate * 0.5f;
    if (!nonlinear && frequencyHz < ny * 0.12f) return ny * 0.98f;
    // Rich table frames and nonlinear stages must leave room for upper
    // harmonics; the former coefficient was less conservative when enabled.
    const float headroom = nonlinear ? 1.35f : 0.78f;
    return juce::jlimit (1000.0f, ny * 0.96f,
                         ny - frequencyHz * headroom);
}

class ParameterSmoother
{
public:
    void prepare (double sampleRate, float timeMs, float initial) noexcept
    {
        current = target = initial;
        const float t = juce::jmax (0.1f, timeMs) * 0.001f;
        coeff = std::exp (-1.0f / static_cast<float> (juce::jmax (1.0, sampleRate * t)));
    }
    void setTarget (float v) noexcept { target = v; }
    float next() noexcept { current = target + coeff * (current - target); return current; }
    float getCurrent() const noexcept { return current; }
private:
    float current = 0.f, target = 0.f, coeff = 0.f;
};

class SafetyStage
{
public:
    void prepare (double sampleRate, int channels)
    {
        juce::ignoreUnused (channels);
        fs = sampleRate > 1.0 ? sampleRate : 44100.0;
        x1L = x1R = y1L = y1R = 0.0f;
        const double rc = 1.0 / (juce::MathConstants<double>::twoPi * 18.0);
        hpA = static_cast<float> (rc / (rc + 1.0 / fs));
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        if (chs <= 0 || n <= 0) return;

        auto run = [this, n] (float* d, float& x1, float& y1) noexcept
        {
            for (int i = 0; i < n; ++i)
            {
                float x = d[i];
                if (!std::isfinite (x)) x = 0.0f;

                const float y = x - x1 + hpA * y1;
                x1 = x;
                y1 = std::isfinite (y) ? y : 0.0f;

                // Do not distort the whole signal. Only engage the safety
                // curve when a sample approaches the final ceiling.
                float out = y;
                const float ax = std::abs (out);
                if (ax > 0.90f)
                    out = std::copysign (0.90f + 0.095f * std::tanh ((ax - 0.90f) * 5.0f), out);

                d[i] = safeSample (out, 0.995f);
            }
        };

        run (buffer.getWritePointer (0), x1L, y1L);
        if (chs > 1) run (buffer.getWritePointer (1), x1R, y1R);
        for (int ch = 2; ch < chs; ++ch)
            for (int i = 0; i < n; ++i)
                buffer.setSample (ch, i, safeSample (buffer.getSample (ch, i)));
    }

private:
    double fs = 44100.0;
    float hpA = 0.997f, x1L = 0.f, x1R = 0.f, y1L = 0.f, y1R = 0.f;
};
}
