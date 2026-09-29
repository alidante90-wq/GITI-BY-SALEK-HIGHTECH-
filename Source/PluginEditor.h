#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/ModMatrixPanel.h"
#include "UI/OpenGLGridBackdrop.h"
#include "UI/SonicCoreGL.h"
#include "UI/MagicPad.h"
#include "UI/OscShapeMonitor.h"

#include "PluginEditorLookAndFeel.inl"
#include "PluginEditorFxLfo.inl"

class WavetableDisplay : public juce::Component, private juce::Timer {
public:
    explicit WavetableDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (3); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { if (isShowing()) repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void WavetableDisplay::paint (juce::Graphics& g) {
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0c0818)); g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f)); g.drawRoundedRectangle (r, 6.0f, 1.f);
    auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
    float pos=gval("osc1_table",0.f);
    juce::Path wave; const int N=64;
    for(int i=0;i<N;++i){ float t=(float)i/(N-1); float y=std::sin(t*juce::MathConstants<float>::twoPi*(1.f+pos*4.f));
        float px=r.getX()+8+t*(r.getWidth()-16); float py=r.getCentreY()-y*(r.getHeight()*0.35f);
        if(i==0)wave.startNewSubPath(px,py); else wave.lineTo(px,py);}
    g.setColour(juce::Colour(0xff00e8ff)); g.strokePath(wave, juce::PathStrokeType(1.5f));
}

#include "PluginEditorVisualizers.inl"

class AdsrDisplay : public juce::Component, private juce::Timer {
public:
    explicit AdsrDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (3); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { if (isShowing()) repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void AdsrDisplay::paint (juce::Graphics& g) {
    auto r=getLocalBounds().toFloat().reduced(2.f);
    g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,6.f);
    g.setColour(juce::Colour(0xff00e8ff).withAlpha(0.35f)); g.drawRoundedRectangle(r,6.f,1.f);
    auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
    float a=gval("amp_attack",0.01f), d=gval("amp_decay",0.2f), s=gval("amp_sustain",0.7f), rel=gval("amp_release",0.3f);
    auto mapT=[&](float t){ return std::pow (juce::jlimit(0.001f,1.f, t / 2.f), 0.55f); };
    float wa=mapT(a), wd=mapT(d), wr=mapT(rel), ws=0.28f;
    float sum=wa+wd+ws+wr; auto plot=r.reduced(8.f,6.f);
    juce::Path env;
    float x0=plot.getX(), y0=plot.getBottom();
    float x1=x0+plot.getWidth()*(wa/sum), y1=plot.getY();
    float x2=x1+plot.getWidth()*(wd/sum), y2=plot.getY()+plot.getHeight()*(1.f-s);
    float x3=x2+plot.getWidth()*(ws/sum), y3=y2;
    float x4=plot.getRight(), y4=plot.getBottom();
    env.startNewSubPath(x0,y0); env.lineTo(x1,y1); env.lineTo(x2,y2); env.lineTo(x3,y3); env.lineTo(x4,y4);
    g.setColour(juce::Colour(0xff00e8ff)); g.strokePath(env, juce::PathStrokeType(1.6f));
}

class FilterCurveDisplay : public juce::Component, private juce::Timer {
public:
    explicit FilterCurveDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (3); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { if (isShowing()) repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void FilterCurveDisplay::paint (juce::Graphics& g) {
    auto r = getLocalBounds().toFloat().reduced (2.f);
    g.setColour (juce::Colour (0xff0c0818));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.45f));
    g.drawRoundedRectangle (r, 6.f, 1.2f);

    auto gval = [&] (const char* id, float d) {
        if (auto* p = apvts.getRawParameterValue (id)) return p->load();
        return d;
    };
    // Map cutoff 20 Hz .. 20 kHz onto log X axis so knob motion is visible
    const float cutHz = juce::jlimit (20.f, 20000.f, gval ("filter_cutoff", 1000.f));
    const float reso  = juce::jlimit (0.f, 1.f, gval ("filter_reso", 0.3f));
    const int fmode = (int) gval ("filter_mode", 0.f);
    const float logMin = std::log (20.f);
    const float logMax = std::log (20000.f);
    const float norm = juce::jlimit (0.f, 1.f, (std::log (cutHz) - logMin) / (logMax - logMin));

    auto plot = r.reduced (6.f, 4.f);
    juce::Path curve;
    const float qSharp = 8.f + reso * 90.f;
    const float peakH  = 0.12f + reso * 0.72f;
    // 0 LP12 1 LP24 2 HP12 3 HP24 4 BP 5 Notch 6 Peak 7 AllPass
    // 8 Acid 9 Ladder 10 Comb 11 Formant 12 BandRej 13 LoShelf 14 HiShelf 15 PhaserN

    for (int i = 0; i < 128; ++i)
    {
        const float t = (float) i / 127.f;
        const float x = plot.getX() + t * plot.getWidth();
        const float dist = t - norm;
        float yNorm = 0.42f;

        auto lp = [&]() {
            float y = 0.42f;
            if (dist > 0.f) y = 0.42f * std::exp (-dist * dist * (12.f + (1.f - reso) * 28.f));
            else y = 0.42f + 0.06f * (1.f - std::exp (dist * 5.f));
            y += peakH * std::exp (-dist * dist * qSharp);
            return y;
        };
        auto hp = [&]() {
            float y = 0.15f;
            if (dist < 0.f) y = 0.15f * std::exp (-dist * dist * (12.f + (1.f - reso) * 28.f));
            else y = 0.55f - 0.05f * std::exp (-dist * 4.f);
            y += peakH * 0.7f * std::exp (-dist * dist * qSharp);
            return y;
        };
        auto bp = [&]() {
            return 0.12f + peakH * std::exp (-dist * dist * (qSharp * 0.7f));
        };
        auto notch = [&]() {
            float y = 0.5f - peakH * std::exp (-dist * dist * qSharp);
            return juce::jmax (0.05f, y);
        };

        switch (fmode)
        {
            case 2: case 3: yNorm = hp(); break;
            case 4: yNorm = bp(); break;
            case 5: case 12: case 15: yNorm = notch(); break;
            case 16: // DiodeLP
                yNorm = 0.35f + (lp() - 0.35f) * (0.8f + 0.35f * reso);
                break;
            case 17: // Sallen-Key
                yNorm = 0.5f * lp() + 0.5f * (0.35f + peakH * 0.35f * std::exp (-dist * dist * qSharp));
                break;
            case 18: // Vocal
                yNorm = 0.18f + peakH * 0.72f * std::exp (-dist * dist * qSharp)
                      + peakH * 0.52f * std::exp (-(dist - 0.16f) * (dist - 0.16f) * qSharp);
                break;
            case 19: // RingMod
                yNorm = 0.42f + 0.22f * std::sin (t * 26.f + reso * 5.f);
                break;
            case 20: // BandPass-Q
                yNorm = 0.10f + peakH * 1.15f * std::exp (-dist * dist * (qSharp * 1.8f));
                break;
            case 21: // LP + HP parallel
                yNorm = 0.5f * (lp() + hp());
                break;
            case 22: // Deep notch
                yNorm = juce::jmax (0.03f, 0.50f - peakH * 1.15f * std::exp (-dist * dist * (qSharp * 1.35f)));
                break;
            case 23: // Wide shelf
                yNorm = 0.5f + (t < norm ? 0.10f : -0.10f) + 0.18f * (1.f - reso);
                break;
            case 6: yNorm = 0.35f + peakH * std::exp (-dist * dist * qSharp); break;
            case 10: // comb ripples
                yNorm = 0.35f + 0.25f * std::sin (t * 40.f) * (0.4f + reso)
                      + peakH * 0.4f * std::exp (-dist * dist * 30.f);
                break;
            case 11: // formant dual peak
                yNorm = 0.2f + peakH * 0.7f * std::exp (-dist * dist * qSharp)
                      + peakH * 0.45f * std::exp (-(dist - 0.12f) * (dist - 0.12f) * qSharp);
                break;
            case 13: // lo shelf
                yNorm = (t < norm) ? (0.55f + reso * 0.2f) : 0.35f;
                break;
            case 14: // hi shelf
                yNorm = (t > norm) ? (0.55f + reso * 0.2f) : 0.35f;
                break;
            case 1: case 9: case 8: // steeper LP
                yNorm = lp();
                if (dist > 0.f) yNorm *= std::exp (-dist * 2.5f);
                break;
            default: yNorm = lp(); break;
        }

        const float y = plot.getBottom() - juce::jlimit (0.04f, 0.96f, yNorm) * plot.getHeight();
        if (i == 0) curve.startNewSubPath (x, y);
        else        curve.lineTo (x, y);
    }

    g.setColour (juce::Colour (0xffff2d9b));
    g.strokePath (curve, juce::PathStrokeType (2.0f));

    // Cutoff marker + RES amount indicator
    const float mx = plot.getX() + norm * plot.getWidth();
    g.setColour (juce::Colour (0xffffd700).withAlpha (0.65f));
    g.drawVerticalLine ((int) mx, plot.getY(), plot.getBottom());
    g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.35f + reso * 0.45f));
    g.fillEllipse (mx - 3.f - reso * 4.f, plot.getY() + 4.f, 6.f + reso * 8.f, 6.f + reso * 4.f);
}

class LfoDisplay : public juce::Component, private juce::Timer {
public:
    explicit LfoDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (3); }
    void paint (juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (juce::Colour (0xff0c0818)); g.fillRoundedRectangle (bounds, 8.f);
        g.setColour (juce::Colour (0xff66ff99).withAlpha (0.4f)); g.drawRoundedRectangle (bounds, 8.f, 1.f);
        auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
        const char* rates[] = {"lfo_rate","lfo2_rate","lfo3_rate"};
        const char* amts[]  = {"lfo_amount","lfo2_amount","lfo3_amount"};
        const char* waves[] = {"lfo_wave","lfo2_wave","lfo3_wave"};
        juce::Colour cols[3] = { juce::Colour(0xff00e8ff), juce::Colour(0xffff2d9b), juce::Colour(0xff39ff14) };
        const char* labels[3] = { "LFO1", "LFO2", "LFO3" };
        float w = bounds.getWidth() / 3.f;
        for (int L = 0; L < 3; ++L)
        {
            auto r = bounds.withX (bounds.getX() + L * w).withWidth (w).reduced (4.f, 3.f);
            g.setColour (cols[L].withAlpha (0.25f)); g.drawRoundedRectangle (r, 6.f, 1.f);
            float rate = gval (rates[L], 1.f); int wave = (int) gval (waves[L], 0.f); float amt = gval (amts[L], 0.5f);
            g.setColour (cols[L]); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
            g.drawText (juce::String (labels[L]) + "  " + juce::String (rate, 2) + "Hz",
                        r.removeFromTop (14).toNearestInt(), juce::Justification::centred);
            auto plot = r.reduced (2.f, 2.f);
            juce::Path curve; const int N = 64;
            float spd = juce::jmax (0.05f, rate) * 0.15f;
            for (int i = 0; i < N; ++i)
            {
                float t = (float) i / (N - 1);
                float ph = t * juce::MathConstants<float>::twoPi * 2.f + phase * spd * (1.f + L * 0.3f);
                float y = 0.f;
                if (wave == 0) y = std::sin (ph);
                else if (wave == 1) y = 1.f - 4.f * std::abs (std::fmod (ph / juce::MathConstants<float>::twoPi + 0.25f, 1.f) - 0.5f);
                else if (wave == 2) y = 2.f * (ph / juce::MathConstants<float>::twoPi - std::floor (ph / juce::MathConstants<float>::twoPi + 0.5f));
                else if (wave == 3) y = (std::sin (ph) >= 0.f ? 1.f : -1.f);
                else y = (std::sin (ph * 0.37f) > 0.f ? 1.f : -1.f);
                y *= (0.25f + 0.75f * amt);
                float px = plot.getX() + t * plot.getWidth();
                float py = plot.getCentreY() - y * plot.getHeight() * 0.4f;
                if (i == 0) curve.startNewSubPath (px, py); else curve.lineTo (px, py);
            }
            g.setColour (cols[L]); g.strokePath (curve, juce::PathStrokeType (1.6f));
        }
    }
    void timerCallback() override { if (! isShowing()) return; phase += 0.12f; repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts; float phase = 0.f;
};

#include "PluginEditorStepGrid.inl"

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                  public juce::DragAndDropContainer,
                                          public juce::Timer,
                                          public juce::ListBoxModel
{
public:
    explicit SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
    SpectrumDisplay spectrum;
    std::unique_ptr<WavetableDisplay> wtDisplay;
    std::unique_ptr<AdsrDisplay> adsrDisplay;
    std::unique_ptr<FilterCurveDisplay> filterDisplay;
    std::unique_ptr<LfoDisplay> lfoDisplay;
    std::unique_ptr<ModMatrixPanel> matrixPanel;
    std::unique_ptr<OpenGLGridBackdrop> glBackdrop;
    std::unique_ptr<SonicCoreGL> sonicCore;
    std::unique_ptr<OscShapeMonitor> oscMon1, oscMon2, oscMon3;
    juce::VBlankAttachment vblank;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    struct Knob { juce::Slider s; juce::Label name; juce::String paramId; };
    std::vector<std::unique_ptr<Knob>> knobs;
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    juce::Component mainTab, oscTab, filterTab, envTab, modTab, lfoTab, fxTab, magicTab, seqTab, presetTab, modularTab;
    juce::ComboBox themeBox;
    juce::ComboBox presetFilterBox;
    juce::ComboBox filterMode, lfoWave;
    juce::ComboBox filterRouteBox, voiceModeBox;
    juce::ComboBox distModeBox, reverbModeBox, delayModeBox;
    juce::ToggleButton arpOn { "ARP ON" }, seqOn { "SEQ ON" };
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, initBtn { "INIT" }, savePresetBtn { "SAVE" }, loadPresetBtn { "LOAD" }, bankBtn { "BANK" };
    juce::Label presetLabel, title, tagline;
    juce::ListBox presetList { "presets", this };
    std::unique_ptr<StepGridComponent> stepGrid;
    juce::OwnedArray<juce::Label> fxSectionLabels;
    juce::OwnedArray<juce::ImageComponent> fxSectionIcons;
    juce::OwnedArray<juce::ImageComponent> tabSectionIcons; // MAIN/LFO/MOD badges
    juce::OwnedArray<juce::TextButton> seqMagicBtns;
    juce::TextButton seqMagicOff;
    juce::OwnedArray<FxMonitor> fxMonitors;
    juce::ToggleButton fxBypass[8];
    juce::TextButton presetToggle { "<<" }; // collapses preset list only
    juce::TextButton bgSwapBtn { "BG" }; // swap background art
    juce::TextButton inspireBtn { "INSPIRE" };
    juce::TextButton charCycleBtn { "CHAR" }; // cycle character overlay PNGs
    juce::TextButton modSrcLfo1 { "LFO1" }, modSrcLfo2 { "LFO2" }, modSrcLfo3 { "LFO3" };
    int armedModSource = -1; // 0/1/2 = LFO1/2/3 for click-to-assign
    bool isModDragging = false; // true while dragging from LFO pill
    int layoutRecoveryTicks = 0;
    bool editorWasShowing = false;
    int restoreLayoutDelay = 1;
    juce::Slider masterGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAtt;
    juce::TextButton langToggle { "EN" };   // EN <-> FA
    int charPortraitIdx = 8;
    juce::Image charImgL, charImgR; // cached portraits
    juce::Rectangle<int> charSlotBounds; // left under preset list
    juce::Image cachedBg;           // pre-scaled hero (rebuild on resize/theme only)
    int cachedBgW = 0, cachedBgH = 0, cachedBgTheme = -1;
    bool uiLangFa = false;
    LfoShapeEditor lfoShapeEditor;
    juce::ComboBox lfo1WaveBox, lfo2WaveBox, lfo3WaveBox;
    juce::ComboBox lfoShapeBank; // 32 preset shapes
    juce::ComboBox lfoPointsBox; // 8 / 16 / 32 editable vertices
    juce::TextButton lfoPresetSine { "SIN" }, lfoPresetTri { "TRI" }, lfoPresetSaw { "SAW" },
                     lfoPresetSqr { "SQR" }, lfoPresetPulse { "PLS" }, lfoPresetCustom { "DRAW" },
                     lfoPresetExp { "EXP" }, lfoPresetLog { "LOG" }, lfoPresetBell { "BELL" },
                     lfoPresetWob { "WOB" }, lfoPresetChaos { "CHAOS" }, lfoPresetGate { "GATE" };
    // Copy custom shape across LFOs + 3 reusable slots
    juce::TextButton lfoCopyTo1 { "->L1" }, lfoCopyTo2 { "->L2" }, lfoCopyTo3 { "->L3" },
                     lfoSaveA { "SAVE A" }, lfoLoadA { "LOAD A" },
                     lfoSaveB { "SAVE B" }, lfoLoadB { "LOAD B" },
                     lfoSaveC { "SAVE C" }, lfoLoadC { "LOAD C" };
    juce::ComboBox osc1ShapeBox, osc2ShapeBox, osc3ShapeBox;
    int lfoShapeTarget = 0;
    std::unique_ptr<MagicPad> magicPad;
    juce::TextButton magicLoopBtn, magicGlitchBtn, magicFlangeBtn, magicPsychBtn;
    juce::TextButton magicHold { "HOLD" };
    juce::Label magicHint;
    juce::MidiKeyboardComponent keyboard;
    int computerKeyboardBase = 36; // computer-keyboard C2; convenient bass range
    juce::Array<int> computerKeyboardHeldNotes;
    juce::Array<int> computerKeyboardHeldKeys;
    float phaseLights = 0.0f;
    float animPhase = 0.0f;
    juce::Colour themeAccent { 0xff00e8ff }, themeAccent2 { 0xffffd700 }, themePanelBg { 0xff0a0614 };
    juce::Image logoImg, logoAltImg, heroImg, faceImg, lianImg, cyanImg;
    struct LogoOverlayComp : public juce::Component
    {
        juce::Image img;
        void paint (juce::Graphics& g) override
        {
            if (! img.isValid()) return;
            // Transparent — no panel behind logo (user request)
            auto r = getLocalBounds().toFloat();
            g.setOpacity (1.f);
            g.drawImage (img, r, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
    };
    LogoOverlayComp logoOverlay;
    void refreshCharCache();
    int heroIndex = 0;
    bool presetCollapsed = false;
    void applyHeroFromTheme();
    void cycleHero();
    void applyUiLanguage();
    struct PresetRow { bool isHeader = false; juce::String label; int programIndex = -1; juce::String category; };
    juce::StringArray collapsedCats; // category names that are folded
    juce::Array<PresetRow> presetRows;
    void rebuildPresetRows();
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void assignModToParam (const juce::String& paramId, float amount = 0.5f);
    void tryAssignModAt (juce::Point<int> editorPos, float amount, const juce::ModifierKeys& mods);
    juce::Point<int> currentMousePositionInEditor() const;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    std::vector<std::unique_ptr<juce::MouseListener>> modHookListeners;
    struct KnobDropTarget : public juce::Component, public juce::DragAndDropTarget
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        juce::String paramId;
        bool hot = false;
        bool isInterestedInDragSource (const SourceDetails& d) override
        {
            return d.description.toString().startsWith ("SALEK_LFO");
        }
        void itemDragEnter (const SourceDetails&) override { hot = true;  repaint(); }
        void itemDragExit  (const SourceDetails&) override { hot = false; repaint(); }
        void itemDropped (const SourceDetails& d) override
        {
            hot = false;
            auto s = d.description.toString(); // SALEK_LFO0/1/2
            int src = 0;
            if (s.endsWithChar ('1')) src = 1;
            else if (s.endsWithChar ('2')) src = 2;
            if (ed != nullptr)
            {
                ed->armedModSource = src;
                float amt = 0.5f;
                if (juce::ModifierKeys::currentModifiers.isShiftDown()) amt = 1.0f;
                if (juce::ModifierKeys::currentModifiers.isAltDown())   amt = -0.5f;
                ed->assignModToParam (paramId, amt);
            }
            repaint();
        }
        void paint (juce::Graphics& g) override
        {
            if (! hot) return;
            g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
            g.fillEllipse (getLocalBounds().toFloat().reduced (2.f));
            g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.95f));
            g.drawEllipse (getLocalBounds().toFloat().reduced (2.f), 2.5f);
        }
        bool shouldDrawDragImageWhenOver() override { return false; }
        // Critical: only intercept hits while a drag is active so slider stays usable
        bool hitTest (int, int) override
        {
            return ed != nullptr && ed->isDragAndDropActive();
        }
    };
    std::vector<std::unique_ptr<KnobDropTarget>> knobDropTargets;
    // bottom model strip
    juce::OwnedArray<juce::ImageComponent> modelStrip;
    juce::Label modelStripTitle;
    juce::Rectangle<int> modelStripBounds;
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};
