SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    keyboard.setWantsKeyboardFocus (true);
    // Keep computer-keyboard → MIDI working even while mouse is on knobs
    setWantsKeyboardFocus (true);
    glBackdrop = nullptr;
    sonicCore = nullptr;

    addAndMakeVisible (keyboard);
    // VBlank disabled — was causing 5-10 FPS lag with heavy BG paint
    // Animation driven by timer only at 12 Hz
    startTimerHz (3); // lighter UI refresh

    setLookAndFeel (&lnf);
    logoImg   = SalekAssets::loadLogoGiti();
    if (! logoImg.isValid()) logoImg = SalekAssets::loadLogo();
    logoAltImg = SalekAssets::loadLogoAlt();
    // Prefer transparent sheet logo when available
    {
        auto sheet = SalekAssets::loadSalekSheetLogo();
        if (sheet.isValid() && sheet.getWidth() > 100)
            logoImg = sheet;
    }
    logoOverlay.img = logoImg;
    logoOverlay.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (logoOverlay);

    inspireBtn.setButtonText ("INSPIRE");
    inspireBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0530));
    inspireBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffff2d9b));
    inspireBtn.setTooltip ("Randomize in current style (genre-aware)");
    addAndMakeVisible (inspireBtn);
    inspireBtn.onClick = [this]
    {
        // Style-aware randomize from current program category
        const int n = processor.getNumPrograms();
        if (n <= 1) return;
        juce::String cur = processor.getProgramName (processor.getCurrentProgram());
        juce::String cat = cur.upToFirstOccurrenceOf ("/", false, false);
        juce::Array<int> pool;
        for (int i = 0; i < n; ++i)
        {
            auto nm = processor.getProgramName (i);
            if (cat.isNotEmpty() && nm.startsWithIgnoreCase (cat + "/"))
                pool.add (i);
        }
        if (pool.isEmpty())
        {
            for (int i = 1; i < n; ++i) pool.add (i); // skip Init
        }
        if (pool.isEmpty()) return;
        const int pick = pool[juce::Random::getSystemRandom().nextInt (pool.size())];
        processor.setCurrentProgram (pick);
        rebuildPresetRows();
        repaint();
    };

    logoOverlay.toFront (false);

    // Bottom model strip title
    modelStripTitle.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    modelStripTitle.setJustificationType (juce::Justification::centred);
    modelStripTitle.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    modelStripTitle.setColour (juce::Label::textColourId, juce::Colour (0xff00e8ff));
    addAndMakeVisible (modelStripTitle);
    modelStrip.clear();
    for (int i = 0; i < 10; ++i)
    {
        auto* ic = new juce::ImageComponent ("model" + juce::String (i));
        auto im = SalekAssets::loadCharPortrait (i, 160, 240); // thumbnail size keeps multi-instance RAM low
        if (im.isValid())
        {
            ic->setImage (im);
            ic->setImagePlacement (juce::RectanglePlacement::xMid | juce::RectanglePlacement::yBottom | juce::RectanglePlacement::onlyReduceInSize);
        }
        ic->setInterceptsMouseClicks (true, false);
        addAndMakeVisible (ic);
        modelStrip.add (ic);
    }

    
    for (int i = 0; i < modelStrip.size(); ++i)
    {
        if (auto* ic = modelStrip[i])
        {
            ic->setMouseCursor (juce::MouseCursor::PointingHandCursor);
            // capture index for click
        }
    }
    // Click any strip model → set main character
    struct ModelPickHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        int idx = 0;
        void mouseDown (const juce::MouseEvent&) override
        {
            if (ed == nullptr) return;
            ed->charPortraitIdx = idx;
            ed->refreshCharCache();
            ed->repaint();
        }
    };
    for (int i = 0; i < modelStrip.size(); ++i)
    {
        auto* hook = new ModelPickHook();
        hook->ed = this;
        hook->idx = i;
        if (auto* ic = modelStrip[i])
            ic->addMouseListener (hook, false);
        modHookListeners.push_back (std::unique_ptr<juce::MouseListener> (hook));
    }

    charPortraitIdx = 5; // startup hero: the white/cyan cyber model from the carousel
    refreshCharCache();
    // face.png removed (was ~2MB) — BGs only
    lianImg   = SalekAssets::loadBgIsatis();
    cyanImg   = SalekAssets::loadBgSalek();
    heroImg   = lianImg;
    refreshCharCache(); // load char PNGs ONCE
    
    // Global LFO mod sources (Serum/Vital style) — visible all tabs
    auto styleModSrc = [this] (juce::TextButton& b, juce::Colour c, int src)
    {
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
        b.setColour (juce::TextButton::buttonOnColourId, c.withAlpha (0.55f));
        b.setColour (juce::TextButton::textColourOffId, c);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.setClickingTogglesState (true);
        b.setTooltip ("Vital-style: arm " + b.getButtonText() + " → click knob | Shift=100% | Alt=invert | Ctrl=25% | Right-click=clear");
        b.onClick = [this, src, &b]
        {
            const bool on = b.getToggleState();
            armedModSource = on ? src : -1;
            modSrcLfo1.setToggleState (src == 0 && on, juce::dontSendNotification);
            modSrcLfo2.setToggleState (src == 1 && on, juce::dontSendNotification);
            modSrcLfo3.setToggleState (src == 2 && on, juce::dontSendNotification);
            if (! on) armedModSource = -1;
        };
        addAndMakeVisible (b);
    };
    styleModSrc (modSrcLfo1, juce::Colour (0xff00e8ff), 0);
    styleModSrc (modSrcLfo2, juce::Colour (0xffff2d9b), 1);
    styleModSrc (modSrcLfo3, juce::Colour (0xff39ff14), 2);

    // Global: when LFO armed, click any knob to route (works even if DnD target misses)
    struct GlobalModClickHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (ed == nullptr || ed->armedModSource < 0) return;
            if (e.eventComponent == &ed->modSrcLfo1 || e.eventComponent == &ed->modSrcLfo2
                || e.eventComponent == &ed->modSrcLfo3)
                return;
            auto pos = e.getEventRelativeTo (ed).getPosition();
            float amt = 0.5f;
            if (e.mods.isShiftDown()) amt = 1.0f;
            if (e.mods.isAltDown()) amt = -0.5f;
            if (e.mods.isCommandDown() || e.mods.isCtrlDown()) amt = 0.25f;
            if (e.mods.isRightButtonDown()) amt = 0.f;
            ed->tryAssignModAt (pos, amt, e.mods);
        }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (ed == nullptr || e.getDistanceFromDragStart() < 3)
                return;
            if (e.eventComponent == &ed->modSrcLfo1) ed->armedModSource = 0;
            else if (e.eventComponent == &ed->modSrcLfo2) ed->armedModSource = 1;
            else if (e.eventComponent == &ed->modSrcLfo3) ed->armedModSource = 2;
            else return;
            // This capture-phase listener sees the gesture even when a DAW
            // keeps the mouse attached to the source button while dragging.
            ed->isModDragging = true;
            ed->setMouseCursor (juce::MouseCursor::CopyingCursor);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            if (ed == nullptr || ! ed->isModDragging) return;
            const auto pos = ed->currentMousePositionInEditor();
            float amt = 0.5f;
            if (e.mods.isShiftDown()) amt = 1.0f;
            if (e.mods.isAltDown()) amt = -0.5f;
            ed->tryAssignModAt (pos, amt, e.mods);
            ed->isModDragging = false;
            ed->setMouseCursor (juce::MouseCursor::NormalCursor);
        }
    };
    {
        auto* ghook = new GlobalModClickHook();
        ghook->ed = this;
        addMouseListener (ghook, true); // all children
        modHookListeners.push_back (std::unique_ptr<juce::MouseListener> (ghook));
    }

    // Real drag-and-drop from LFO pills onto knobs
    struct LfoDragHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        int src = 0;
        void mouseDown (const juce::MouseEvent&) override
        {
            if (ed == nullptr) return;
            ed->armedModSource = src;
            ed->isModDragging = false;
            // exclusive arm UI
            ed->modSrcLfo1.setToggleState (src == 0, juce::dontSendNotification);
            ed->modSrcLfo2.setToggleState (src == 1, juce::dontSendNotification);
            ed->modSrcLfo3.setToggleState (src == 2, juce::dontSendNotification);
        }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (ed == nullptr || e.getDistanceFromDragStart() < 3) return;
            ed->armedModSource = src;
            ed->isModDragging = true;
            ed->setMouseCursor (juce::MouseCursor::CopyingCursor);
            // Keep the source's mouse capture until release. The host's
            // drag manager can steal that capture in several DAWs, so route
            // the release against the screen-space pointer directly instead.
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            if (ed == nullptr) return;
            if (ed->isModDragging || ed->armedModSource >= 0)
            {
                const auto pos = ed->currentMousePositionInEditor();
                float amt = 0.5f;
                if (e.mods.isShiftDown()) amt = 1.0f;
                if (e.mods.isAltDown()) amt = -0.5f;
                if (e.mods.isRightButtonDown()) amt = 0.f;
                ed->tryAssignModAt (pos, amt, e.mods);
            }
            ed->isModDragging = false;
            ed->setMouseCursor (juce::MouseCursor::NormalCursor);
        }
    };
    for (int i = 0; i < 3; ++i)
    {
        auto* hook = new LfoDragHook();
        hook->ed = this;
        hook->src = i;
        juce::TextButton* btns[3] = { &modSrcLfo1, &modSrcLfo2, &modSrcLfo3 };
        btns[i]->addMouseListener (hook, false);
        // leak-free store
        modHookListeners.push_back (std::unique_ptr<juce::MouseListener> (hook));
    }

    masterGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterGainSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterGainSlider.setTooltip ("Master Gain");
    addAndMakeVisible (masterGainSlider);
    masterGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "master_gain", masterGainSlider);

    setSize (1280, 820);
    setResizable (true, true);
    setResizeLimits (1020, 700, 1700, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("ALIEN", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    scope.setFifo (&processor.getVisualFifo());
    spectrum.setFifo (&processor.getVisualFifo());
    addAndMakeVisible (scope);
    addAndMakeVisible (spectrum);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    matrixPanel = std::make_unique<ModMatrixPanel> (processor.getModMatrix());
    speedMonitor = std::make_unique<AudioSpeedMonitor> (processor);
    aboutInfo = std::make_unique<AboutInfoPanel>();
    speedTab.addAndMakeVisible (*speedMonitor);
    aboutTab.addAndMakeVisible (*aboutInfo);

    tabs.addTab ("MAIN", juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab ("MOD", juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab ("LFO", juce::Colours::transparentBlack, &lfoTab, false);
    tabs.addTab ("FX", juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab ("MAGIC", juce::Colours::transparentBlack, &magicTab, false);
    
    // SALEK icon pack badges for MAIN / LFO / MOD
    {
        // indices: OSC1,OSC2,OSC3,FILTER1,ENV, LFO,LFO_SYNC,LFO_RND, MATRIX,MACRO,FM,WAVETABLE
        const int idxs[] = { 0, 1, 2, 11, 15, 17, 18, 19, 20, 21, 5, 3 };
        for (int i = 0; i < 12; ++i)
        {
            auto* ic = tabSectionIcons.add (new juce::ImageComponent ("tabIcon" + juce::String (i)));
            auto img = SalekAssets::loadIcon (idxs[i]);
            if (img.isValid())
            {
                ic->setImage (img);
                ic->setImagePlacement (juce::RectanglePlacement::centred);
            }
            ic->setInterceptsMouseClicks (false, false);
            ic->setVisible (false);
            addAndMakeVisible (ic); // reparent to tabs in layout
        }
    }

    tabs.addTab ("SEQ", juce::Colours::transparentBlack, &seqTab, false);
    tabs.addTab ("SPEED", juce::Colours::transparentBlack, &speedTab, false);
    tabs.addTab ("ABOUT", juce::Colours::transparentBlack, &aboutTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    // Glass UI: transparent so animated theme shows through (no solid pink slab)
    tabs.setOpaque (false);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack);
    tabs.setColour (juce::TabbedComponent::outlineColourId, juce::Colour (0x55ffffff));
    for (auto* panel : { &mainTab, &modTab, &lfoTab, &magicTab, &seqTab, &speedTab, &aboutTab, &oscTab, &filterTab, &envTab, &presetTab })
    {
        panel->setOpaque (false);
        panel->setColour (juce::ResizableWindow::backgroundColourId, juce::Colours::transparentBlack);
    }
    // FX: solid dark underlay so knobs readable (user: black under FX)
    // Opaque without paint() shows garbage/pink — keep transparent; editor paints dark underlay
    fxTab.setOpaque (false);
    fxTab.setColour (juce::ResizableWindow::backgroundColourId, juce::Colours::transparentBlack);

    {
        mainTab.addAndMakeVisible (oscTab);
        mainTab.addAndMakeVisible (filterTab);
        mainTab.addAndMakeVisible (envTab);
        // presetTab is global left column — parent is editor (not mainTab)
        addAndMakeVisible (presetTab);
        if (filterDisplay != nullptr) filterTab.addAndMakeVisible (*filterDisplay);
        if (adsrDisplay != nullptr) envTab.addAndMakeVisible (*adsrDisplay);

        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);

        addKnob (oscTab, "osc1_level", "LVL", C); addKnob (oscTab, "osc1_table", "TABLE", M);
        addKnob (oscTab, "osc1_warp", "WARP", O); addKnob (oscTab, "osc1_fold", "FOLD", M);
        addKnob (oscTab, "osc1_drive", "DRIVE", O); addKnob (oscTab, "osc1_octave", "OCT", V); addKnob (oscTab, "osc1_semi", "SEMI", M); addKnob (oscTab, "osc1_coarse", "CRS PITCH", G);
        addKnob (oscTab, "osc2_level", "LVL", C); addKnob (oscTab, "osc2_table", "TABLE", M);
        addKnob (oscTab, "osc2_warp", "WARP", O); addKnob (oscTab, "osc2_fold", "FOLD", M);
        addKnob (oscTab, "osc2_drive", "DRIVE", O); addKnob (oscTab, "osc2_octave", "OCT", V); addKnob (oscTab, "osc2_semi", "SEMI", M); addKnob (oscTab, "osc2_coarse", "CRS PITCH", G);
        addKnob (oscTab, "osc3_level", "LVL", C); addKnob (oscTab, "osc3_table", "TABLE", M);
        addKnob (oscTab, "osc3_warp", "WARP", O); addKnob (oscTab, "osc3_fold", "FOLD", M);
        addKnob (oscTab, "osc3_drive", "DRIVE", O); addKnob (oscTab, "osc3_octave", "OCT", V); addKnob (oscTab, "osc3_semi", "SEMI", M); addKnob (oscTab, "osc3_coarse", "CRS PITCH", G);
        // Per-osc unison row (each column gets its own UNI / DET / SPR)
        addKnob (oscTab, "osc1_unison", "UNI", C);
        addKnob (oscTab, "osc1_udet", "DET", M);
        addKnob (oscTab, "osc1_uspread", "SPR", O);
        addKnob (oscTab, "osc2_unison", "UNI", C);
        addKnob (oscTab, "osc2_udet", "DET", M);
        addKnob (oscTab, "osc2_uspread", "SPR", O);
        addKnob (oscTab, "osc3_unison", "UNI", C);
        addKnob (oscTab, "osc3_udet", "DET", M);
        addKnob (oscTab, "osc3_uspread", "SPR", O);
        // Phase + random phase (prevents locked static note phase)
        addKnob (oscTab, "osc1_phase", "PHASE", C); addKnob (oscTab, "osc1_rand", "RAND", M); addKnob (oscTab, "osc1_pan", "PAN", O);
        addKnob (oscTab, "osc2_phase", "PHASE", C); addKnob (oscTab, "osc2_rand", "RAND", M); addKnob (oscTab, "osc2_pan", "PAN", O);
        addKnob (oscTab, "osc3_phase", "PHASE", C); addKnob (oscTab, "osc3_rand", "RAND", M); addKnob (oscTab, "osc3_pan", "PAN", O);

        #include "PluginEditorOscMonitors.inl"

        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "F DRIVE", O);
        addKnob (filterTab, "filter_env", "F ENV", G);
        // 16-mode filter bank
        addCombo (filterTab, filterMode, "filter_mode", {
            "LP12","LP24","HP12","HP24","BP","Notch","Peak","AllPass",
            "AcidLP","Ladder","Comb","Formant","BandRej","LoShelf","HiShelf","PhaserN",
            "DiodeLP","Sallen","Vocal","RingMod","BP-Q","LP+HP","Notch2","Wide"
        });
        filterMode.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        filterMode.setColour (juce::ComboBox::textColourId, juce::Colour (0xff00e8ff));
        addCombo (filterTab, filterRouteBox, "filter_route", {
            "All","OSC1","OSC2","OSC3","1+2","1+3","2+3"
        });
        filterRouteBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        filterRouteBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffff66cc));
        addCombo (envTab, voiceModeBox, "voice_mode", { "Poly", "Mono", "Legato" });
        voiceModeBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        voiceModeBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xff00e8ff));

        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);

        presetList.setModel (this);
        presetList.setRowHeight (24);
        presetList.setOpaque (false);
        presetTab.setOpaque (false);
        presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0x66080414));
        presetList.setColour (juce::ListBox::outlineColourId, juce::Colour (0xff00e8ff).withAlpha (0.35f));
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (savePresetBtn);
        presetTab.addAndMakeVisible (loadPresetBtn);
        presetTab.addAndMakeVisible (bankBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setFont (juce::FontOptions (11.0f));
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00e8ff));
        savePresetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a1050));
        savePresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
        loadPresetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0a3050));
        loadPresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
        bankBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff201040));
        bankBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffaa00));
        prevPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i > 0) processor.setCurrentProgram (i - 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetLabel.setTooltip (processor.getProgramDescription (processor.getCurrentProgram()));
            presetList.updateContent();
            presetList.repaint();
        };
        nextPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i + 1 < processor.getNumPrograms()) processor.setCurrentProgram (i + 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetLabel.setTooltip (processor.getProgramDescription (processor.getCurrentProgram()));
            presetList.updateContent();
            presetList.repaint();
        };
        initBtn.onClick = [this] {
            processor.setCurrentProgram (0);
            presetLabel.setText (processor.getProgramName (0), juce::dontSendNotification);
            presetLabel.setTooltip (processor.getProgramDescription (0));
            presetList.updateContent();
            presetList.repaint();
        };
        #include "PluginEditorPresetIO.inl"
        rebuildPresetRows();
        presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
        presetLabel.setTooltip (processor.getProgramDescription (processor.getCurrentProgram()));
    }

    {
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        #include "PluginEditorModKnobs.inl"
    }

    {
        #include "PluginEditorLfoSetup.inl"
    }

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        #include "PluginEditorFxSections.inl"
    }

    {
        #include "PluginEditorMagic.inl"

    // SEQ → Magic trigger row (2x7): fire magic modes from sequencer page
    {
        static const char* names[14] = {
            "VOID","STAB","WARP","SLICE","DRONE","ACID","GLASS",
            "CRASH","ECHO","PSY","LOOP","NOISE","ALIEN","TORO"
        };
        for (int i = 0; i < 14; ++i)
        {
            auto* b = seqMagicBtns.add (new juce::TextButton (names[i]));
            b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            seqTab.addAndMakeVisible (b);
            b->onClick = [this, i]
            {
                // Mirror Magic combo behaviour
                const int modes[14] = { 4,0,3,5,0,6,2,7,2,3,0,8,3,1 };
                const float xs[14]  = { 0.85f,0.3f,0.7f,0.95f,0.2f,0.6f,0.4f,0.9f,0.55f,0.75f,0.35f,0.8f,0.65f,0.88f };
                const float ys[14]  = { 0.9f,0.5f,0.8f,0.7f,0.4f,0.85f,0.6f,0.95f,0.55f,0.9f,0.45f,0.75f,0.85f,0.92f };
                if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
                { pa->beginChangeGesture(); pa->setValueNotifyingHost (1.f); pa->endChangeGesture(); }
                if (auto* pm = dynamic_cast<juce::AudioParameterChoice*> (processor.getAPVTS().getParameter ("magic_mode")))
                { pm->beginChangeGesture(); pm->setValueNotifyingHost (pm->convertTo0to1 ((float) modes[i])); pm->endChangeGesture(); }
                auto setF = [this] (const char* id, float v) {
                    if (auto* p = processor.getAPVTS().getParameter (id)) {
                        p->beginChangeGesture();
                        p->setValueNotifyingHost (p->convertTo0to1 (v));
                        p->endChangeGesture();
                    }
                };
                setF ("magic_x", xs[i]); setF ("magic_y", ys[i]);
                processor.getMagic().setMode (modes[i]);
                processor.getMagic().setXY (xs[i], ys[i]);
                processor.getMagic().setActive (true);
            };
        }
        // Magic OFF + latch row helpers
        seqMagicOff.setButtonText ("MAGIC OFF");
        seqMagicOff.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a1020));
        seqMagicOff.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffff2d9b));
        seqTab.addAndMakeVisible (seqMagicOff);
        seqMagicOff.onClick = [this] {
            if (auto* pt = processor.getAPVTS().getParameter ("seq_magic_target"))
            { pt->beginChangeGesture(); pt->setValueNotifyingHost (pt->convertTo0to1 (0.f)); pt->endChangeGesture(); }
            if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            { pa->beginChangeGesture(); pa->setValueNotifyingHost (0.f); pa->endChangeGesture(); }
            processor.getMagic().setActive (false);
        };
    }

    // Explicit X/Y knobs (also LFO-modulatable)
    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        addKnob (magicTab, "magic_x", "MAGIC X", C);
        addKnob (magicTab, "magic_y", "MAGIC Y", M);
    }

    }

    {
        seqTab.addAndMakeVisible (arpOn);
        seqTab.addAndMakeVisible (seqOn);
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "arp_on", arpOn));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "seq_on", seqOn));
        stepGrid = std::make_unique<StepGridComponent> (processor.getStepSequencer());
        seqTab.addAndMakeVisible (*stepGrid);
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto G = juce::Colour (0xff7c4dff);
        // Keep the sequencer strip focused: four controls, all clearly named
        // for the step sequencer. ARP retains its own stable engine defaults.
        addKnob (seqTab, "seq_rate", "SEQ RATE", O);
        addKnob (seqTab, "seq_length", "SEQ STEPS", C);
        addKnob (seqTab, "seq_swing", "SEQ SWING", M);
        addKnob (seqTab, "seq_gate", "SEQ GATE", O);
        addKnob (seqTab, "seq_magic_depth", "XY DEPTH", C);
        addCombo (seqTab, seqMagicTargetBox, "seq_magic_target", { "Off", "X", "Y", "X+Y" });
        // Extra voice colour — registered last so indices of FX/MOD stay stable
        addKnob (envTab, "glide", "GLIDE", C);
        addKnob (envTab, "poly_voices", "VOICES", M);
        addKnob (envTab, "noise_level", "NOISE", O);
        addKnob (envTab, "sub_level", "SUB", G);
    }

    themeBox.addItem ("CYBER", 1);
    themeBox.addItem ("ACID", 2);
    themeBox.addItem ("NEON", 3);
    themeBox.addItem ("PATINA", 4);
    themeBox.setSelectedId (1); // default CYBER
    themeBox.onChange = [this] { applyHeroFromTheme(); repaint(); };
    addAndMakeVisible (themeBox);
    bgSwapBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    bgSwapBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
    bgSwapBtn.setTooltip ("Swap background (ISATIS man+cat / SALEK 2 girls)");
    bgSwapBtn.onClick = [this]
    {
        // toggle between CYBER (1) and NEON (3) → different BGs
        themeBox.setSelectedId (themeBox.getSelectedId() == 1 ? 3 : 1, juce::sendNotification);
    };
    addAndMakeVisible (bgSwapBtn);

    charCycleBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    charCycleBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffff2d9b));
    charCycleBtn.setButtonText ("MODEL");
    charCycleBtn.setTooltip ("Change character model (left panel) — does NOT change background");
    charCycleBtn.onClick = [this]
    {
        charPortraitIdx = (charPortraitIdx + 1) % 10; // 3 models only
        refreshCharCache();
        repaint();
    };
    addAndMakeVisible (charCycleBtn);

    // Collapse PRESET list only (hero stays visible)
    presetToggle.setButtonText (juce::CharPointer_UTF8 ("â")); // ◀
    presetToggle.setClickingTogglesState (true);
    presetToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    presetToggle.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff3a1060));
    presetToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
    presetToggle.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffd700));
    presetToggle.setTooltip ("Collapse / expand presets");
    presetToggle.onClick = [this]
    {
        presetCollapsed = presetToggle.getToggleState();
        presetToggle.setButtonText (presetCollapsed
            ? juce::CharPointer_UTF8 ("â¶")   // ▶
            : juce::CharPointer_UTF8 ("â")); // ◀
        resized();
        repaint();
    };
    presetTab.addAndMakeVisible (presetToggle); // must be child of presetTab for correct coords

    // Bilingual UI: EN (default) <-> FA
    langToggle.setButtonText ("EN");
    langToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
    langToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
    langToggle.setTooltip ("UI language EN / FA");
    langToggle.onClick = [this]
    {
        uiLangFa = ! uiLangFa;
        langToggle.setButtonText (uiLangFa ? "FA" : "EN");
        applyUiLanguage();
    };
    addAndMakeVisible (langToggle);

    applyHeroFromTheme();
    applyUiLanguage();
    rebuildPresetRows();
    // Start with categories folded — click [+] to expand (Serum/Vital style browser)
    {
        juce::StringArray cats;
        const int n = processor.getNumPrograms();
        for (int i = 0; i < n; ++i)
        {
            auto name = processor.getProgramName (i);
            auto cat = name.upToFirstOccurrenceOf ("/", false, false);
            if (cat == name) cat = "OTHER";
            if (! cats.contains (cat)) cats.add (cat);
        }
        collapsedCats = cats;
        rebuildPresetRows();
    }
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    glBackdrop = nullptr;
    setLookAndFeel (nullptr);
}

SalekHightechAudioProcessorEditor::Knob& SalekHightechAudioProcessorEditor::addKnob (
    juce::Component& parent, const char* id, const char* label, juce::Colour c)
{
    auto k = std::make_unique<Knob>();
    k->paramId = id;
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k->s.setNumDecimalPlacesToDisplay (1);   // short numbers (fixes "هزار رقمی")
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    k->s.setScrollWheelEnabled (true);
    k->s.setMouseDragSensitivity (180);
    // Cleaner value text: max 2 decimals, integers without .00
    k->s.textFromValueFunction = [] (double v) {
        if (std::abs (v) >= 1000.0) return juce::String ((int) std::round (v));
        if (std::abs (v - std::round (v)) < 1e-4) return juce::String ((int) std::round (v));
        if (std::abs (v) >= 100.0) return juce::String (v, 0);
        if (std::abs (v) >= 10.0) return juce::String (v, 1);
        return juce::String (v, 2);
    };
    parent.addAndMakeVisible (k->s);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), id, k->s));
    k->name.setText (label, juce::dontSendNotification);
    k->name.setJustificationType (juce::Justification::centred);
    k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    k->name.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    parent.addAndMakeVisible (k->name);

    // Vital-style: arm LFO1/2/3 then click knob name to assign
    struct ModHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        juce::String pid;
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (ed == nullptr) return;
            if (ed->armedModSource < 0) return;
            float amt = 0.5f;
            if (e.mods.isShiftDown()) amt = 1.0f;
            if (e.mods.isAltDown())   amt = -0.5f;
            if (e.mods.isCommandDown() || e.mods.isCtrlDown()) amt = 0.25f;
            if (e.mods.isRightButtonDown()) amt = 0.f;
            ed->assignModToParam (pid, amt);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            // Drop: if an LFO was armed via drag, assign on release over this knob
            if (ed == nullptr || ed->armedModSource < 0) return;
            // Only if this release is not a simple click already handled
            if (e.mouseWasDraggedSinceMouseDown())
                ed->assignModToParam (pid, 0.5f);
        }
    };
    auto hook = std::make_unique<ModHook>();
    hook->ed = this;
    hook->pid = id;
    k->name.addMouseListener (hook.get(), false);
    k->s.addMouseListener (hook.get(), false);
    modHookListeners.push_back (std::move (hook));

    // Real DragAndDropTarget over the knob (Serum/Vital style drop zone)
    {
        auto dt = std::make_unique<KnobDropTarget>();
        dt->ed = this;
        dt->paramId = id;
        dt->setInterceptsMouseClicks (true, false);
        parent.addAndMakeVisible (*dt);
        dt->toFront (false); // above slider; hitTest false unless dragging
        knobDropTargets.push_back (std::move (dt));
    }

    knobs.push_back (std::move (k));
    return *knobs.back();
}

void SalekHightechAudioProcessorEditor::assignModToParam (const juce::String& paramId, float amount)
{
    if (armedModSource < 0 || armedModSource > 2) return;

    using D = salek::ModMatrix::Dest;
    using S = salek::ModMatrix::Source;
    D dest = D::NumDests;
    if      (paramId == "filter_cutoff") dest = D::FilterCutoff;
    else if (paramId == "filter_reso")   dest = D::FilterReso;
    else if (paramId == "osc1_level")    dest = D::Osc1Level;
    else if (paramId == "osc2_level")    dest = D::Osc2Level;
    else if (paramId == "osc3_level")    dest = D::Osc3Level;
    else if (paramId == "osc1_table")    dest = D::Osc1Table;
    else if (paramId == "osc2_table")    dest = D::Osc2Table;
    else if (paramId == "osc3_table")    dest = D::Osc3Table;
    else if (paramId == "osc1_warp")     dest = D::Osc1Warp;
    else if (paramId == "osc2_warp")     dest = D::Osc2Warp;
    else if (paramId == "osc3_warp")     dest = D::Osc3Warp;
    else if (paramId == "osc1_fold")     dest = D::Osc1Fold;
    else if (paramId == "osc2_fold")     dest = D::Osc2Fold;
    else if (paramId == "osc3_fold")     dest = D::Osc3Fold;
    else if (paramId == "osc1_pan")      dest = D::Osc1Pan;
    else if (paramId == "osc2_pan")      dest = D::Osc2Pan;
    else if (paramId == "osc3_pan")      dest = D::Osc3Pan;
    else if (paramId == "osc1_drive")    dest = D::Osc1Drive;
    else if (paramId == "osc2_drive")    dest = D::Osc2Drive;
    else if (paramId == "osc3_drive")    dest = D::Osc3Drive;
    else if (paramId == "fm_2to1")       dest = D::Fm2to1;
    else if (paramId == "fm_3to1")       dest = D::Fm3to1;
    else if (paramId == "delay_mix")     dest = D::DelayMix;
    else if (paramId == "reverb_mix")    dest = D::ReverbMix;
    else if (paramId == "dist_drive")    dest = D::DistDrive;
    else if (paramId == "chorus_mix")    dest = D::ChorusMix;
    else if (paramId == "phaser_mix")    dest = D::PhaserMix;
    else if (paramId == "filter_env")    dest = D::FilterEnv;
    else if (paramId == "bassify")       dest = D::Bassify;
    else if (paramId == "magic_x")       dest = D::MagicX;
    else if (paramId == "magic_y")       dest = D::MagicY;
    else if (paramId == "osc1_unison")   dest = D::Osc1Level;
    else if (paramId == "osc1_udet")     dest = D::Osc1Warp;
    else if (paramId == "osc1_uspread")  dest = D::Osc1Pan;
    else if (paramId == "osc2_unison")   dest = D::Osc2Level;
    else if (paramId == "osc2_udet")     dest = D::Osc2Warp;
    else if (paramId == "osc3_unison")   dest = D::Osc3Level;
    else if (paramId == "master_drive")  dest = D::DistDrive;
    else return;

    S src = (armedModSource == 0) ? S::LFO1 : (armedModSource == 1) ? S::LFO2 : S::LFO3;
    if (std::abs (amount) < 1e-4f)
        processor.getModMatrix().removeRoute (src, dest);
    else
        processor.getModMatrix().addRoute (src, dest, juce::jlimit (-1.f, 1.f, amount));

    // Keep LFO armed for multiple routes (click OFF on LFO to disarm)
    // Brief visual: flash matrix
    if (matrixPanel != nullptr)
        matrixPanel->repaint();
    repaint();
}

void SalekHightechAudioProcessorEditor::addCombo (juce::Component& parent, juce::ComboBox& box,
                                                   const char* id, juce::StringArray items)
{
    box.addItemList (items, 1);
    parent.addAndMakeVisible (box);
    if (processor.getAPVTS().getParameter (id) != nullptr)
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), id, box));
}

void SalekHightechAudioProcessorEditor::rebuildPresetRows()
{
    // Group by unique category (no duplicate headers)
    presetRows.clear();
    juce::StringArray cats;
    const int n = processor.getNumPrograms();
    for (int i = 0; i < n; ++i)
    {
        auto name = processor.getProgramName (i);
        auto cat = name.upToFirstOccurrenceOf ("/", false, false);
        if (cat == name || cat.isEmpty()) cat = "OTHER";
        if (! cats.contains (cat, false))
            cats.add (cat);
    }
    // Prefer order matching mockup
    const juce::StringArray preferred { "OTHER","KICK","BASS","LEAD","SYNTH","PLUCK","PAD","ATMOSPHERE","HIGHTECH","FX","ACID","FM","RETRO","SQUEEK","KEYS","ARP","SALEK","AMBIENT","USER" };
    juce::StringArray ordered;
    for (auto& p : preferred)
        if (cats.contains (p, false)) ordered.add (p);
    for (auto& c : cats)
        if (! ordered.contains (c, false)) ordered.add (c);

    for (auto& cat : ordered)
    {
        PresetRow h; h.isHeader = true; h.label = cat.toUpperCase(); h.programIndex = -1; h.category = cat;
        presetRows.add (h);
        if (collapsedCats.contains (cat))
            continue;
        for (int i = 0; i < n; ++i)
        {
            auto name = processor.getProgramName (i);
            auto c = name.upToFirstOccurrenceOf ("/", false, false);
            if (c == name || c.isEmpty()) c = "OTHER";
            if (c != cat) continue;
            PresetRow r; r.isHeader = false;
            r.label = name.fromFirstOccurrenceOf ("/", false, false);
            if (r.label.isEmpty()) r.label = name;
            r.programIndex = i;
            r.category = cat;
            presetRows.add (r);
        }
    }
    presetList.updateContent();
    presetList.repaint();
}


void SalekHightechAudioProcessorEditor::tryAssignModAt (juce::Point<int> editorPos, float amount, const juce::ModifierKeys&)
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
}

juce::Point<int> SalekHightechAudioProcessorEditor::currentMousePositionInEditor() const
{
    const auto screen = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().roundToInt();
    return getLocalPoint (nullptr, screen);
}

void SalekHightechAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    juce::AudioProcessorEditor::mouseDrag (e);
}
void SalekHightechAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    // Fallback: if LFO armed and release over a knob, assign (even without DnD target hit)
    if (armedModSource >= 0 && e.mouseWasDraggedSinceMouseDown())
    {
        auto pos = currentMousePositionInEditor();
        float amt = 0.5f;
        if (e.mods.isShiftDown()) amt = 1.0f;
        if (e.mods.isAltDown()) amt = -0.5f;
        tryAssignModAt (pos, amt, e.mods);
    }
    juce::AudioProcessorEditor::mouseUp (e);
}
