#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

/** Compact route list inspired by the HTML reference: LFO activity, routes,
    draggable depth bars, and explicit add/clear controls. */
class ModMatrixPanel : public juce::Component, private juce::Timer
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m)
    {
        for (int i = 0; i < (int) salek::ModMatrix::Source::NumSources; ++i)
            sourceBox.addItem (salek::ModMatrix::sourceName ((salek::ModMatrix::Source) i), i + 1);
        for (int i = 0; i < (int) salek::ModMatrix::Dest::NumDests; ++i)
            destBox.addItem (salek::ModMatrix::destName ((salek::ModMatrix::Dest) i), i + 1);
        sourceBox.setSelectedId (1);
        destBox.setSelectedId (1);
        addButton.setButtonText ("+ ADD");
        clearButton.setButtonText ("CLR");
        for (auto* b : { &addButton, &clearButton })
        {
            b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff171026));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            addAndMakeVisible (*b);
        }
        sourceBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff100b1d));
        sourceBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xff00e8ff));
        destBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff100b1d));
        destBox.setColour (juce::ComboBox::textColourId, juce::Colours::white);
        addAndMakeVisible (sourceBox);
        addAndMakeVisible (destBox);
        addButton.onClick = [this]
        {
            const auto src = (salek::ModMatrix::Source) juce::jmax (0, sourceBox.getSelectedId() - 1);
            const auto dst = (salek::ModMatrix::Dest) juce::jmax (0, destBox.getSelectedId() - 1);
            matrix.addRoute (src, dst, 0.5f);
            selectedSource = (int) src;
            selectedDest = (int) dst;
            repaint();
        };
        clearButton.onClick = [this] { matrix.clear(); selectedSource = selectedDest = -1; repaint(); };
        startTimerHz (12);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6, 3);
        r.removeFromTop (53);
        auto controls = r.removeFromTop (25);
        const int w = controls.getWidth();
        sourceBox.setBounds (controls.removeFromLeft (juce::jmin (w / 3, 180)).reduced (1));
        destBox.setBounds (controls.removeFromLeft (juce::jmin (w / 2, 220)).reduced (1));
        clearButton.setBounds (controls.removeFromRight (46).reduced (1));
        addButton.setBounds (controls.removeFromRight (62).reduced (1));
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.93f));
        g.fillRoundedRectangle (r, 8.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.38f));
        g.drawRoundedRectangle (r, 8.f, 1.2f);
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.setColour (juce::Colour (0xffffd700));
        g.drawText ("MOD ROUTES", 10, 3, 140, 17, juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffc0a0d0));
        g.setFont (juce::FontOptions (9.f));
        g.drawText ("drag a bar to set depth  |  wheel = fine adjust  |  X = remove",
                    130, 3, juce::jmax (80, getWidth() - 140), 17, juce::Justification::centredLeft);

        const juce::Colour cols[] = { juce::Colour (0xff00e8ff), juce::Colour (0xffff2d9b), juce::Colour (0xff39ff14) };
        auto srcArea = getLocalBounds().reduced (8, 0).withY (20).withHeight (27).toFloat();
        const float sw = srcArea.getWidth() / 3.f;
        for (int i = 0; i < 3; ++i)
        {
            auto cell = srcArea.withX (srcArea.getX() + i * sw).withWidth (sw - 4.f);
            g.setColour (juce::Colour (0xff151020));
            g.fillRoundedRectangle (cell, 4.f);
            const float value = juce::jlimit (-1.f, 1.f, matrix.getSourceValue ((salek::ModMatrix::Source) i));
            auto track = cell.reduced (5.f, 5.f);
            g.setColour (cols[i].withAlpha (0.8f));
            const float half = track.getCentreX();
            const float edge = half + value * track.getWidth() * 0.5f;
            g.fillRoundedRectangle (juce::Rectangle<float> (juce::jmin (half, edge), track.getY(),
                                      juce::jmax (1.f, std::abs (edge - half)), track.getHeight()), 3.f);
            g.setColour (cols[i]);
            g.setFont (juce::FontOptions (9.f, juce::Font::bold));
            g.drawText (juce::String ("LFO") + juce::String (i + 1), cell.toNearestInt().withTrimmedRight (cell.getWidth() * 0.72f), juce::Justification::centredLeft);
        }

        const auto& routes = matrix.getRoutes();
        const int rowTop = 82;
        const int rowH = 25;
        int shown = 0;
        for (int slot = 0; slot < salek::ModMatrix::MaxRoutes; ++slot)
        {
            const auto& route = routes[(size_t) slot];
            if (! route.active) continue;
            if (shown++ < routeOffset) continue;
            const int y = rowTop + (shown - routeOffset - 1) * rowH;
            if (y + rowH > getHeight() - 2) break;
            auto row = juce::Rectangle<float> (8.f, (float) y, getWidth() - 16.f, rowH - 2.f);
            const int src = (int) route.source;
            const int dst = (int) route.dest;
            const bool selected = src == selectedSource && dst == selectedDest;
            g.setColour (selected ? juce::Colour (0xff17142a) : juce::Colour (0xff100d1b));
            g.fillRoundedRectangle (row, 4.f);
            const auto col = cols[src >= 0 && src < 3 ? src : 0];
            g.setColour (col);
            g.setFont (juce::FontOptions (9.f, juce::Font::bold));
            const juce::String sourceName = salek::ModMatrix::sourceName (route.source);
            const juce::String destName = salek::ModMatrix::destName (route.dest);
            g.drawText (sourceName + "  ->  " + destName, row.reduced (5.f, 0.f).withWidth (150.f).toNearestInt(), juce::Justification::centredLeft);
            auto bar = row.withX (row.getX() + 158.f).withWidth (juce::jmax (25.f, row.getWidth() - 218.f)).reduced (1.f, 6.f);
            g.setColour (juce::Colour (0xff252137));
            g.fillRoundedRectangle (bar, 3.f);
            const float mid = bar.getCentreX();
            const float end = mid + juce::jlimit (-1.f, 1.f, route.amount) * bar.getWidth() * 0.5f;
            g.setColour (route.amount >= 0.f ? col : juce::Colour (0xffff2d9b));
            g.fillRoundedRectangle ({ juce::jmin (mid, end), bar.getY(), juce::jmax (1.f, std::abs (end - mid)), bar.getHeight() }, 3.f);
            g.setColour (juce::Colours::white);
            g.setFont (juce::FontOptions (9.f));
            g.drawText (juce::String (route.amount, 2), (int) row.getRight() - 47, y, 31, rowH - 2, juce::Justification::centredRight);
            g.setColour (juce::Colour (0xffff2d9b));
            g.drawText ("X", (int) row.getRight() - 22, y, 16, rowH - 2, juce::Justification::centred);
            if (selected) selectedSlot = slot;
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        int slot = routeAt (e.position.y);
        if (slot < 0) return;
        auto& route = matrix.getRoute (slot);
        selectedSlot = slot;
        selectedSource = (int) route.source;
        selectedDest = (int) route.dest;
        if (e.position.x > getWidth() - 34.f || e.mods.isRightButtonDown())
        {
            matrix.removeRoute (slot);
            if (slot == selectedSlot) selectedSlot = -1;
            repaint();
            return;
        }
        dragging = true;
        setAmountFromX (e.position.x);
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! dragging) return;
        setAmountFromX (e.position.x);
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override { dragging = false; }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const int slot = routeAt (e.position.y);
        if (slot < 0) { routeOffset = juce::jmax (0, routeOffset + (wheel.deltaY < 0.f ? 1 : -1)); repaint(); return; }
        auto& route = matrix.getRoute (slot);
        selectedSlot = slot;
        selectedSource = (int) route.source;
        selectedDest = (int) route.dest;
        route.amount = juce::jlimit (-1.f, 1.f, route.amount + wheel.deltaY * 0.06f);
        repaint();
    }

private:
    salek::ModMatrix& matrix;
    juce::ComboBox sourceBox, destBox;
    juce::TextButton addButton, clearButton;
    int selectedSlot = -1, selectedSource = -1, selectedDest = -1, routeOffset = 0;
    bool dragging = false;

    int routeAt (float y) const noexcept
    {
        if (y < 82.f) return -1;
        const int row = (int) ((y - 82.f) / 25.f) + routeOffset;
        int seen = 0;
        const auto& routes = matrix.getRoutes();
        for (int i = 0; i < salek::ModMatrix::MaxRoutes; ++i)
            if (routes[(size_t) i].active && seen++ == row) return i;
        return -1;
    }

    void setAmountFromX (float x) noexcept
    {
        if (selectedSlot < 0) return;
        auto& route = matrix.getRoute (selectedSlot);
        const float left = 174.f;
        const float right = (float) getWidth() - 72.f;
        route.amount = juce::jlimit (-1.f, 1.f, (x - (left + right) * 0.5f) / juce::jmax (1.f, (right - left) * 0.5f));
        if (std::abs (route.amount) < 0.015f) route.amount = 0.f;
    }

    void timerCallback() override { if (isShowing()) repaint(); }
};
