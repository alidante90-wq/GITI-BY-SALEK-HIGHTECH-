#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

namespace salek
{

/** Map APVTS parameter id → ModMatrix destination (for LFO drop targets). */
inline bool paramIdToDest (const juce::String& id, ModMatrix::Dest& out) noexcept
{
    if (id == "filter_cutoff") { out = ModMatrix::Dest::FilterCutoff; return true; }
    if (id == "filter_reso" || id == "filter_res") { out = ModMatrix::Dest::FilterReso; return true; }
    if (id == "filter_env") { out = ModMatrix::Dest::FilterEnv; return true; }
    if (id == "osc1_level") { out = ModMatrix::Dest::Osc1Level; return true; }
    if (id == "osc2_level") { out = ModMatrix::Dest::Osc2Level; return true; }
    if (id == "osc3_level") { out = ModMatrix::Dest::Osc3Level; return true; }
    if (id == "osc1_table" || id == "osc1_wtpos") { out = ModMatrix::Dest::Osc1Table; return true; }
    if (id == "osc2_table" || id == "osc2_wtpos") { out = ModMatrix::Dest::Osc2Table; return true; }
    if (id == "osc3_table" || id == "osc3_wtpos") { out = ModMatrix::Dest::Osc3Table; return true; }
    if (id == "osc1_warp") { out = ModMatrix::Dest::Osc1Warp; return true; }
    if (id == "osc2_warp") { out = ModMatrix::Dest::Osc2Warp; return true; }
    if (id == "osc3_warp") { out = ModMatrix::Dest::Osc3Warp; return true; }
    if (id == "osc1_fold") { out = ModMatrix::Dest::Osc1Fold; return true; }
    if (id == "osc2_fold") { out = ModMatrix::Dest::Osc2Fold; return true; }
    if (id == "osc3_fold") { out = ModMatrix::Dest::Osc3Fold; return true; }
    if (id == "osc1_drive") { out = ModMatrix::Dest::Osc1Drive; return true; }
    if (id == "osc2_drive") { out = ModMatrix::Dest::Osc2Drive; return true; }
    if (id == "osc3_drive") { out = ModMatrix::Dest::Osc3Drive; return true; }
    if (id == "osc1_pan") { out = ModMatrix::Dest::Osc1Pan; return true; }
    if (id == "osc2_pan") { out = ModMatrix::Dest::Osc2Pan; return true; }
    if (id == "osc3_pan") { out = ModMatrix::Dest::Osc3Pan; return true; }
    if (id == "fm_2to1" || id == "osc1_fm") { out = ModMatrix::Dest::Fm2to1; return true; }
    if (id == "fm_3to1") { out = ModMatrix::Dest::Fm3to1; return true; }
    if (id == "amp_level" || id == "master_gain") { out = ModMatrix::Dest::Amp; return true; }
    if (id == "delay_mix") { out = ModMatrix::Dest::DelayMix; return true; }
    if (id == "reverb_mix") { out = ModMatrix::Dest::ReverbMix; return true; }
    if (id == "chorus_mix") { out = ModMatrix::Dest::ChorusMix; return true; }
    if (id == "dist_drive" || id == "dist_mix") { out = ModMatrix::Dest::DistDrive; return true; }
    if (id == "phaser_mix") { out = ModMatrix::Dest::PhaserMix; return true; }
    if (id == "bassify") { out = ModMatrix::Dest::Bassify; return true; }
    if (id == "magic_x") { out = ModMatrix::Dest::MagicX; return true; }
    if (id == "magic_y") { out = ModMatrix::Dest::MagicY; return true; }
    // Pitch-ish
    if (id == "osc1_fine" || id == "osc1_detune" || id == "glide") { out = ModMatrix::Dest::Pitch; return true; }
    return false;
}

/** Drag payload: "SHAE_MOD_SRC:<sourceIndex>" */
inline juce::String modSourceDragDescription (int sourceIndex) 
{
    return "SHAE_MOD_SRC:" + juce::String (sourceIndex);
}

inline int parseModSourceDrag (const juce::String& desc) noexcept
{
    if (! desc.startsWith ("SHAE_MOD_SRC:")) return -1;
    return desc.fromFirstOccurrenceOf (":", false, false).getIntValue();
}

/** Rotary that accepts LFO/source drops and creates matrix routes. */
class ModDropSlider : public juce::Slider,
                      public juce::DragAndDropTarget
{
public:
    ModDropSlider() = default;

    void setModContext (ModMatrix* m, juce::String parameterId)
    {
        matrix = m;
        paramId = std::move (parameterId);
    }

    bool isInterestedInDragSource (const SourceDetails& details) override
    {
        return matrix != nullptr && parseModSourceDrag (details.description.toString()) >= 0
               && paramIdToDest (paramId, dropDestScratch);
    }

    void itemDragEnter (const SourceDetails&) override { dropHighlight = true;  repaint(); }
    void itemDragExit  (const SourceDetails&) override { dropHighlight = false; repaint(); }

    void itemDropped (const SourceDetails& details) override
    {
        dropHighlight = false;
        const int src = parseModSourceDrag (details.description.toString());
        ModMatrix::Dest dest;
        if (matrix == nullptr || src < 0 || ! paramIdToDest (paramId, dest))
        {
            repaint();
            return;
        }
        matrix->addRoute ((ModMatrix::Source) src, dest, 0.45f);
        flashUntil = juce::Time::getMillisecondCounter() + 450;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        juce::Slider::paint (g);
        if (dropHighlight)
        {
            g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.35f));
            g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.f), 8.f, 2.5f);
        }
        else if (juce::Time::getMillisecondCounter() < flashUntil)
        {
            g.setColour (juce::Colour (0xff39ff14).withAlpha (0.45f));
            g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.f), 8.f, 2.f);
        }
    }

private:
    ModMatrix* matrix = nullptr;
    juce::String paramId;
    ModMatrix::Dest dropDestScratch = ModMatrix::Dest::FilterCutoff;
    bool dropHighlight = false;
    uint32_t flashUntil = 0;
};

/** Text button that starts a mod-source drag (LFO1/2/3 …). */
class ModSourceDragButton : public juce::TextButton
{
public:
    void setSourceIndex (int idx) noexcept { sourceIndex = idx; }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        juce::TextButton::mouseDrag (e);
        if (e.getDistanceFromDragStart() > 6)
        {
            if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor (this))
            {
                container->startDragging (modSourceDragDescription (sourceIndex), this);
            }
        }
    }

private:
    int sourceIndex = 0;
};

} // namespace salek
