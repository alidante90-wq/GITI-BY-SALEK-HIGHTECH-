
        if (lfoDisplay != nullptr) lfoTab.addAndMakeVisible (*lfoDisplay);
        lfoTab.addAndMakeVisible (lfoShapeEditor);
        lfoShapeEditor.setLfo (&processor.getLfo1());

        // Points resolution for freehand draw (8/16/32)
        lfoPointsBox.clear();
        lfoPointsBox.addItem ("PTS 8", 1);
        lfoPointsBox.addItem ("PTS 16", 2);
        lfoPointsBox.addItem ("PTS 32", 3);
        lfoPointsBox.setSelectedId (2, juce::dontSendNotification); // default 16
        lfoPointsBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff12081c));
        lfoPointsBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffffd700));
        lfoPointsBox.onChange = [this]
        {
            const int id = lfoPointsBox.getSelectedId();
            lfoShapeEditor.setActivePoints (id == 1 ? 8 : (id == 3 ? 32 : 16));
        };
        lfoTab.addAndMakeVisible (lfoPointsBox);
        lfoShapeEditor.setActivePoints (16);

        // Serum-inspired shape bank buttons
        struct ShapeBtn { juce::TextButton* b; const char* label; int preset; };
        ShapeBtn shapes[] = {
            { &lfoPresetSine,  "Sine",       0 },
            { &lfoPresetTri,   "Triangle",   1 },
            { &lfoPresetSaw,   "Saw Up",     2 },
            { &lfoPresetSqr,   "Square",     4 },
            { &lfoPresetPulse, "Pulse",      5 },
            { &lfoPresetExp,   "Exp Rise",   7 },
            { &lfoPresetLog,   "Log Fall",   9 },
            { &lfoPresetBell,  "Bell",      10 },
            { &lfoPresetWob,   "Wobble",    11 },
            { &lfoPresetChaos, "Chaos",     12 },
            { &lfoPresetGate,  "Gate",      13 },
            { &lfoPresetCustom,"Draw",      -1 }
        };
        for (auto& sh : shapes)
        {
            sh.b->setButtonText (sh.label);
            sh.b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a30));
            sh.b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            lfoTab.addAndMakeVisible (*sh.b);
        }

        auto loadShape = [this] (int preset)
        {
            salek::LFO* targets[3] = { &processor.getLfo1(), &processor.getLfo2(), &processor.getLfo3() };
            auto* L = targets[juce::jlimit (0, 2, lfoShapeTarget)];
            L->loadPresetShape (preset);
            L->setWave (salek::LFO::Wave::Custom);
            lfoShapeEditor.setLfo (L);
            lfoShapeEditor.syncFromLfo();
            const char* ids[] = { "lfo_wave", "lfo2_wave", "lfo3_wave" };
            if (auto* p = processor.getAPVTS().getParameter (ids[juce::jlimit(0,2,lfoShapeTarget)]))
                p->setValueNotifyingHost (p->convertTo0to1 (5.f)); // Custom
        };
        lfoPresetSine.onClick  = [loadShape] { loadShape (0); };
        lfoPresetTri.onClick   = [loadShape] { loadShape (1); };
        lfoPresetSaw.onClick   = [loadShape] { loadShape (2); };
        lfoPresetSqr.onClick   = [loadShape] { loadShape (4); };
        lfoPresetPulse.onClick = [loadShape] { loadShape (5); };
        lfoPresetExp.onClick   = [loadShape] { loadShape (7); };
        lfoPresetLog.onClick   = [loadShape] { loadShape (9); };
        lfoPresetBell.onClick  = [loadShape] { loadShape (10); };
        lfoPresetWob.onClick   = [loadShape] { loadShape (11); };
        lfoPresetChaos.onClick = [loadShape] { loadShape (12); };
        lfoPresetGate.onClick  = [loadShape] { loadShape (13); };
        lfoPresetCustom.onClick = [this] {
            if (auto* p = processor.getAPVTS().getParameter ("lfo_wave"))
                p->setValueNotifyingHost (p->convertTo0to1 (5.f));
            lfoShapeEditor.setLfo (&processor.getLfo1());
            lfoShapeEditor.syncFromLfo();
        };

        // Persian + English wave names
        auto waveItems = juce::StringArray {
            "Sine", "Triangle", "Saw", "Square", "S&H", "Custom"
        };
        lfo1WaveBox.addItemList (waveItems, 1);
        lfo2WaveBox.addItemList (waveItems, 1);
        lfo3WaveBox.addItemList (waveItems, 1);
        lfoTab.addAndMakeVisible (lfo1WaveBox);
        lfoTab.addAndMakeVisible (lfo2WaveBox);
        lfoTab.addAndMakeVisible (lfo3WaveBox);
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo_wave", lfo1WaveBox));
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo2_wave", lfo2WaveBox));
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo3_wave", lfo3WaveBox));

        lfo1WaveBox.onChange = [this] {
            if (lfo1WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 0; lfoShapeEditor.setLfo (&processor.getLfo1()); lfoShapeEditor.syncFromLfo(); }
        };
        lfo2WaveBox.onChange = [this] {
            if (lfo2WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 1; lfoShapeEditor.setLfo (&processor.getLfo2()); lfoShapeEditor.syncFromLfo(); }
        };
        lfo3WaveBox.onChange = [this] {
            if (lfo3WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 2; lfoShapeEditor.setLfo (&processor.getLfo3()); lfoShapeEditor.syncFromLfo(); }
        };

        // Apply custom shape to any LFO (reuse on other destinations via matrix)
        auto styleCopy = [] (juce::TextButton& b, juce::Colour c)
        {
            b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
            b.setColour (juce::TextButton::textColourOffId, c);
            b.setColour (juce::TextButton::buttonOnColourId, c.darker (0.2f));
        };
        styleCopy (lfoCopyTo1, juce::Colour (0xff00e8ff));
        styleCopy (lfoCopyTo2, juce::Colour (0xffff2d9b));
        styleCopy (lfoCopyTo3, juce::Colour (0xff39ff14));
        styleCopy (lfoSaveA, juce::Colour (0xffffd700));
        styleCopy (lfoLoadA, juce::Colour (0xffffd700));
        styleCopy (lfoSaveB, juce::Colour (0xff7c4dff));
        styleCopy (lfoLoadB, juce::Colour (0xff7c4dff));
        styleCopy (lfoSaveC, juce::Colour (0xffc0ff00));
        styleCopy (lfoLoadC, juce::Colour (0xffc0ff00));

        auto forceCustom = [this] (int which)
        {
            const char* ids[] = { "lfo_wave", "lfo2_wave", "lfo3_wave" };
            if (auto* p = processor.getAPVTS().getParameter (ids[which]))
                p->setValueNotifyingHost (p->convertTo0to1 (5.f));
            salek::LFO* targets[3] = { &processor.getLfo1(), &processor.getLfo2(), &processor.getLfo3() };
            targets[which]->setWave (salek::LFO::Wave::Custom);
        };

        lfoCopyTo1.onClick = [this, forceCustom] {
            lfoShapeEditor.applyTo (processor.getLfo1());
            forceCustom (0);
            lfoShapeTarget = 0;
            lfoShapeEditor.setLfo (&processor.getLfo1());
        };
        lfoCopyTo2.onClick = [this, forceCustom] {
            lfoShapeEditor.applyTo (processor.getLfo2());
            forceCustom (1);
            lfoShapeTarget = 1;
            lfoShapeEditor.setLfo (&processor.getLfo2());
        };
        lfoCopyTo3.onClick = [this, forceCustom] {
            lfoShapeEditor.applyTo (processor.getLfo3());
            forceCustom (2);
            lfoShapeTarget = 2;
            lfoShapeEditor.setLfo (&processor.getLfo3());
        };
        lfoSaveA.onClick = [this] { lfoShapeEditor.saveSlot (0); };
        lfoLoadA.onClick = [this] { lfoShapeEditor.loadSlot (0); };
        lfoSaveB.onClick = [this] { lfoShapeEditor.saveSlot (1); };
        lfoLoadB.onClick = [this] { lfoShapeEditor.loadSlot (1); };
        lfoSaveC.onClick = [this] { lfoShapeEditor.saveSlot (2); };
        lfoLoadC.onClick = [this] { lfoShapeEditor.loadSlot (2); };

        lfoTab.addAndMakeVisible (lfoCopyTo1);
        lfoTab.addAndMakeVisible (lfoCopyTo2);
        lfoTab.addAndMakeVisible (lfoCopyTo3);
        lfoTab.addAndMakeVisible (lfoSaveA);
        lfoTab.addAndMakeVisible (lfoLoadA);
        lfoTab.addAndMakeVisible (lfoSaveB);
        lfoTab.addAndMakeVisible (lfoLoadB);
        lfoTab.addAndMakeVisible (lfoSaveC);
        lfoTab.addAndMakeVisible (lfoLoadC);

        {
            const auto C = juce::Colour (0xff00e8ff);
            const auto M = juce::Colour (0xffff2d9b);
            const auto O = juce::Colour (0xff39ff14);
            const auto V = juce::Colour (0xffc0ff00);
            const auto G = juce::Colour (0xff7c4dff);
            addKnob (lfoTab, "lfo_rate", "LFO1 Hz", C);
            addKnob (lfoTab, "lfo_amount", "LFO1 AMT", M);
            addKnob (lfoTab, "lfo2_rate", "LFO2 Hz", O);
            addKnob (lfoTab, "lfo2_amount", "LFO2 AMT", V);
            addKnob (lfoTab, "lfo3_rate", "LFO3 Hz", G);
            addKnob (lfoTab, "lfo3_amount", "LFO3 AMT", C);
        }

        // Full 32-shape bank (Serum/Vital style)
        lfoShapeBank.clear (juce::dontSendNotification);
        for (int i = 0; i < salek::LFO::NumShapes; ++i)
            lfoShapeBank.addItem (salek::LFO::shapeName (i), i + 1);
        lfoShapeBank.setSelectedId (1, juce::dontSendNotification);
        lfoShapeBank.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        lfoShapeBank.setColour (juce::ComboBox::textColourId, juce::Colour (0xffffd700));
        lfoTab.addAndMakeVisible (lfoShapeBank);
        lfoShapeBank.onChange = [this]
        {
            const int preset = lfoShapeBank.getSelectedId() - 1;
            if (preset < 0) return;
            salek::LFO* targets[3] = { &processor.getLfo1(), &processor.getLfo2(), &processor.getLfo3() };
            auto* L = targets[juce::jlimit (0, 2, lfoShapeTarget)];
            L->loadPresetShape (preset);
            L->setWave (salek::LFO::Wave::Custom);
            lfoShapeEditor.setLfo (L);
            lfoShapeEditor.syncFromLfo();
            const char* ids[] = { "lfo_wave", "lfo2_wave", "lfo3_wave" };
            if (auto* p = processor.getAPVTS().getParameter (ids[juce::jlimit (0, 2, lfoShapeTarget)]))
                p->setValueNotifyingHost (p->convertTo0to1 (5.f));
        };
