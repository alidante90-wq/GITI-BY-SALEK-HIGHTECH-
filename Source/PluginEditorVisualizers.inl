#pragma once
#include "UI/VisualFifo.h"

class ScopeDisplay : public juce::Component, private juce::Timer {
public:
    explicit ScopeDisplay (VisualFifo* fifo = nullptr) : visualFifo (fifo) { startTimerHz (10); }
    void setFifo (VisualFifo* f) { visualFifo = f; }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xff0a0614)); g.fillRoundedRectangle (r, 4.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.55f)); g.drawRoundedRectangle (r, 4.f, 1.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.15f));
        g.drawHorizontalLine ((int) r.getCentreY(), r.getX() + 2, r.getRight() - 2);

        const int N = 96;
        float samples[96];
        int got = 0;
        if (visualFifo != nullptr)
            got = visualFifo->readLatest (samples, N);

        juce::Path p;
        if (got > 4)
        {
            float peak = 0.001f;
            for (int i = 0; i < got; ++i) peak = juce::jmax (peak, std::abs (samples[i]));
            float scale = 0.85f / peak;
            for (int i = 0; i < got; ++i)
            {
                float t = (float) i / (float) (got - 1);
                float x = r.getX() + 3.f + t * (r.getWidth() - 6.f);
                float y = r.getCentreY() - samples[i] * scale * r.getHeight() * 0.42f;
                if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }
            g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.9f));
            g.strokePath (p, juce::PathStrokeType (1.4f));
            g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.2f));
            g.strokePath (p, juce::PathStrokeType (3.5f));
        }
        else
        {
            for (int i = 0; i < N; ++i)
            {
                float t = (float) i / (N - 1);
                float y = std::sin (t * 6.28f * 2.f + phase) * 0.08f;
                float x = r.getX() + 3.f + t * (r.getWidth() - 6.f);
                float py = r.getCentreY() + y * r.getHeight();
                if (i == 0) p.startNewSubPath (x, py); else p.lineTo (x, py);
            }
            g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.35f));
            g.strokePath (p, juce::PathStrokeType (1.0f));
        }
    }
    void timerCallback() override { if (isShowing()) { phase += 0.12f; repaint(); } }
private:
    VisualFifo* visualFifo = nullptr;
    float phase = 0.f;
};

class SpectrumDisplay : public juce::Component, private juce::Timer {
public:
    explicit SpectrumDisplay (VisualFifo* fifo = nullptr) : visualFifo (fifo) { startTimerHz (8); }
    void setFifo (VisualFifo* f) { visualFifo = f; }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xff0a0614)); g.fillRoundedRectangle (r, 4.f);
        g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.5f)); g.drawRoundedRectangle (r, 4.f, 1.f);

        const int N = 256;
        float samples[256];
        int got = visualFifo ? visualFifo->readLatest (samples, N) : 0;
        const int bands = 16;
        if (got > bands)
        {
            int per = got / bands;
            for (int b = 0; b < bands; ++b)
            {
                float e = 0.f;
                for (int i = 0; i < per; ++i)
                    e += samples[b * per + i] * samples[b * per + i];
                float energy = std::sqrt (e / (float) per);
                smooth[b] = smooth[b] * 0.65f + energy * 0.35f;
            }
        }
        float maxE = 0.001f;
        for (int b = 0; b < bands; ++b) maxE = juce::jmax (maxE, smooth[b]);
        const float bw = (r.getWidth() - 6.f) / (float) bands;
        for (int b = 0; b < bands; ++b)
        {
            float h = (smooth[b] / maxE) * (r.getHeight() - 6.f);
            auto bar = juce::Rectangle<float> (r.getX() + 3.f + b * bw + 1.f,
                                               r.getBottom() - 3.f - h, bw - 2.f, h);
            float t = (float) b / (float) (bands - 1);
            juce::Colour c = juce::Colour (0xff00e8ff).interpolatedWith (juce::Colour (0xffff2d9b), t);
            g.setColour (c.withAlpha (0.85f));
            g.fillRoundedRectangle (bar, 2.f);
        }
    }
    void timerCallback() override { if (isShowing()) repaint(); }
private:
    VisualFifo* visualFifo = nullptr;
    float smooth[16] = {};
};
