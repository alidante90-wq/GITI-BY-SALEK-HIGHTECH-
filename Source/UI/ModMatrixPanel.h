#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

/** Serum-inspired mod matrix: grid + selected route amount strip (horizontal fader) */
class ModMatrixPanel : public juce::Component, private juce::Timer
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m)
    {
        startTimerHz (8);
        matrix.addRoute (salek::ModMatrix::Source::LFO1, salek::ModMatrix::Dest::FilterCutoff, 0.0f);
        matrix.addRoute (salek::ModMatrix::Source::LFO2, salek::ModMatrix::Dest::Osc1Table, 0.0f);
        matrix.addRoute (salek::ModMatrix::Source::LFO3, salek::ModMatrix::Dest::Osc1Warp, 0.0f);
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.85f));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.35f));
        g.drawRoundedRectangle (r, 8.0f, 1.2f);

        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffffd700));
        g.drawText ("MOD MATRIX  |  drag/wheel = amount  |  dbl-click = clear",
                    r.removeFromTop (18).reduced (6, 0), juce::Justification::centredLeft);

        layoutGrid();
        const int ns = (int) salek::ModMatrix::Source::NumSources;
        const int nd = (int) salek::ModMatrix::Dest::NumDests;

        g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
        for (int d = 0; d < nd; ++d)
        {
            auto cell = cellRect (-1, d);
            g.setColour (juce::Colour (0xffc0a0d0));
            g.drawText (salek::ModMatrix::destName ((salek::ModMatrix::Dest) d),
                        cell.toNearestInt(), juce::Justification::centred);
        }

        for (int s = 0; s < ns; ++s)
        {
            auto label = cellRect (s, -1);
            float srcVal = matrix.getSourceValue ((salek::ModMatrix::Source) s);
            g.setColour (juce::Colour (0xff12101c));
            g.fillRoundedRectangle (label, 3.0f);
            float mid = label.getCentreY();
            float fillH = std::abs (srcVal) * (label.getHeight() * 0.45f);
            g.setColour (srcVal >= 0 ? juce::Colour (0xff00e8ff).withAlpha (0.7f)
                                     : juce::Colour (0xffff2d9b).withAlpha (0.7f));
            if (srcVal >= 0)
                g.fillRect (label.getX() + 2, mid - fillH, 4.0f, fillH);
            else
                g.fillRect (label.getX() + 2, mid, 4.0f, fillH);

            g.setColour (juce::Colour (0xff00e8ff));
            g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
            g.drawText (salek::ModMatrix::sourceName ((salek::ModMatrix::Source) s),
                        label.toNearestInt().withTrimmedLeft (8), juce::Justification::centredLeft);

            for (int d = 0; d < nd; ++d)
            {
                auto cell = cellRect (s, d);
                float amt = 0.f; bool active = false;
                for (auto& route : matrix.getRoutes())
                    if (route.active
                        && route.source == (salek::ModMatrix::Source) s
                        && route.dest == (salek::ModMatrix::Dest) d)
                    { amt = route.amount; active = true; break; }

                g.setColour (juce::Colour (0xff12101c));
                g.fillRoundedRectangle (cell, 3.0f);

                if (active && std::abs (amt) > 0.01f)
                {
                    const float midY = cell.getCentreY();
                    const float fillH2 = std::abs (amt) * (cell.getHeight() * 0.5f);
                    juce::Rectangle<float> fill;
                    if (amt >= 0.0f)
                        fill = { cell.getX() + 1.0f, midY - fillH2, cell.getWidth() - 2.0f, fillH2 };
                    else
                        fill = { cell.getX() + 1.0f, midY, cell.getWidth() - 2.0f, fillH2 };
                    juce::Colour col = amt >= 0.0f ? juce::Colour (0xff00e8ff) : juce::Colour (0xffff2d9b);
                    g.setColour (col.withAlpha (0.55f + std::abs (amt) * 0.4f));
                    g.fillRoundedRectangle (fill, 2.0f);
                    g.setColour (juce::Colours::white.withAlpha (0.9f));
                    g.setFont (juce::FontOptions (7.5f, juce::Font::bold));
                    g.drawText (juce::String (amt, 2), cell, juce::Justification::centred);
                }

                const bool isSel = (selSrc == s && selDst == d);
                const bool isDrag = (dragSrc == s && dragDst == d);
                g.setColour (isDrag || isSel ? juce::Colour (0xffffd700)
                                    : (active ? juce::Colour (0xff00e8ff).withAlpha (0.55f)
                                              : juce::Colour (0xff2a2840)));
                g.drawRoundedRectangle (cell, 3.0f, (isDrag || isSel) ? 1.8f : 0.8f);
            }
        }

        // ---- Serum-style amount strip ----
        auto strip = amountStrip();
        g.setColour (juce::Colour (0xff12101c));
        g.fillRoundedRectangle (strip, 6.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRoundedRectangle (strip, 6.f, 1.f);

        // Pure ASCII to avoid Windows font mojibake
        juce::String srcN = (selSrc >= 0) ? salek::ModMatrix::sourceName ((salek::ModMatrix::Source) selSrc) : "-";
        juce::String dstN = (selDst >= 0) ? salek::ModMatrix::destName ((salek::ModMatrix::Dest) selDst) : "-";
        float amt = selectedAmount();

        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.drawText (srcN + "  ->  " + dstN, strip.reduced (10, 4).removeFromTop (18), juce::Justification::centredLeft);

        auto track = strip.reduced (12.f, 8.f);
        track.removeFromTop (20.f);
        track = track.withHeight (14.f);
        g.setColour (juce::Colour (0xff1a1830));
        g.fillRoundedRectangle (track, 4.f);
        float midX = track.getCentreX();
        g.setColour (juce::Colour (0xff3a3850));
        g.fillRect (midX - 1.f, track.getY(), 2.f, track.getHeight());

        if (selSrc >= 0 && selDst >= 0)
        {
            float t = (amt + 1.f) * 0.5f;
            float kx = track.getX() + t * track.getWidth();
            juce::Colour col = amt >= 0 ? juce::Colour (0xff00e8ff) : juce::Colour (0xffff2d9b);
            g.setColour (col.withAlpha (0.5f));
            if (amt >= 0)
                g.fillRoundedRectangle ({ midX, track.getY(), kx - midX, track.getHeight() }, 3.f);
            else
                g.fillRoundedRectangle ({ kx, track.getY(), midX - kx, track.getHeight() }, 3.f);
            g.setColour (juce::Colours::white);
            g.fillEllipse (kx - 6.f, track.getCentreY() - 6.f, 12.f, 12.f);
            g.setColour (col);
            g.drawEllipse (kx - 6.f, track.getCentreY() - 6.f, 12.f, 12.f, 1.5f);
        }

        g.setColour (juce::Colour (0xffffd700));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.drawText (juce::String (amt, 2), strip.reduced (10, 4).removeFromRight (48), juce::Justification::centredRight);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (amountStrip().contains (e.position))
        {
            draggingStrip = true;
            setAmountFromStripX (e.position.x);
            repaint();
            return;
        }
        if (! hitCell (e.position, dragSrc, dragDst)) { dragSrc = dragDst = -1; return; }
        selSrc = dragSrc; selDst = dragDst;
        setAmountFromY (e.position.y);
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (draggingStrip) { setAmountFromStripX (e.position.x); repaint(); return; }
        if (dragSrc < 0 || dragDst < 0) return;
        setAmountFromY (e.position.y);
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragSrc = dragDst = -1;
        draggingStrip = false;
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        int s = -1, d = -1;
        if (! hitCell (e.position, s, d)) return;
        matrix.removeRoute ((salek::ModMatrix::Source) s, (salek::ModMatrix::Dest) d);
        if (selSrc == s && selDst == d) { selSrc = selDst = -1; }
        repaint();
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        int s = selSrc, d = selDst;
        if (s < 0 || d < 0)
            if (! hitCell (e.position, s, d)) return;
        selSrc = s; selDst = d;
        float amt = selectedAmount();
        amt = juce::jlimit (-1.f, 1.f, amt + wheel.deltaY * 0.08f);
        if (std::abs (amt) < 0.02f) amt = 0.f;
        matrix.addRoute ((salek::ModMatrix::Source) s, (salek::ModMatrix::Dest) d, amt);
        repaint();
    }

    void timerCallback() override { if (isShowing()) repaint(); }

private:
    salek::ModMatrix& matrix;
    int dragSrc = -1, dragDst = -1;
    int selSrc = -1, selDst = -1;
    bool draggingStrip = false;
    float gridLeft = 52.f, gridTop = 22.f, cellW = 28.f, cellH = 22.f;
    float stripH = 52.f;

    juce::Rectangle<float> amountStrip() const
    {
        return getLocalBounds().toFloat().reduced (6.f).removeFromBottom (stripH);
    }

    void layoutGrid()
    {
        const int ns = (int) salek::ModMatrix::Source::NumSources;
        const int nd = (int) salek::ModMatrix::Dest::NumDests;
        auto area = getLocalBounds().toFloat().reduced (4.f, 2.f);
        area.removeFromTop (18.f);
        area.removeFromBottom (stripH + 4.f);
        gridLeft = 52.f;
        gridTop = area.getY();
        cellW = juce::jmax (18.f, (area.getWidth() - gridLeft) / (float) nd);
        cellH = juce::jmax (14.f, area.getHeight() / (float) (ns + 1));
    }

    juce::Rectangle<float> cellRect (int s, int d) const
    {
        float x = gridLeft + (d < 0 ? -gridLeft : d * cellW);
        float y = gridTop + (s < 0 ? 0.f : (s + 1) * cellH);
        float w = (d < 0 ? gridLeft - 2.f : cellW - 2.f);
        float h = cellH - 2.f;
        return { x + 1.f, y + 1.f, w, h };
    }

    bool hitCell (juce::Point<float> p, int& sOut, int& dOut) const
    {
        const int ns = (int) salek::ModMatrix::Source::NumSources;
        const int nd = (int) salek::ModMatrix::Dest::NumDests;
        for (int s = 0; s < ns; ++s)
            for (int d = 0; d < nd; ++d)
                if (cellRect (s, d).contains (p))
                {
                    sOut = s; dOut = d;
                    return true;
                }
        return false;
    }

    float selectedAmount() const
    {
        if (selSrc < 0 || selDst < 0) return 0.f;
        for (auto& route : matrix.getRoutes())
            if (route.active
                && route.source == (salek::ModMatrix::Source) selSrc
                && route.dest == (salek::ModMatrix::Dest) selDst)
                return route.amount;
        return 0.f;
    }

    void setAmountFromY (float y)
    {
        if (dragSrc < 0 || dragDst < 0) return;
        auto cell = cellRect (dragSrc, dragDst);
        float t = 1.0f - (y - cell.getY()) / juce::jmax (1.0f, cell.getHeight());
        float amt = juce::jlimit (-1.0f, 1.0f, t * 2.0f - 1.0f);
        if (std::abs (amt) < 0.03f) amt = 0.0f;
        matrix.addRoute ((salek::ModMatrix::Source) dragSrc,
                         (salek::ModMatrix::Dest) dragDst, amt);
    }

    void setAmountFromStripX (float x)
    {
        if (selSrc < 0 || selDst < 0) return;
        auto track = amountStrip().reduced (12.f, 8.f);
        track.removeFromTop (20.f);
        float t = (x - track.getX()) / juce::jmax (1.f, track.getWidth());
        float amt = juce::jlimit (-1.f, 1.f, t * 2.f - 1.f);
        if (std::abs (amt) < 0.03f) amt = 0.f;
        matrix.addRoute ((salek::ModMatrix::Source) selSrc,
                         (salek::ModMatrix::Dest) selDst, amt);
    }
};
