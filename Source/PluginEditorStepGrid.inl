/** 16-step grid: click=on, drag=vel, alt=gate, shift=pitch, ctrl=accent, wheel=pitch */
class StepGridComponent : public juce::Component, private juce::Timer
{
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq)
    {
        startTimerHz (30);
        clearBtn.setButtonText ("CLR");
        randBtn.setButtonText ("RND");
        fillBtn.setButtonText ("FILL");
        clearBtn.onClick = [this] { clearPattern(); };
        randBtn.onClick = [this] { randomPattern(); };
        fillBtn.onClick = [this] { fillPattern(); };
        for (auto* b : { &clearBtn, &randBtn, &fillBtn })
        {
            b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1030));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            addAndMakeVisible (*b);
        }
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto bar = r.removeFromTop (26).reduced (4, 2);
        clearBtn.setBounds (bar.removeFromLeft (52).reduced (2));
        randBtn.setBounds (bar.removeFromLeft (52).reduced (2));
        fillBtn.setBounds (bar.removeFromLeft (52).reduced (2));
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff060312));
        g.fillRoundedRectangle (r, 12.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.45f));
        g.drawRoundedRectangle (r, 12.0f, 1.5f);

        auto header = r.removeFromTop (28.0f);
        g.setColour (juce::Colour (0xff0c0820));
        g.fillRoundedRectangle (header.reduced (1.0f), 8.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.7f));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("SEQ  ·  click=on  drag=vel  alt=gate  shift=pitch  ctrl=accent  middle-drag=Magic XY",
                    header.withTrimmedLeft (170.0f).reduced (4, 0),
                    juce::Justification::centredLeft, false);

        r = r.reduced (6.0f);

        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2 - 18.0f;
        const int play = sequencer.getCurrentStep();

        for (int g4 = 0; g4 < n; g4 += 4)
        {
            auto band = juce::Rectangle<float> (
                r.getX() + gap + g4 * (w + gap) - 2.0f,
                r.getY(),
                4 * (w + gap) - gap + 4.0f,
                h + 8.0f);
            g.setColour (juce::Colour ((g4 / 4) % 2 == 0 ? 0xff0a1830 : 0xff120820).withAlpha (0.55f));
            g.fillRoundedRectangle (band, 6.0f);
        }

        for (int i = 0; i < n; ++i)
        {
            const auto& st = sequencer.getStep (i);
            const bool on = st.active;
            const bool isPlay = (i == play) && sequencer.isEnabled();

            auto cell = juce::Rectangle<float> (
                r.getX() + gap + i * (w + gap),
                r.getY() + 4.0f, w, h);

            if (isPlay)
            {
                g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.25f));
                g.fillRoundedRectangle (cell.expanded (2.0f), 7.0f);
            }
            g.setColour (on ? juce::Colour (0xff12102a) : juce::Colour (0xff0a0618));
            g.fillRoundedRectangle (cell, 6.0f);

            if (on)
            {
                float vh = cell.getHeight() * juce::jlimit (0.08f, 1.0f, st.velocity);
                auto velBar = cell.withTop (cell.getBottom() - vh).reduced (3.0f, 2.0f);
                juce::Colour vc = isPlay ? juce::Colour (0xffff2d9b) : juce::Colour (0xff00e8ff);
                juce::ColourGradient gr (vc.brighter (0.3f), velBar.getCentreX(), velBar.getY(),
                                         vc.darker (0.5f), velBar.getCentreX(), velBar.getBottom(), false);
                g.setGradientFill (gr);
                g.fillRoundedRectangle (velBar, 4.0f);
                g.setColour (vc.withAlpha (0.8f));
                g.fillEllipse (cell.getCentreX() - 3.5f, cell.getY() + 4.0f, 7.0f, 7.0f);

                float gateW = cell.getWidth() * juce::jlimit (0.1f, 1.0f, st.gate);
                g.setColour (juce::Colour (0xffffd700).withAlpha (0.75f));
                g.fillRoundedRectangle (cell.getX() + 3.0f, cell.getY() + 3.0f, gateW - 6.0f, 3.5f, 1.5f);

                g.setColour (juce::Colour (0xffffd700).withAlpha (0.9f));
                g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
                juce::String pitchTxt = (st.noteOffset == 0) ? "0" : ((st.noteOffset > 0 ? "+" : "") + juce::String (st.noteOffset));
                g.drawText (pitchTxt, cell.reduced (1, 8), juce::Justification::centredTop);

                if (st.accent)
                {
                    juce::Path tri;
                    float ax = cell.getRight() - 8.0f, ay = cell.getY() + 4.0f;
                    tri.addTriangle (ax, ay, ax + 6.0f, ay, ax + 3.0f, ay + 6.0f);
                    g.setColour (juce::Colour (0xffff2d9b));
                    g.fillPath (tri);
                }
            }

            // Gold lane is the per-step automation value sent to Magic X/Y.
            const float modY = cell.getBottom() - juce::jlimit (0.f, 1.f, st.modValue) * cell.getHeight();
            g.setColour (juce::Colour (0xffffd700).withAlpha (on ? 0.9f : 0.3f));
            g.drawHorizontalLine ((int) modY, cell.getX() + 2.f, cell.getRight() - 2.f);

            if (isPlay)
            {
                g.setColour (juce::Colour (0xffffd700).withAlpha (0.95f));
                g.drawRoundedRectangle (cell.expanded (1.5f), 6.0f, 2.4f);
            }
            else
            {
                g.setColour (on ? juce::Colour (0xff00e8ff).withAlpha (0.55f) : juce::Colour (0xff2a1840));
                g.drawRoundedRectangle (cell, 6.0f, 1.2f);
            }

            g.setColour (isPlay ? juce::Colour (0xffffd700) : juce::Colours::white.withAlpha (0.4f));
            g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
            g.drawText (juce::String (i + 1),
                        (int) (r.getX() + gap + i * (w + gap)), (int) (r.getBottom() - 16),
                        (int) w, 14, juce::Justification::centred);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        int idx = hitStep (e.position);
        if (idx < 0) return;
        auto& st = sequencer.getStep (idx);

        if (e.mods.isMiddleButtonDown())
        {
            setModValueFromY (idx, e.position.y);
            st.active = true;
            dragModStep = idx;
            repaint();
            return;
        }

        if (e.mods.isRightButtonDown())
        {
            st.active = false;
            st.noteOffset = 0;
            st.accent = false;
            repaint();
            return;
        }
        if (e.mods.isCtrlDown() || e.mods.isCommandDown())
        {
            st.accent = ! st.accent;
            st.active = true;
            if (st.accent) st.velocity = juce::jmax (st.velocity, 0.9f);
            repaint();
            return;
        }
        if (e.mods.isShiftDown())
        {
            static const int cycle[] = { 0, 7, 12, -12, -7 };
            int cidx = 0;
            for (int i = 0; i < 5; ++i) if (cycle[i] == st.noteOffset) { cidx = i; break; }
            st.noteOffset = cycle[(cidx + 1) % 5];
            st.active = true;
            repaint();
            return;
        }
        st.active = ! st.active;
        if (st.active && st.velocity < 0.05f) st.velocity = 0.8f;
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragModStep >= 0 && e.mods.isMiddleButtonDown())
        {
            setModValueFromY (dragModStep, e.position.y);
            repaint();
            return;
        }
        int idx = hitStep (e.position);
        if (idx < 0) return;
        auto& st = sequencer.getStep (idx);
        auto r = getLocalBounds().toFloat();
        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);
        const float h = r.getHeight() - 8.0f - 16.0f;
        float relY = 1.0f - juce::jlimit (0.f, 1.f, (e.position.y - r.getY() - 4.0f) / h);
        if (e.mods.isAltDown())
        {
            st.gate = juce::jlimit (0.05f, 1.0f, relY);
            st.active = true;
        }
        else
        {
            st.velocity = juce::jlimit (0.05f, 1.0f, relY);
            st.active = true;
        }
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override { dragModStep = -1; }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        int idx = hitStep (e.position);
        if (idx < 0) return;
        auto& st = sequencer.getStep (idx);
        int delta = (wheel.deltaY > 0.0f) ? 1 : -1;
        if (e.mods.isShiftDown()) delta *= 12;
        st.noteOffset = juce::jlimit (-24, 24, st.noteOffset + delta);
        st.active = true;
        repaint();
    }

    void timerCallback() override { if (isShowing()) repaint(); }

private:
    salek::StepSequencer& sequencer;
    juce::TextButton clearBtn, randBtn, fillBtn;
    int dragModStep = -1;

    void setModValueFromY (int idx, float y)
    {
        auto r = getLocalBounds().toFloat();
        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);
        const float h = juce::jmax (1.f, r.getHeight() - 24.0f);
        sequencer.getStep (idx).modValue = juce::jlimit (0.f, 1.f, 1.f - (y - r.getY() - 4.f) / h);
    }

    int hitStep (juce::Point<float> pos) const
    {
        auto r = getLocalBounds().toFloat();
        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 3.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        int idx = (int) ((pos.x - r.getX() - gap) / (w + gap));
        if (idx < 0 || idx >= n) return -1;
        if (pos.y < r.getY() || pos.y > r.getBottom() - 14) return -1;
        return idx;
    }

    void clearPattern()
    {
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = false; st.velocity = 0.8f; st.gate = 0.6f; st.noteOffset = 0; st.accent = false;
            st.modValue = (float) i / (float) juce::jmax (1, salek::StepSequencer::NumSteps - 1);
        }
        repaint();
    }

    void randomPattern()
    {
        juce::Random rng;
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = rng.nextFloat() > 0.45f;
            st.velocity = 0.4f + rng.nextFloat() * 0.6f;
            st.gate = 0.3f + rng.nextFloat() * 0.6f;
            st.noteOffset = 0;
            st.modValue = rng.nextFloat();
            st.accent = false;
            if (rng.nextFloat() > 0.85f) st.noteOffset = (rng.nextBool() ? 12 : -12);
            if (st.active && (i % 4) == 0) { st.accent = true; st.velocity = juce::jmax (st.velocity, 0.9f); }
        }
        repaint();
    }

    void fillPattern()
    {
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = true; st.velocity = 0.85f; st.gate = 0.6f;
            st.modValue = (float) i / (float) juce::jmax (1, salek::StepSequencer::NumSteps - 1);
        }
        repaint();
    }
};
