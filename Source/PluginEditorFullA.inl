SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (p), processor (p), keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // RESTORED STUB — full body must be restored from commit ba4be59 / local backup
    // See Source/UI/LFO_DRAG_DROP.md and run Actions after full restore
    setSize (1280, 800);
}
