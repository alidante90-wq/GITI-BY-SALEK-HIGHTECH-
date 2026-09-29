void SalekHightechAudioProcessorEditor::refreshCharCache()
{
    // Downscale once — full-res PNGs destroy UI FPS
    charImgL = SalekAssets::loadCharPortrait (charPortraitIdx, 480, 720);
    charImgR = {}; // right portrait removed — saves RAM/CPU
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    // ONLY the 2 official backgrounds:
    // 1/2 -> bg_isatis (you + cat / city)
    // 3/4 -> bg_salek (2 girls)
    if (id == 1 || id == 2)
    {
        heroImg = lianImg;
        themeAccent  = (id == 2) ? juce::Colour (0xff39ff14) : juce::Colour (0xff00e8ff);
        themeAccent2 = (id == 2) ? juce::Colour (0xffffab40) : juce::Colour (0xffffd700);
        themePanelBg = (id == 2) ? juce::Colour (0xff061408) : juce::Colour (0xff0a0614);
    }
    else
    {
        heroImg = cyanImg;
        themeAccent  = (id == 3) ? juce::Colour (0xffff2d9b) : juce::Colour (0xffb388ff);
        themeAccent2 = juce::Colour (0xff00e8ff);
        themePanelBg = (id == 3) ? juce::Colour (0xff12081c) : juce::Colour (0xff0c0818);
    }
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    // Toggle between the 2 official arts only
    heroIndex = (heroIndex + 1) % 2;
    themeBox.setSelectedId (heroIndex == 0 ? 1 : 3, juce::sendNotification);
}

void SalekHightechAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    juce::ignoreUnused (e);
}

void SalekHightechAudioProcessorEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused (e, wheel);
}

// Computer keyboard → MIDI even while mouse is on knobs / other controls
bool SalekHightechAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    const auto ch = juce::CharacterFunctions::toLowerCase (key.getTextCharacter());
    if (ch == '-' || ch == '=')
    {
        computerKeyboardBase = juce::jlimit (24, 84, computerKeyboardBase + (ch == '-' ? -12 : 12));
        return true;
    }
    const juce::String rows[] = { "zxcvbnm,./", "asdfghjkl;'", "qwertyuiop[]\\" };
    const int rowStarts[] = { 36, 48, 60 };
    const int whiteNoteOffsets[] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21 };
    int note = -1;
    for (int row = 0; row < 3 && note < 0; ++row)
    {
        const int index = rows[row].indexOfChar (ch);
        if (index >= 0 && index < 13)
            note = juce::jlimit (0, 127, computerKeyboardBase + rowStarts[row] - 36 + whiteNoteOffsets[index]);
    }
    if (note >= 0)
    {
        if (! computerKeyboardHeldKeys.contains ((int) ch))
        {
            const bool noteAlreadyHeld = computerKeyboardHeldNotes.contains (note);
            computerKeyboardHeldNotes.add (note);
            computerKeyboardHeldKeys.add ((int) ch);
            if (! noteAlreadyHeld)
                processor.getKeyboardState().noteOn (1, note, 0.92f);
        }
        return true;
    }
    if (keyboard.keyPressed (key))
        return true;
    return false;
}

bool SalekHightechAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    for (int i = computerKeyboardHeldKeys.size(); --i >= 0;)
    {
        const int keyChar = computerKeyboardHeldKeys.getUnchecked (i);
        if (keyChar < 0 || ! juce::KeyPress::isKeyCurrentlyDown (keyChar))
        {
            const int note = computerKeyboardHeldNotes.getUnchecked (i);
            bool stillHeld = false;
            for (int j = 0; j < computerKeyboardHeldNotes.size(); ++j)
                if (j != i && computerKeyboardHeldNotes.getUnchecked (j) == note) { stillHeld = true; break; }
            if (! stillHeld)
                processor.getKeyboardState().noteOff (1, note, 0.0f);
            computerKeyboardHeldNotes.remove (i);
            computerKeyboardHeldKeys.remove (i);
        }
    }
    if (keyboard.keyStateChanged (isKeyDown))
        return true;
    return false;
}

void SalekHightechAudioProcessorEditor::applyUiLanguage()
{
    tabs.setTabName (0, "MAIN");
    tabs.setTabName (1, "MOD");
    tabs.setTabName (2, "LFO");
    tabs.setTabName (3, "FX");
    tabs.setTabName (4, "MAGIC");
    tabs.setTabName (5, "SEQ");
    tabs.setTabName (6, "SPEED");
    tabs.setTabName (7, "ABOUT");
    aboutInfo->setPersian (uiLangFa);
    magicLoopBtn.setButtonText ("LOOP");
    magicGlitchBtn.setButtonText ("GLITCH");
    magicFlangeBtn.setButtonText ("FLANGE");
    magicPsychBtn.setButtonText ("PSY");
    magicHold.setButtonText ("HOLD");
    for (int i = 0; i < 8; ++i)
        fxBypass[i].setButtonText (juce::CharPointer_UTF8 ("⏻"));
    repaint();
}
