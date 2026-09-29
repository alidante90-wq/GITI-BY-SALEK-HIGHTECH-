#pragma once
#include <JuceHeader.h>
#include "../Synth/SynthVoice.h"

/** Live mini wavetable-shape scope for one oscillator */
class OscShapeMonitor : public juce::Component, private juce::Timer
{
public:
    OscShapeMonitor() { }
    void visibilityChanged() override
    {
        if (isShowing()) startTimerHz (25);
        else stopTimer();
    }

    void setAPVTS (juce::AudioProcessorValueTreeState* s) { apvts = s; }
    void setOscIndex (int i) { osc = juce::jlimit (0, 2, i); }
    void setAccent (juce::Colour c) { accent = c; }
    void setTitle (const juce::String& t) { title = t; }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);

        // 3D panel + outer glow (Serum-inspired)
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (2.f, 3.f), 7.f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 7.f);

        // double-frame
        g.setColour (accent.withAlpha (0.55f));
        g.drawRoundedRectangle (r, 7.f, 1.6f);
        g.setColour (accent.withAlpha (0.18f));
        g.drawRoundedRectangle (r.reduced (2.5f), 5.5f, 1.0f);

        g.setColour (accent);
        g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.drawText (title, r.removeFromTop (15).reduced (5, 0), juce::Justification::centredLeft);

        auto plot = r.reduced (7.f, 5.f).withTrimmedBottom (12.f);

        // subtle grid
        g.setColour (accent.withAlpha (0.08f));
        for (int i = 1; i < 4; ++i)
        {
            float x = plot.getX() + plot.getWidth() * (float) i / 4.f;
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        }
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

        float table = 0.f, warp = 0.f, fold = 0.f, drive = 0.f;
        if (apvts != nullptr)
        {
            auto gval = [&] (const char* id, float d) {
                if (auto* p = apvts->getRawParameterValue (id)) return p->load();
                return d;
            };
            const char* tid[] = { "osc1_table", "osc2_table", "osc3_table" };
            const char* wid[] = { "osc1_warp", "osc2_warp", "osc3_warp" };
            const char* fid[] = { "osc1_fold", "osc2_fold", "osc3_fold" };
            const char* did[] = { "osc1_drive", "osc2_drive", "osc3_drive" };
            table = gval (tid[osc], 0.f);
            warp  = gval (wid[osc], 0.f);
            fold  = gval (fid[osc], 0.f);
            drive = gval (did[osc], 0.f);
        }

        juce::Path wave, fill;
        const int N = 96;
        for (int i = 0; i < N; ++i)
        {
            float t = (float) i / (float) (N - 1);
            // Draw samples from the same SHAE wavetable the oscillator reads.
            float phase = t;
            if (warp > 1.0e-4f)
            {
                const float amount = 1.0f + warp * 3.4f;
                phase = std::pow (juce::jmax (1.0e-6f, phase), amount);
                if (warp > 0.42f)
                {
                    const float mirrorAmt = juce::jlimit (0.f, 1.f, (warp - 0.42f) * 1.4f);
                    if (phase > 0.5f) phase = 0.5f + (phase - 0.5f) * (1.f - mirrorAmt * 0.55f);
                    if (warp > 0.75f) phase = std::fmod (phase * (1.f + (warp - 0.75f) * 3.2f), 1.f);
                }
                phase = juce::jlimit (0.f, 0.99999f, phase);
            }
            float y = salek::SynthVoice::getSharedWavetable().getSample (table, phase);
            if (fold > 1.0e-4f)
            {
                const float f = fold * fold;
                const float x = y * (1.f + f * 7.5f);
                float hard = x;
                for (int k = 0; k < 3; ++k) { if (hard > 1.f) hard = 2.f - hard; else if (hard < -1.f) hard = -2.f - hard; else break; }
                const float folded = std::sin (x * juce::MathConstants<float>::halfPi * (0.8f + f * 0.9f));
                y = (folded * (1.f - f * 0.45f) + hard * (f * 0.45f)) / (1.f + f * 0.9f);
            }
            if (drive > 1.0e-4f)
            {
                const float gain = 1.f + drive * 5.8f;
                float shaped = y * gain;
                shaped = std::tanh (shaped * (1.f + drive * 0.28f)) - 0.07f * drive * shaped * shaped;
                y = shaped / (0.88f + 0.12f * std::tanh (gain));
            }
            y *= 0.88f;
            float px = plot.getX() + t * plot.getWidth();
            float py = plot.getCentreY() - y * plot.getHeight() * 0.44f;
            if (i == 0) { wave.startNewSubPath (px, py); fill.startNewSubPath (px, plot.getBottom()); fill.lineTo (px, py); }
            else { wave.lineTo (px, py); fill.lineTo (px, py); }
        }
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.closeSubPath();

        g.setColour (accent.withAlpha (0.12f));
        g.fillPath (fill);

        g.setColour (accent.withAlpha (0.95f));
        g.strokePath (wave, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        float scan = std::fmod (anim * 0.18f, 1.f);
        float sx = plot.getX() + scan * plot.getWidth();
        g.setColour (accent.withAlpha (0.35f));
        g.drawVerticalLine ((int) sx, plot.getY(), plot.getBottom());

        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.setColour (accent.withAlpha (0.9f));
        auto info = juce::String::formatted ("T:%.0f  W:%.0f  F:%.0f",
            table * 100.f, warp * 100.f, fold * 100.f);
        g.drawText (info, getLocalBounds().removeFromBottom (13).reduced (5, 0),
                    juce::Justification::centredLeft);
    }

    void timerCallback() override { if (! isShowing()) return; anim += 0.1f; repaint(); }


    // Drag horizontally on the monitor to scrub TABLE (wavetable frame morph 0..1)
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (apvts == nullptr) return;
        dragTable (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (apvts == nullptr) return;
        if (e.mods.isAltDown())
        {
            // Alt+drag: edit FOLD (Y) and WARP (X) — point-style shape sculpting proxy
            const char* foldIds[] = { "osc1_fold", "osc2_fold", "osc3_fold" };
            const char* warpIds[] = { "osc1_warp", "osc2_warp", "osc3_warp" };
            float nx = juce::jlimit (0.f, 1.f, e.position.x / (float) juce::jmax (1, getWidth()));
            float ny = juce::jlimit (0.f, 1.f, 1.f - e.position.y / (float) juce::jmax (1, getHeight()));
            if (auto* pf = apvts->getParameter (foldIds[osc]))
            { pf->beginChangeGesture(); pf->setValueNotifyingHost (pf->convertTo0to1 (ny)); pf->endChangeGesture(); }
            if (auto* pw = apvts->getParameter (warpIds[osc]))
            { pw->beginChangeGesture(); pw->setValueNotifyingHost (pw->convertTo0to1 (nx)); pw->endChangeGesture(); }
            return;
        }
        dragTable (e);
    }
    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        // snap to classic frames: 0 sine, 0.25 tri, 0.5 square-ish, 0.75 saw-ish
        if (apvts == nullptr) return;
        const char* ids[] = { "osc1_table", "osc2_table", "osc3_table" };
        if (auto* p = apvts->getParameter (ids[osc]))
        {
            static float snaps[4] = { 0.f, 0.25f, 0.5f, 0.75f };
            snapIdx = (snapIdx + 1) % 4;
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (snaps[snapIdx]));
            p->endChangeGesture();
        }
    }

private:
    void dragTable (const juce::MouseEvent& e)
    {
        const char* ids[] = { "osc1_table", "osc2_table", "osc3_table" };
        if (auto* p = apvts->getParameter (ids[osc]))
        {
            float t = juce::jlimit (0.f, 1.f, e.position.x / (float) juce::jmax (1, getWidth()));
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (t));
            p->endChangeGesture();
        }
    }
    int snapIdx = 0;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    int osc = 0;
    float anim = 0.f;
    juce::Colour accent { 0xff00e8ff };
    juce::String title { "OSC" };
};
