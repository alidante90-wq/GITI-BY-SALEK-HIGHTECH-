// Final layout — reserved space for SALEK HIGHTECH branding + ISATIS + central visualizer
void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();

    // Keyboard at very bottom
    {
        auto kbArea = full.removeFromBottom (72).reduced (2, 2);
        const int whiteKeys = 52;
        int keyW = juce::jmax (11, kbArea.getWidth() / whiteKeys);
        keyboard.setKeyWidth ((float) keyW);
        keyboard.setBounds (kbArea);
    }
    full.removeFromBottom (6); // space for ISATIS text

    // Left branding / logo panel reserved (matches paint)
    full.removeFromLeft (230);

    // Top header strip (title is painted, so just controls)
    auto header = full.removeFromTop (40);
    themeBox.setBounds (header.removeFromRight (140).reduced (3));
    scope.setBounds (header.removeFromRight (120).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (220).reduced (2));

    full.removeFromTop (4);

    // Main content area for tabs
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    // ---- MAIN tab layout ----
    {
        auto b = mainTab.getLocalBounds().reduced (6);
        // Preset list left
        presetTab.setBounds (b.removeFromLeft (200));
        // Filter right
        filterTab.setBounds (b.removeFromRight (260));
        // Env bottom
        envTab.setBounds (b.removeFromBottom (140));
        // Oscillators take remaining center
        oscTab.setBounds (b);

        // Preset controls
        {
            auto pb = presetTab.getLocalBounds().reduced (4);
            auto top = pb.removeFromTop (32);
            prevPreset.setBounds (top.removeFromLeft (32).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (32).reduced (1));
            initBtn.setBounds (top.removeFromLeft (50).reduced (1));
            presetLabel.setBounds (top.reduced (1));
            presetList.setBounds (pb);
        }

        auto layoutKnobs = [] (juce::Component& parent, std::vector<std::unique_ptr<Knob>>& all, int start, int count)
        {
            auto r = parent.getLocalBounds().reduced (4);
            if (count <= 0) return;
            int cols = juce::jmin (count, 5);
            int rows = (count + cols - 1) / cols;
            int cellW = juce::jmax (1, r.getWidth() / cols);
            int cellH = juce::jmax (1, r.getHeight() / juce::jmax (1, rows));
            for (int i = 0; i < count; ++i)
            {
                int idx = start + i;
                if (idx >= (int) all.size()) break;
                auto* k = all[(size_t) idx].get();
                int c = i % cols, row = i / cols;
                auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (3);
                k->name.setBounds (cell.removeFromBottom (14));
                k->s.setBounds (cell);
            }
        };

        layoutKnobs (oscTab, knobs, 0, 17);
        layoutKnobs (filterTab, knobs, 17, 4);
        layoutKnobs (envTab, knobs, 21, 4);

        if (filterDisplay != nullptr)
            filterDisplay->setBounds (filterTab.getLocalBounds().removeFromTop (80).reduced (3));
        if (adsrDisplay != nullptr)
            adsrDisplay->setBounds (envTab.getLocalBounds().removeFromTop (60).reduced (3));
    }

    // ---- MOD tab ----
    {
        auto r = modTab.getLocalBounds().reduced (4);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (58).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromTop (juce::jmin (290, r.getHeight() / 2)).reduced (2));
        int x = r.getX(), y = r.getY(), w = 88, h = 95;
        for (int i = 25; i < juce::jmin (37, (int) knobs.size()); ++i)
        {
            auto* k = knobs[(size_t) i].get();
            k->s.setBounds (x, y, w, h - 16);
            k->name.setBounds (x, y + h - 16, w, 14);
            x += w;
            if (x + w > r.getRight()) { x = r.getX(); y += h; }
        }
    }

    // ---- FX tab ----
    {
        auto r = fxTab.getLocalBounds().reduced (6);
        int cols = 4, i0 = 37;
        int n = juce::jmin (22, (int) knobs.size() - i0);
        int cellW = juce::jmax (1, r.getWidth() / cols);
        int cellH = 105;
        for (int i = 0; i < n; ++i)
        {
            auto* k = knobs[(size_t) (i0 + i)].get();
            int c = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (3);
            k->name.setBounds (cell.removeFromBottom (14));
            k->s.setBounds (cell);
        }
    }

    // ---- SEQ tab ----
    {
        auto bounds = seqTab.getLocalBounds().reduced (4);
        auto top = bounds.removeFromTop (26);
        arpOn.setBounds (top.removeFromLeft (70).reduced (2));
        seqOn.setBounds (top.removeFromLeft (70).reduced (2));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (3));
    }

    // Make sure OpenGL visualizer (if present) sits behind and covers center
    if (sonicCore != nullptr)
    {
        // Position it in the visual center area
        auto center = getLocalBounds().reduced (240, 50).withTrimmedBottom (80);
        sonicCore->setBounds (center);
        sonicCore->toBack();
    }
}
