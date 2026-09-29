void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();
    // DAWs can briefly report a minimized/zero client area while hiding and
    // restoring an editor. Keep the last valid layout until real bounds return.
    if (full.getWidth() < 900 || full.getHeight() < 600)
        return;

    auto place = [this] (juce::Rectangle<int> area, auto& arr, int start, int count, int cols, int maxCell = 0)
    {
        if (count <= 0 || cols <= 0) return;
        const int rows = (count + cols - 1) / cols;
        const int cw = juce::jmax (1, area.getWidth() / cols);
        const int rh = juce::jmax (1, area.getHeight() / rows);
        for (int i = 0; i < count; ++i)
        {
            const int idx = start + i;
            if (idx < 0 || idx >= (int) arr.size()) break;
            auto* k = arr[(size_t) idx].get();
            const int c = i % cols, r = i / cols;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, area.getY() + r * rh, cw, rh).reduced (2);
            if (maxCell > 0 && cell.getWidth() > maxCell)
            {
                const int pad = (cell.getWidth() - maxCell) / 2;
                cell = cell.withTrimmedLeft (pad).withTrimmedRight (pad);
            }
            k->name.setBounds (cell.removeFromBottom (14));
            k->name.setJustificationType (juce::Justification::centred);
            k->name.setVisible (true);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, cell.getWidth() - 8), 12);
            k->s.setNumDecimalPlacesToDisplay (2);
            k->s.textFromValueFunction = [] (double v)
                {
                    if (std::abs (v) >= 1000.0)
                        return juce::String ((int) std::round (v));
                    if (std::abs (v - std::round (v)) < 1e-3)
                        return juce::String ((int) std::round (v));
                    if (std::abs (v) >= 100.0)
                        return juce::String (v, 0);
                    if (std::abs (v) >= 10.0)
                        return juce::String (v, 1);
                    return juce::String (v, 2);
                };
            k->s.setBounds (cell);
            k->s.setVisible (true);
        }
    };

    {
        // ONLY place for model strip: above keyboard (never over knobs)
        auto bottom = full.removeFromBottom (64);
        auto strip = juce::Rectangle<int>();
        // Character carousel removed from the keyboard area: the selected hero is now shown only in the large left panel.
        modelStripBounds = {};
        modelStripTitle.setBounds (0, 0, 0, 0);
        modelStripTitle.setVisible (false);
        for (auto* ic : modelStrip) if (ic) { ic->setBounds (0, 0, 0, 0); ic->setVisible (false); }
        const int n = 0;
        if (n > 0)
        {
            const int cell = juce::jmax (12, strip.getWidth() / n);
            for (int i = 0; i < n; ++i)
            {
                if (auto* ic = modelStrip[i])
                {
                    ic->setBounds (strip.getX() + i * cell + 3, strip.getY() + 2, cell - 6, strip.getHeight() - 4);
                    ic->setVisible (true);
                    ic->toFront (false);
                }
            }
        }
        auto kb = bottom.reduced (2, 1);
        keyboard.setKeyWidth ((float) juce::jmax (10, kb.getWidth() / 52));
        keyboard.setBounds (kb);
        keyboard.toFront (false);
    }
    full.removeFromBottom (2);

    {
        const int presetW = presetCollapsed ? 28 : 260; // room for preset names
        auto leftStrip = full.removeFromLeft (presetW);
        const int logoPad = 8; // presets start near top of left strip
        // Preset ~32%, big character model gets remaining left column
        const int presetH = juce::jmax (170, (int) ((leftStrip.getHeight() - logoPad) * 0.42f)); // compact browser, larger hero model
        presetTab.setBounds (leftStrip.getX(), leftStrip.getY() + logoPad, leftStrip.getWidth(), presetH);
        presetTab.toFront (false);
        // Character PNG slot under presets (always visible when not collapsed)
        {
            if (presetCollapsed)
            {
                charSlotBounds = juce::Rectangle<int> (
                    leftStrip.getX() + 2, leftStrip.getY() + logoPad,
                    leftStrip.getWidth() - 4, juce::jmax (100, leftStrip.getHeight() - logoPad - 8));
            }
            else
            {
                const int cy = leftStrip.getY() + logoPad + presetH + 4;
                const int ch = juce::jmax (120, leftStrip.getBottom() - cy - 4);
                charSlotBounds = juce::Rectangle<int> (leftStrip.getX() + 2, cy, leftStrip.getWidth() - 4, ch);
            }
        }
        auto pb = presetTab.getLocalBounds().reduced (2);
        // Two-line toolbar — bigger hit targets
        auto row1 = pb.removeFromTop (28);
        presetToggle.setBounds (row1.removeFromLeft (32).reduced (1, 1));
        if (! presetCollapsed)
        {
            prevPreset.setBounds (row1.removeFromLeft (32).reduced (1, 1));
            nextPreset.setBounds (row1.removeFromLeft (32).reduced (1, 1));
            initBtn.setBounds (row1.removeFromLeft (row1.getWidth()).reduced (1, 1));
            auto row2 = pb.removeFromTop (28);
            savePresetBtn.setBounds (row2.removeFromLeft (row2.getWidth() / 3).reduced (1, 1));
            loadPresetBtn.setBounds (row2.removeFromLeft (row2.getWidth() / 2).reduced (1, 1));
            bankBtn.setBounds (row2.reduced (1, 1));
            presetLabel.setBounds (pb.removeFromTop (16).reduced (2, 0));
            presetList.setBounds (pb);
            presetList.setOpaque (false);
            presetTab.setOpaque (false);
            presetList.setVisible (true);
            prevPreset.setVisible (true); nextPreset.setVisible (true);
            initBtn.setVisible (true); savePresetBtn.setVisible (true);
            loadPresetBtn.setVisible (true); bankBtn.setVisible (true);
            presetLabel.setVisible (true);
        }
        else
        {
            presetList.setVisible (false); prevPreset.setVisible (false);
            nextPreset.setVisible (false); initBtn.setVisible (false);
            savePresetBtn.setVisible (false); loadPresetBtn.setVisible (false);
            bankBtn.setVisible (false); presetLabel.setVisible (false);
        }
    }

    auto header = full.removeFromTop (96); // logo prominence (I WANT)
    langToggle.setBounds (header.removeFromRight (36).reduced (2));
    themeBox.setBounds (header.removeFromRight (90).reduced (2));
    charCycleBtn.setBounds (header.removeFromRight (48).reduced (2));
    bgSwapBtn.setBounds (header.removeFromRight (40).reduced (2));
    masterGainSlider.setBounds (header.removeFromRight (40).reduced (2));
    // Compact LFO sources (horizontal strip, always visible)
    // LFO sources — drag sources (Vital-style)
    modSrcLfo3.setBounds (header.removeFromRight (56).reduced (2, 8));
    modSrcLfo2.setBounds (header.removeFromRight (56).reduced (2, 8));
    modSrcLfo1.setBounds (header.removeFromRight (56).reduced (2, 8));
    modSrcLfo1.setButtonText ("LFO1");
    modSrcLfo2.setButtonText ("LFO2");
    modSrcLfo3.setButtonText ("LFO3");
    inspireBtn.setBounds (header.removeFromRight (56).reduced (2, 10));
    // Big logo dead-center of header strip (above tabs / osc monitors)
    {
        const int lw = juce::jmin (900, juce::jmax (500, header.getWidth() * 7 / 10));
        const int lh = header.getHeight() + 10;
        logoOverlay.setBounds (header.getCentreX() - lw / 2, header.getY() - 5, lw, lh);
        logoOverlay.toFront (false);
        logoOverlay.setVisible (true);
    }

    // Keep drop targets aligned with knob sliders
    for (size_t i = 0; i < knobDropTargets.size() && i < knobs.size(); ++i)
    {
        if (knobs[i] != nullptr && knobDropTargets[i] != nullptr)
        {
            auto r = knobs[i]->s.getBounds();
            // parent of drop may differ — convert
            if (auto* p = knobDropTargets[i]->getParentComponent())
            {
                if (auto* sp = knobs[i]->s.getParentComponent())
                {
                    auto topLeft = p->getLocalPoint (sp, r.getTopLeft());
                    knobDropTargets[i]->setBounds (topLeft.x, topLeft.y, r.getWidth(), r.getHeight());
                }
            }
            knobDropTargets[i]->setVisible (knobs[i]->s.isVisible());
        }
    }

    // Remove top scopes / spectrum / wt strip (user X)
    spectrum.setBounds (0, 0, 0, 0); spectrum.setVisible (false);
    scope.setBounds (0, 0, 0, 0); scope.setVisible (false);
    if (wtDisplay != nullptr) { wtDisplay->setBounds (0, 0, 0, 0); wtDisplay->setVisible (false); }

    tabs.setBounds (full);
    tabs.toFront (false);

    // MAIN — place ALL knobs by PARAMETER ID (never by fragile array index)
    {
        auto byId = [this] (const char* id) -> Knob*
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id)
                    return k.get();
            return nullptr;
        };
        auto placeId = [&] (juce::Rectangle<int> box, const char* id)
        {
            if (auto* k = byId (id))
            {
                k->name.setBounds (box.removeFromBottom (14));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, box.getWidth() - 8), 12);
                k->s.setNumDecimalPlacesToDisplay (2);
                k->s.textFromValueFunction = [] (double v)
                {
                    if (std::abs (v) >= 1000.0)
                        return juce::String ((int) std::round (v));
                    if (std::abs (v - std::round (v)) < 1e-3)
                        return juce::String ((int) std::round (v));
                    if (std::abs (v) >= 100.0)
                        return juce::String (v, 0);
                    if (std::abs (v) >= 10.0)
                        return juce::String (v, 1);
                    return juce::String (v, 2);
                };
                k->s.setBounds (box);
                k->s.setVisible (true);
            }
        };

        auto b = mainTab.getLocalBounds().reduced (2);
        // FL Studio sometimes reports 0-size mid-session — skip only MAIN placement
        if (b.getWidth() >= 200 && b.getHeight() >= 120)
        {
        // ADSR strip taller so ATTACK/DECAY/SUSTAIN/RELEASE are visible
        envTab.setBounds (b.removeFromBottom (130));
        filterTab.setBounds (b.removeFromRight (220));
        auto oscFull = b;
        oscTab.setBounds (oscFull);
        oscTab.setVisible (true);
        filterTab.setVisible (true);
        envTab.setVisible (true);
        mainTab.setVisible (true);

        {
            auto oa = oscTab.getLocalBounds().reduced (2);
            const int gap = 12;
            const int colW = (oa.getWidth() - gap * 2) / 3;

        // SALEK icons over OSC / FILTER headers
        if (tabSectionIcons.size() >= 5)
        {
            const int ih = 28;
            for (int c = 0; c < 3; ++c)
            {
                auto col = juce::Rectangle<int> (oa.getX() + c * (colW + gap), oa.getY(), colW, ih);
                tabSectionIcons[c]->setBounds (col.removeFromLeft (ih).reduced (2));
                if (tabSectionIcons[c]->getParentComponent() != &oscTab)
                    oscTab.addAndMakeVisible (*tabSectionIcons[c]);
                tabSectionIcons[c]->setVisible (true);
                tabSectionIcons[c]->toFront (false);
            }
            // filter + env icons
            auto fr = filterTab.getLocalBounds().removeFromTop (28);
            tabSectionIcons[3]->setBounds (fr.removeFromLeft (28).reduced (2));
            if (tabSectionIcons[3]->getParentComponent() != &filterTab)
                filterTab.addAndMakeVisible (*tabSectionIcons[3]);
            tabSectionIcons[3]->setVisible (true);
            auto er = envTab.getLocalBounds().removeFromTop (28);
            tabSectionIcons[4]->setBounds (er.removeFromLeft (28).reduced (2));
            if (tabSectionIcons[4]->getParentComponent() != &envTab)
                envTab.addAndMakeVisible (*tabSectionIcons[4]);
            tabSectionIcons[4]->setVisible (true);
        }

            juce::Component* mons[3] = { oscMon1.get(), oscMon2.get(), oscMon3.get() };
            juce::ComboBox* shapes[3] = { &osc1ShapeBox, &osc2ShapeBox, &osc3ShapeBox };
            const char* idsLvl[3]   = { "osc1_level", "osc2_level", "osc3_level" };
            const char* idsTbl[3]   = { "osc1_table", "osc2_table", "osc3_table" };
            const char* idsWarp[3]  = { "osc1_warp", "osc2_warp", "osc3_warp" };
            const char* idsFold[3]  = { "osc1_fold", "osc2_fold", "osc3_fold" };
            const char* idsDrive[3] = { "osc1_drive", "osc2_drive", "osc3_drive" };
            const char* idsOct[3]   = { "osc1_octave", "osc2_octave", "osc3_octave" };
            const char* idsSemi[3]  = { "osc1_semi", "osc2_semi", "osc3_semi" };
            const char* idsCoarse[3] = { "osc1_coarse", "osc2_coarse", "osc3_coarse" };
            const char* idsUni[3]   = { "osc1_unison", "osc2_unison", "osc3_unison" };
            const char* idsDet[3]   = { "osc1_udet", "osc2_udet", "osc3_udet" };
            const char* idsSpr[3]   = { "osc1_uspread", "osc2_uspread", "osc3_uspread" };
            const char* idsPh[3]    = { "osc1_phase", "osc2_phase", "osc3_phase" };
            const char* idsRnd[3]   = { "osc1_rand", "osc2_rand", "osc3_rand" };
            const char* idsPan[3]   = { "osc1_pan", "osc2_pan", "osc3_pan" };

            for (int c = 0; c < 3; ++c)
            {
                auto col = juce::Rectangle<int> (oa.getX() + c * (colW + gap), oa.getY(), colW, oa.getHeight()).reduced (2, 2);
                auto monH = juce::jlimit (36, 56, col.getHeight() / 5);
                if (mons[c] != nullptr) mons[c]->setBounds (col.removeFromTop (monH).reduced (1));
                shapes[c]->setBounds (col.removeFromTop (18).reduced (1));

                // Equal rows so knobs stay readable
                const int rowH = juce::jmax (48, col.getHeight() / 5);
                auto row1 = col.removeFromTop (rowH);
                placeId (row1.removeFromLeft (row1.getWidth() / 2).reduced (3), idsLvl[c]);
                placeId (row1.reduced (3), idsTbl[c]);
                auto row2 = col.removeFromTop (rowH);
                placeId (row2.removeFromLeft (row2.getWidth() / 2).reduced (3), idsWarp[c]);
                placeId (row2.reduced (3), idsFold[c]);
                auto row3 = col.removeFromTop (rowH);
                const int pitchCellW = row3.getWidth() / 4;
                placeId (row3.removeFromLeft (pitchCellW).reduced (2), idsDrive[c]);
                placeId (row3.removeFromLeft (pitchCellW).reduced (2), idsOct[c]);
                placeId (row3.removeFromLeft (pitchCellW).reduced (2), idsSemi[c]);
                placeId (row3.reduced (2), idsCoarse[c]);
                auto uniRow = col.removeFromTop (rowH);
                const int uw = uniRow.getWidth() / 3;
                placeId (uniRow.removeFromLeft (uw).reduced (2), idsUni[c]);
                placeId (uniRow.removeFromLeft (uw).reduced (2), idsDet[c]);
                placeId (uniRow.reduced (2), idsSpr[c]);
                auto pr = col;
                auto pr3 = pr.getWidth() / 3;
                placeId (pr.removeFromLeft (pr3).reduced (2), idsPh[c]);
                placeId (pr.removeFromLeft (pr3).reduced (2), idsRnd[c]);
                placeId (pr.reduced (2), idsPan[c]);
            }
        }
        {
            auto fr = filterTab.getLocalBounds().reduced (3);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (48).reduced (2));
            auto modeRow = fr.removeFromTop (20);
            filterMode.setBounds (modeRow.removeFromLeft (modeRow.getWidth() / 2).reduced (1));
            filterRouteBox.setBounds (modeRow.reduced (1));
            // Clear vertical stack: CUTOFF (big) | RESO | F DRIVE | F ENV
            const int fh = juce::jmax (40, fr.getHeight() / 4);
            placeId (fr.removeFromTop (fh).reduced (3, 2), "filter_cutoff");
            placeId (fr.removeFromTop (fh).reduced (3, 2), "filter_reso");
            placeId (fr.removeFromTop (fh).reduced (3, 2), "filter_drive");
            placeId (fr.reduced (3, 2), "filter_env");
        }
        {
            auto er = envTab.getLocalBounds().reduced (4);
            // ADSR curve
            if (adsrDisplay != nullptr)
            {
                auto curve = er.removeFromLeft (120);
                adsrDisplay->setBounds (curve.reduced (2));
            }
            // ATTACK DECAY SUSTAIN RELEASE — fixed wide cells so they never disappear
            auto adsrKnobs = er.removeFromLeft (320);
            {
                const int aw = adsrKnobs.getWidth() / 4;
                placeId (juce::Rectangle<int> (adsrKnobs.getX(), adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_attack");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_decay");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw * 2, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_sustain");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw * 3, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_release");
            }
            // Mono + glide / voices / noise / sub on the right
            voiceModeBox.setBounds (er.removeFromTop (22).removeFromLeft (120).reduced (1));
            const int ew = juce::jmax (1, er.getWidth() / 4);
            placeId (er.removeFromLeft (ew).reduced (3, 2), "glide");
            placeId (er.removeFromLeft (ew).reduced (3, 2), "poly_voices");
            placeId (er.removeFromLeft (ew).reduced (3, 2), "noise_level");
            placeId (er.reduced (3, 2), "sub_level");
        }
    }


        } // end valid MAIN size
    // FL recovery: never leave MAIN knobs invisible after tab switch / host resize
    {
        auto forceVis = [] (juce::Component& c)
        {
            c.setVisible (true);
            for (int i = 0; i < c.getNumChildComponents(); ++i)
                if (auto* ch = c.getChildComponent (i))
                    ch->setVisible (true);
        };
        forceVis (oscTab);
        forceVis (filterTab);
        forceVis (envTab);
        forceVis (mainTab);
    }

    // MOD — matrix left; clean FM grid + macros right (hide stray knobs)
    {

        // MOD icons: MATRIX / MACRO / FM
        if (tabSectionIcons.size() >= 11)
        {
            auto top = modTab.getLocalBounds().removeFromTop (32).reduced (4, 2);
            for (int i = 0; i < 3; ++i)
            {
                auto* ic = tabSectionIcons[8 + i];
                ic->setBounds (top.removeFromLeft (42).reduced (2));
                if (ic->getParentComponent() != &modTab)
                    modTab.addAndMakeVisible (*ic);
                ic->setVisible (true);
                ic->toFront (false);
            }
        }

        auto r = modTab.getLocalBounds().reduced (4);
        auto right = r.removeFromRight (juce::jmin (300, r.getWidth() * 34 / 100));

        // Only hide MOD-owned params (never touch osc/filter/env parents)
        static const char* modOnly[] = {
            "fm_2to1","fm_3to1","fm_3to2","pm_2to1","rm_2to1","am_2to1",
            "macro1","macro2","macro3","macro4","lfo_rate","lfo_amount"
        };
        for (auto& kk : knobs)
        {
            if (kk == nullptr) continue;
            bool isMod = false;
            for (auto* id : modOnly)
                if (kk->paramId == id) { isMod = true; break; }
            if (isMod)
            {
                kk->s.setVisible (false);
                kk->name.setVisible (false);
            }
        }

        auto byId = [this] (const char* id) -> Knob*
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id) return k.get();
            return nullptr;
        };
        auto placeOne = [&] (juce::Rectangle<int> cell, const char* id)
        {
            auto* k = byId (id);
            if (k == nullptr) return;
            k->name.setBounds (cell.removeFromBottom (14));
            k->name.setJustificationType (juce::Justification::centred);
            k->name.setFont (juce::FontOptions (10.f, juce::Font::bold));
            k->name.setVisible (true);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (40, cell.getWidth() - 8), 13);
            k->s.setNumDecimalPlacesToDisplay (2);
            k->s.setBounds (cell);
            k->s.setVisible (true);
        };

        // FM block top of right rail (2 rows x 3 cols)
        auto fmArea = right.removeFromTop (juce::jmax (160, right.getHeight() * 55 / 100)).reduced (4, 4);
        const char* fmIds[] = { "fm_2to1","fm_3to1","fm_3to2","pm_2to1","rm_2to1","am_2to1" };
        const int fmCols = 3, fmRows = 2;
        const int cellW = fmArea.getWidth() / fmCols;
        const int cellH = fmArea.getHeight() / fmRows;
        for (int i = 0; i < 6; ++i)
        {
            auto cell = juce::Rectangle<int> (
                fmArea.getX() + (i % fmCols) * cellW,
                fmArea.getY() + (i / fmCols) * cellH, cellW, cellH).reduced (5, 3);
            placeOne (cell, fmIds[i]);
        }

        // Macros bottom of right rail — 4 equal columns
        auto macroBand = right.reduced (4, 6);
        const char* macIds[] = { "macro1","macro2","macro3","macro4" };
        const int cw = juce::jmax (1, macroBand.getWidth() / 4);
        for (int i = 0; i < 4; ++i)
        {
            auto cell = juce::Rectangle<int> (macroBand.getX() + i * cw, macroBand.getY(),
                                              cw, macroBand.getHeight()).reduced (4, 2);
            placeOne (cell, macIds[i]);
        }

        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.reduced (2));
    }

    // LFO
    {

        // LFO tab icons (LFO / SYNC / RND)
        if (tabSectionIcons.size() >= 8)
        {
            auto top = lfoTab.getLocalBounds().removeFromTop (42).reduced (4, 2);
            for (int i = 0; i < 3; ++i)
            {
                auto* ic = tabSectionIcons[5 + i];
                ic->setBounds (top.removeFromLeft (32).reduced (2));
                if (ic->getParentComponent() != &lfoTab)
                    lfoTab.addAndMakeVisible (*ic);
                ic->setVisible (true);
                ic->toFront (false);
            }
        }

        auto r = lfoTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr) lfoDisplay->setBounds (r.removeFromTop (56).reduced (1));
        auto presetRow = r.removeFromTop (24);
        const int bw = presetRow.getWidth() / 12;
        juce::TextButton* presets[] = {
            &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw, &lfoPresetSqr,
            &lfoPresetPulse, &lfoPresetExp, &lfoPresetLog, &lfoPresetBell,
            &lfoPresetWob, &lfoPresetChaos, &lfoPresetGate, &lfoPresetCustom
        };
        for (auto* b : presets) b->setBounds (presetRow.removeFromLeft (bw).reduced (1));
        auto waveRow = r.removeFromTop (24);
        lfo1WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 3).reduced (2));
        lfo2WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 2).reduced (2));
        lfo3WaveBox.setBounds (waveRow.reduced (2));
        auto bankRow = r.removeFromTop (24);
        lfoPointsBox.setBounds (bankRow.removeFromRight (90).reduced (2));
        lfoShapeBank.setBounds (bankRow.reduced (2));
        auto copyRow = r.removeFromTop (22);
        const int cw = copyRow.getWidth() / 9;
        lfoCopyTo1.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo2.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo3.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveA.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadA.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveB.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadB.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveC.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadC.setBounds (copyRow.reduced (1));
        auto knobArea = r.removeFromBottom (96);
        {
            const char* lids[] = { "lfo_rate","lfo_amount","lfo2_rate","lfo2_amount","lfo3_rate","lfo3_amount" };
            const int lfoCw = juce::jmax (1, knobArea.getWidth() / 6);
            for (int i = 0; i < 6; ++i)
            {
                Knob* k = nullptr;
                for (auto& kk : knobs)
                    if (kk != nullptr && kk->paramId == lids[i]) { k = kk.get(); break; }
                if (k == nullptr) continue;
                auto cell = juce::Rectangle<int> (knobArea.getX() + i * lfoCw, knobArea.getY(), lfoCw, knobArea.getHeight()).reduced (3, 2);
                k->name.setBounds (cell.removeFromBottom (14));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, lfoCw - 10), 12);
                k->s.setBounds (cell);
                k->s.setVisible (true);
            }
        }
        lfoShapeEditor.setBounds (r.reduced (2));
    }

    #include "PluginEditorFullB_FxLayout.inl"

    // MAGIC: pad + combo presets only (8 mode/hold buttons removed per UX)
    {
        auto r = magicTab.getLocalBounds().reduced (6);
        // Keep HOLD visible so XY pad stays active without finger-down
        magicLoopBtn.setVisible (false);
        magicGlitchBtn.setVisible (false);
        magicFlangeBtn.setVisible (false);
        magicPsychBtn.setVisible (false);
        magicHint.setVisible (false);
        for (int c = 0; c < magicTab.getNumChildComponents(); ++c)
            if (auto* b = dynamic_cast<juce::TextButton*> (magicTab.getChildComponent (c)))
            {
                auto t = b->getButtonText();
                if (t.startsWith ("H-") || t == "LOOP" || t == "GLITCH" || t == "FLANGE" || t == "PSY")
                    b->setVisible (false);
            }
        // HOLD strip under combo row is placed with pad
        magicHold.setButtonText ("HOLD XY");
        magicHold.setVisible (true);
        magicHold.toFront (false);

        // Combo presets strip bottom
        {
            auto row = r.removeFromBottom (52).reduced (2, 1);
            const int n = 14;
            const int cw = juce::jmax (36, row.getWidth() / n);
            for (int i = 0; i < n; ++i)
                if (auto* c = magicTab.findChildWithID ("magicCombo" + juce::String (i)))
                {
                    c->setVisible (true);
                    c->setBounds (row.getX() + i * cw, row.getY(), cw - 2, row.getHeight());
                }
        }
        // HOLD + MAGIC X/Y (hold keeps FX active so XY works continuously)
        {
            auto byId = [this] (const char* id) -> Knob* {
                for (auto& k : knobs)
                    if (k != nullptr && k->paramId == id) return k.get();
                return nullptr;
            };
            auto placeM = [&] (juce::Rectangle<int> cell, const char* id) {
                if (auto* k = byId (id))
                {
                    k->name.setBounds (cell.removeFromBottom (14));
                    k->name.setVisible (true);
                    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 12);
                    k->s.setBounds (cell);
                    k->s.setVisible (true);
                }
            };
            auto xyCol = r.removeFromRight (110).reduced (4, 2);
            magicHold.setBounds (xyCol.removeFromTop (36).reduced (2));
            magicHold.setVisible (true);
            placeM (xyCol.removeFromTop (xyCol.getHeight() / 2).reduced (2), "magic_x");
            placeM (xyCol.reduced (2), "magic_y");
        }
        if (magicPad != nullptr)
        {
            magicPad->setBounds (r.reduced (4));
            magicPad->setAudioPeak (processor.getOutputPeak());
            magicPad->setVisible (true);
        }
    }

    // SEQ
    {
        auto bounds = seqTab.getLocalBounds().reduced (6);
        auto top = bounds.removeFromTop (116);
        auto toggles = top.removeFromLeft (120);
        arpOn.setBounds (toggles.removeFromTop (36).reduced (2));
        seqOn.setBounds (toggles.removeFromTop (36).reduced (2));
        seqMagicTargetBox.setBounds (toggles.reduced (2, 1));
        seqMagicTargetBox.setTooltip ("Choose whether the running step sequence moves Magic X, Y, or both.");
        auto placeSeqKnob = [this] (juce::Rectangle<int> cell, const char* id)
        {
            for (auto& owned : knobs)
                if (owned != nullptr && owned->paramId == id)
                {
                    cell = cell.reduced (2, 0);
                    owned->name.setBounds (cell.removeFromBottom (13));
                    owned->name.setJustificationType (juce::Justification::centred);
                    owned->name.setVisible (true);
                    owned->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 12);
                    owned->s.setBounds (cell);
                    owned->s.setVisible (true);
                    break;
                }
        };
        auto controls = top.reduced (4, 1);
        const int controlWidth = controls.getWidth() / 5;
        const char* seqIds[] = { "seq_rate", "seq_length", "seq_swing", "seq_gate", "seq_magic_depth" };
        for (int i = 0; i < 5; ++i)
        {
            placeSeqKnob ({ controls.getX() + i * controlWidth, controls.getY(), controlWidth, controls.getHeight() }, seqIds[i]);
        }
        // 2 rows of Magic trigger buttons under seq strip
        {
            auto magicStrip = bounds.removeFromBottom (64).reduced (2, 1);
            seqMagicOff.setBounds (magicStrip.removeFromRight (72).reduced (2));
            seqMagicOff.setVisible (true);
            auto row1 = magicStrip.removeFromTop (30);
            auto row2 = magicStrip;
            const int n = seqMagicBtns.size();
            const int half = n / 2;
            if (half > 0)
            {
                const int cw1 = juce::jmax (28, row1.getWidth() / half);
                const int cw2 = juce::jmax (28, row2.getWidth() / juce::jmax (1, n - half));
                for (int i = 0; i < half; ++i)
                    if (auto* b = seqMagicBtns[i])
                    {
                        b->setBounds (row1.getX() + i * cw1, row1.getY(), cw1 - 2, row1.getHeight());
                        b->setVisible (true);
                    }
                for (int i = half; i < n; ++i)
                    if (auto* b = seqMagicBtns[i])
                    {
                        b->setBounds (row2.getX() + (i - half) * cw2, row2.getY(), cw2 - 2, row2.getHeight());
                        b->setVisible (true);
                    }
            }
        }
        bounds.removeFromTop (4);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds);
    }

    if (speedMonitor != nullptr)
        speedMonitor->setBounds (speedTab.getLocalBounds().reduced (14));
    if (aboutInfo != nullptr)
        aboutInfo->setBounds (aboutTab.getLocalBounds().reduced (8));
}
