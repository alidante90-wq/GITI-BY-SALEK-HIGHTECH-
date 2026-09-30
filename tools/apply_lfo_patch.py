#!/usr/bin/env python3
"""Apply LFO drag-drop improvements to Source/PluginEditorFullA.inl"""
from pathlib import Path
import sys

path = Path("Source/PluginEditorFullA.inl")
if not path.exists():
    print("ERROR: run from repo root (GITI-BY-SALEK-HIGHTECH-)")
    sys.exit(1)

text = path.read_text(encoding="utf-8", errors="replace")
if "startDragging" in text and "getComponentAt" in text:
    print("Already patched.")
    sys.exit(0)

old_drag = """            ed->armedModSource = src;
            ed->isModDragging = true;
            ed->setMouseCursor (juce::MouseCursor::CopyingCursor);
            // Keep the source's mouse capture until release. The host's
            // drag manager can steal that capture in several DAWs, so route
            // the release against the screen-space pointer directly instead."""

new_drag = """            ed->armedModSource = src;
            ed->isModDragging = true;
            ed->setMouseCursor (juce::MouseCursor::CopyingCursor);
            if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor (e.eventComponent))
                container->startDragging ("SALEK_LFO" + juce::String (src), e.eventComponent);"""

if old_drag not in text:
    print("ERROR: drag block not found (file may differ)")
    sys.exit(1)
text = text.replace(old_drag, new_drag)

a0 = text.find("void SalekHightechAudioProcessorEditor::assignModToParam")
a1 = text.find("void SalekHightechAudioProcessorEditor::addCombo")
if a0 < 0 or a1 < 0 or a1 <= a0:
    print("ERROR: assignModToParam bounds not found")
    sys.exit(1)

new_assign = r'''void SalekHightechAudioProcessorEditor::assignModToParam (const juce::String& paramId, float amount)
{
    if (armedModSource < 0 || armedModSource > 2) return;

    using D = salek::ModMatrix::Dest;
    using S = salek::ModMatrix::Source;
    D dest = D::NumDests;
    const auto id = paramId;

    if      (id == "filter_cutoff") dest = D::FilterCutoff;
    else if (id == "filter_reso" || id == "filter_res") dest = D::FilterReso;
    else if (id == "filter_env") dest = D::FilterEnv;
    else if (id == "osc1_level") dest = D::Osc1Level;
    else if (id == "osc2_level") dest = D::Osc2Level;
    else if (id == "osc3_level") dest = D::Osc3Level;
    else if (id == "osc1_table" || id == "osc1_wtpos") dest = D::Osc1Table;
    else if (id == "osc2_table" || id == "osc2_wtpos") dest = D::Osc2Table;
    else if (id == "osc3_table" || id == "osc3_wtpos") dest = D::Osc3Table;
    else if (id == "osc1_warp") dest = D::Osc1Warp;
    else if (id == "osc2_warp") dest = D::Osc2Warp;
    else if (id == "osc3_warp") dest = D::Osc3Warp;
    else if (id == "osc1_fold") dest = D::Osc1Fold;
    else if (id == "osc2_fold") dest = D::Osc2Fold;
    else if (id == "osc3_fold") dest = D::Osc3Fold;
    else if (id == "osc1_pan") dest = D::Osc1Pan;
    else if (id == "osc2_pan") dest = D::Osc2Pan;
    else if (id == "osc3_pan") dest = D::Osc3Pan;
    else if (id == "osc1_drive") dest = D::Osc1Drive;
    else if (id == "osc2_drive") dest = D::Osc2Drive;
    else if (id == "osc3_drive") dest = D::Osc3Drive;
    else if (id == "fm_2to1" || id == "osc1_fm") dest = D::Fm2to1;
    else if (id == "fm_3to1") dest = D::Fm3to1;
    else if (id == "delay_mix") dest = D::DelayMix;
    else if (id == "reverb_mix") dest = D::ReverbMix;
    else if (id == "dist_drive" || id == "dist_mix") dest = D::DistDrive;
    else if (id == "chorus_mix") dest = D::ChorusMix;
    else if (id == "phaser_mix") dest = D::PhaserMix;
    else if (id == "bassify") dest = D::Bassify;
    else if (id == "magic_x") dest = D::MagicX;
    else if (id == "magic_y") dest = D::MagicY;
    else if (id == "master_gain" || id == "amp_level") dest = D::Amp;
    else if (id == "osc1_fine" || id == "osc1_detune" || id == "glide") dest = D::Pitch;
    else if (id == "osc1_unison") dest = D::Osc1Level;
    else if (id == "osc1_udet") dest = D::Osc1Warp;
    else if (id == "osc1_uspread") dest = D::Osc1Pan;
    else if (id == "osc2_unison") dest = D::Osc2Level;
    else if (id == "osc2_udet") dest = D::Osc2Warp;
    else if (id == "osc3_unison") dest = D::Osc3Level;
    else if (id == "master_drive") dest = D::DistDrive;
    else if (id == "sub_level") dest = D::Osc1Level;
    else if (id == "noise_level") dest = D::Osc3Level;
    else return;

    S src = (armedModSource == 0) ? S::LFO1 : (armedModSource == 1) ? S::LFO2 : S::LFO3;
    if (std::abs (amount) < 1e-4f)
        processor.getModMatrix().removeRoute (src, dest);
    else
        processor.getModMatrix().addRoute (src, dest, juce::jlimit (-1.f, 1.f, amount));

    if (matrixPanel != nullptr)
        matrixPanel->repaint();
    repaint();
}

'''
text = text[:a0] + new_assign + text[a1:]

old_try = """void SalekHightechAudioProcessorEditor::tryAssignModAt (juce::Point<int> editorPos, float amount, const juce::ModifierKeys&)
{
    if (armedModSource < 0) return;
    for (auto& k : knobs)
    {
        if (k == nullptr || ! k->s.isShowing()) continue;
        auto r = getLocalArea (&k->s, k->s.getLocalBounds());
        auto rn = getLocalArea (&k->name, k->name.getLocalBounds());
        if (r.contains (editorPos) || rn.contains (editorPos))
        {
            assignModToParam (k->paramId, amount);
            return;
        }
    }
}"""

new_try = """void SalekHightechAudioProcessorEditor::tryAssignModAt (juce::Point<int> editorPos, float amount, const juce::ModifierKeys&)
{
    if (armedModSource < 0) return;

    if (auto* hit = getComponentAt (editorPos))
    {
        for (auto* c = hit; c != nullptr && c != this; c = c->getParentComponent())
        {
            for (auto& k : knobs)
            {
                if (k == nullptr) continue;
                if (c == &k->s || c == &k->name)
                {
                    assignModToParam (k->paramId, amount);
                    return;
                }
            }
        }
    }

    for (auto& k : knobs)
    {
        if (k == nullptr || ! k->s.isShowing()) continue;
        auto r = getLocalArea (&k->s, k->s.getLocalBounds()).expanded (6);
        auto rn = getLocalArea (&k->name, k->name.getLocalBounds()).expanded (4);
        if (r.contains (editorPos) || rn.contains (editorPos))
        {
            assignModToParam (k->paramId, amount);
            return;
        }
    }
}"""

if old_try not in text:
    print("ERROR: tryAssignModAt block not found")
    sys.exit(1)
text = text.replace(old_try, new_try)

path.write_text(text, encoding="utf-8")
print("OK: patched", path, "size", len(text))
print("Now run:")
print("  git add Source/PluginEditorFullA.inl")
print('  git commit -m "SHAE UI: LFO drag-drop on all knobs"')
print("  git push origin giti-by-salek-hightech")
