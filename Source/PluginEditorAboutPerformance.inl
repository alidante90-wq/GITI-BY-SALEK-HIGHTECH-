#pragma once

class AudioSpeedMonitor : public juce::Component, private juce::Timer
{
public:
    explicit AudioSpeedMonitor (SalekHightechAudioProcessor& p) : processor (p) { startTimerHz (8); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (10.f);
        g.setColour (juce::Colour (0xff080610).withAlpha (0.94f));
        g.fillRoundedRectangle (r, 14.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.55f));
        g.drawRoundedRectangle (r, 14.f, 1.4f);

        auto inner = r.reduced (24.f, 20.f);
        g.setColour (juce::Colour (0xffffd700));
        g.setFont (juce::FontOptions (18.f, juce::Font::bold));
        g.drawText ("AUDIO SPEED MONITOR", inner.removeFromTop (34.f).toNearestInt(), juce::Justification::centredLeft);
        inner.removeFromTop (8.f);

        const float load = processor.getAudioLoadPercent();
        const float peak = processor.getAudioPeakLoadPercent();
        const float callbackMs = processor.getAudioCallbackMs();
        const double sr = processor.getSampleRate();
        const int block = processor.getBlockSize();
        const float budgetMs = sr > 0.0 ? (1000.f * (float) block / (float) sr) : 0.f;

        auto stats = inner.removeFromTop (78.f);
        const int colW = stats.getWidth() / 3;
        drawStat (g, stats.removeFromLeft (colW), "DSP LOAD", juce::String (load, 1) + "%", load > 85.f ? juce::Colour (0xffff4778) : juce::Colour (0xff00e8ff));
        drawStat (g, stats.removeFromLeft (colW), "CALLBACK", juce::String (callbackMs, 3) + " ms", juce::Colour (0xffff2d9b));
        drawStat (g, stats, "BLOCK BUDGET", juce::String (budgetMs, 2) + " ms", juce::Colour (0xff39ff14));

        inner.removeFromTop (12.f);
        g.setColour (juce::Colours::white.withAlpha (0.72f));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.drawText ("REAL-TIME DEADLINE", inner.removeFromTop (18.f).toNearestInt(), juce::Justification::centredLeft);
        auto bar = inner.removeFromTop (30.f).reduced (0.f, 5.f);
        g.setColour (juce::Colour (0xff171324));
        g.fillRoundedRectangle (bar, 7.f);
        for (float t : { 0.70f, 0.90f })
        {
            g.setColour (juce::Colour (t > 0.8f ? 0xffff4778 : 0xffffd700).withAlpha (0.65f));
            const float x = bar.getX() + bar.getWidth() * t;
            g.drawVerticalLine ((int) x, bar.getY(), bar.getBottom());
        }
        const float amount = juce::jlimit (0.f, 1.f, load / 100.f);
        const auto loadColour = load >= 90.f ? juce::Colour (0xffff4778)
                             : load >= 70.f ? juce::Colour (0xffffd700)
                                            : juce::Colour (0xff00e8ff);
        if (amount > 0.f)
        {
            g.setColour (loadColour.withAlpha (0.85f));
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * amount), 7.f);
        }
        g.setColour (juce::Colours::white);
        g.drawText (juce::String (load, 1) + "%  |  peak " + juce::String (peak, 1) + "%",
                    bar.toNearestInt(), juce::Justification::centred);

        inner.removeFromTop (16.f);
        g.setColour (juce::Colour (0xffb9b3c8));
        g.setFont (juce::FontOptions (11.f));
        g.drawFittedText ("This measures SALEK's audio callback against the host's current block deadline. Below 100% means the callback finished within its time budget. It does not measure the whole DAW or other plugins.",
                          inner.toNearestInt(), juce::Justification::topLeft, 3);
    }

private:
    SalekHightechAudioProcessor& processor;

    static void drawStat (juce::Graphics& g, juce::Rectangle<float> r,
                          const juce::String& title, const juce::String& value, juce::Colour colour)
    {
        r = r.reduced (5.f, 2.f);
        g.setColour (juce::Colour (0xff12101d));
        g.fillRoundedRectangle (r, 8.f);
        g.setColour (colour.withAlpha (0.65f));
        g.drawRoundedRectangle (r, 8.f, 1.f);
        g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.setColour (colour);
        g.drawText (title, r.removeFromTop (26.f).toNearestInt(), juce::Justification::centred);
        g.setFont (juce::FontOptions (17.f, juce::Font::bold));
        g.setColour (juce::Colours::white);
        g.drawText (value, r.toNearestInt(), juce::Justification::centred);
    }

    void timerCallback() override { if (isShowing()) repaint(); }
};

class AboutInfoPanel : public juce::Component
{
public:
    AboutInfoPanel()
    {
        title.setText ("SALEK HIGHTECH  |  GITI AUDIO INSTRUMENT", juce::dontSendNotification);
        title.setFont (juce::FontOptions (17.f, juce::Font::bold));
        title.setColour (juce::Label::textColourId, juce::Colour (0xffffd700));
        title.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (title);

        body.setMultiLine (true);
        body.setReadOnly (true);
        body.setCaretVisible (false);
        body.setScrollbarsShown (true);
        body.setFont (juce::FontOptions (13.f));
        body.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0b0914).withAlpha (0.92f));
        body.setColour (juce::TextEditor::textColourId, juce::Colour (0xffe3deed));
        body.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff00e8ff).withAlpha (0.45f));
        body.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xffff2d9b));
        addAndMakeVisible (body);
        setPersian (false);
    }

    void setPersian (bool usePersian)
    {
        const char* english = R"(PURPOSE
SALEK HIGHTECH is a playable software synthesizer built around the GITI sound world. It is designed for direct playing and hands-on sound shaping inside a DAW.

SOUND ENGINE
Three wavetable oscillators provide table position, warp, fold, drive, phase, tuning, stereo pan and unison. FM, PM, AM and ring modulation let one oscillator shape another. SHAE smooths parameter motion, limits unsafe output and reduces harsh high-frequency aliasing. The filter and amp envelope shape each voice; the master stage stays clean unless Master Drive is raised.

MODULATION
Three LFOs provide rate and shape controls, including editable curves. In MOD, add source-to-destination routes, drag an amount bar to set depth, or drag an LFO source onto a knob. Macros provide hands-on control for multiple mapped destinations.

PERFORMANCE
The SEQ tab combines a 16-step note sequencer with a per-step motion lane. Select Magic X, Y or X+Y to animate the Magic pad from the sequence. The ARP and computer/MIDI keyboard provide other ways to perform. Magic effects respond to the X/Y pad and can be held.

EFFECTS AND PRESETS
The FX page contains chorus, delay, reverb, phaser, distortion, equalization, compression and spatial processing. The factory bank includes 50 GITI identities with 50 native variations each. Presets are starting points: use the controls and modulation system to shape them for a track.

MONITORING
The SPEED page measures this instrument's audio callback time against the current host buffer deadline. It is an instrument-only estimate, not a system-wide or whole-DAW CPU meter. Actual performance depends on the host, sample rate, buffer size, polyphony and effects.

QUICK START
1. Choose a preset, or select INIT for a clean starting patch.
2. Play notes from the DAW, MIDI keyboard or the on-screen keyboard.
3. Shape the oscillators and filter on MAIN; use LFO and MOD to add motion.
4. Add effects as needed and watch SPEED if the audio begins to overload.
)";
        const char* persian = u8R"(درباره SALEK HIGHTECH
SALEK HIGHTECH یک ساز نرم‌افزاری برای اجرا و طراحی صدا در DAW است که بر پایه دنیای صوتی GITI ساخته شده است.

موتور صدا
سه اسیلاتور ویوتیبل، تیونینگ، پن، یونی‌سون، FM، PM، AM و مدولاسیون رینگ در اختیار شماست. موتور SHAE حرکت پارامترها را نرم می‌کند و خروجی را ایمن نگه می‌دارد. فیلتر و پوش دامنه شکل هر نت را می‌سازند. درایو مستر فقط وقتی فعال است که خودتان آن را بالا ببرید.

مدولاسیون
سه LFO با شکل‌های آماده و منحنی قابل ویرایش دارید. در تب MOD مسیر منبع تا مقصد بسازید و نوار مقدار را بکشید تا عمق تغییر کند؛ یا LFO را روی ناب موردنظر رها کنید. ماکروها برای کنترل هم‌زمان چند مقصد هستند.

اجرا و سکوینس
تب SEQ شامل سکوینسر ۱۶ پله‌ای نت و مسیر حرکت جداگانه برای هر پله است. مقصد Magic X، Y یا X+Y را انتخاب کنید تا پد Magic با سکوینس حرکت کند. آرپژیاتور و کیبورد MIDI یا کیبورد روی صفحه نیز برای اجرا هستند. افکت‌های Magic به پد X/Y پاسخ می‌دهند و حالت Hold دارند.

افکت و پریست
تب FX شامل Chorus، Delay، Reverb، Phaser، Distortion، اکولایزر، کمپرسور و پردازش فضایی است. بانک کارخانه ۵۰ هویت GITI و برای هرکدام ۵۰ گونه صوتی بومی دارد. پریست‌ها نقطه شروع‌اند؛ با ناب‌ها و مدولاسیون آن‌ها را برای قطعه خود شکل دهید.

مانیتور سرعت
تب SPEED زمان پردازش صوتی همین ساز را با مهلت بافر میزبان مقایسه می‌کند. این عدد فقط برای این ساز است و CPU کل ویندوز یا DAW را نشان نمی‌دهد. نرخ نمونه‌برداری، اندازه بافر، تعداد نت‌ها و افکت‌ها روی نتیجه اثر دارند.

شروع سریع
۱. یک پریست انتخاب کنید یا برای صدای ساده INIT را بزنید.
۲. از MIDI، کیبورد کامپیوتر یا کیبورد روی صفحه نت بنوازید.
۳. در MAIN اسیلاتور و فیلتر را تنظیم کنید و با LFO و MOD حرکت بیفزایید.
۴. افکت‌ها را به‌اندازه نیاز اضافه کنید؛ اگر صدا قطع‌و‌وصل شد تب SPEED را ببینید.
)";
        body.setText (juce::String::fromUTF8 (usePersian ? persian : english), juce::dontSendNotification);
        body.setCaretPosition (0);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (18);
        title.setBounds (r.removeFromTop (34));
        r.removeFromTop (8);
        body.setBounds (r);
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (6.f);
        g.setColour (juce::Colour (0xff080610).withAlpha (0.90f));
        g.fillRoundedRectangle (r, 12.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.42f));
        g.drawRoundedRectangle (r, 12.f, 1.3f);
    }

private:
    juce::Label title;
    juce::TextEditor body;
};
