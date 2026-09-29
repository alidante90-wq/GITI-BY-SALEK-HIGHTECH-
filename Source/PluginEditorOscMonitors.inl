// Three live OSC shape monitors + ASCII shape presets (no broken Persian glyphs)
{
    oscMon1 = std::make_unique<OscShapeMonitor>();
    oscMon2 = std::make_unique<OscShapeMonitor>();
    oscMon3 = std::make_unique<OscShapeMonitor>();
    oscMon1->setAPVTS (&processor.getAPVTS()); oscMon1->setOscIndex (0);
    oscMon1->setAccent (juce::Colour (0xff00e8ff));
    oscMon1->setTitle ("OSC1");
    oscMon2->setAPVTS (&processor.getAPVTS()); oscMon2->setOscIndex (1);
    oscMon2->setAccent (juce::Colour (0xffff2d9b));
    oscMon2->setTitle ("OSC2");
    oscMon3->setAPVTS (&processor.getAPVTS()); oscMon3->setOscIndex (2);
    oscMon3->setAccent (juce::Colour (0xff39ff14));
    oscMon3->setTitle ("OSC3");
    oscTab.addAndMakeVisible (*oscMon1);
    oscTab.addAndMakeVisible (*oscMon2);
    oscTab.addAndMakeVisible (*oscMon3);

    auto shapeItems = juce::StringArray {
        "Sine", "Saw", "Square", "Triangle",
        "Fold", "Alien", "Fuzzy", "Custom"
    };

    auto applyShape = [this] (int osc, int idx)
    {
        static const float map[8][3] = {
            { 0.00f, 0.00f, 0.00f },
            { 3.0f / 127.0f, 0.00f, 0.00f },
            { 2.0f / 127.0f, 0.00f, 0.00f },
            { 1.0f / 127.0f, 0.00f, 0.00f },
            { 0.35f, 0.25f, 0.55f },
            { 0.78f, 0.55f, 0.30f },
            { 0.60f, 0.40f, 0.20f },
            { 0.50f, 0.20f, 0.15f }
        };
        idx = juce::jlimit (0, 7, idx);
        const char* tid[] = { "osc1_table", "osc2_table", "osc3_table" };
        const char* wid[] = { "osc1_warp",  "osc2_warp",  "osc3_warp" };
        const char* fid[] = { "osc1_fold",  "osc2_fold",  "osc3_fold" };
        auto setP = [&] (const char* id, float v)
        {
            if (auto* p = processor.getAPVTS().getParameter (id))
                p->setValueNotifyingHost (p->convertTo0to1 (v));
        };
        setP (tid[osc], map[idx][0]);
        setP (wid[osc], map[idx][1]);
        setP (fid[osc], map[idx][2]);
    };

    auto setupBox = [&] (juce::ComboBox& box, int osc, juce::Colour col)
    {
        box.clear (juce::dontSendNotification);
        box.addItemList (shapeItems, 1);
        box.setSelectedItemIndex (0, juce::dontSendNotification);
        box.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        box.setColour (juce::ComboBox::textColourId, col);
        oscTab.addAndMakeVisible (box);
        box.onChange = [this, &box, osc, applyShape]
        {
            applyShape (osc, box.getSelectedItemIndex());
        };
    };
    setupBox (osc1ShapeBox, 0, juce::Colour (0xff00e8ff));
    setupBox (osc2ShapeBox, 1, juce::Colour (0xffff2d9b));
    setupBox (osc3ShapeBox, 2, juce::Colour (0xff39ff14));
}
