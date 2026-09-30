
void SalekHightechAudioProcessor::applyParamsToEngine (int samplesThisBlock)
{
    auto g = [&](const char* id) -> float { if (auto* p = apvts.getRawParameterValue(id)) return p->load(); return 0.f; };
    float o1l = juce::jlimit(0.f,1.f, g("osc1_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Level)*0.5f);
    float o2l = juce::jlimit(0.f,1.f, g("osc2_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Level)*0.5f);
    float o3l = juce::jlimit(0.f,1.f, g("osc3_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Level)*0.5f);
    synthEngine.setOsc1Level(o1l); synthEngine.setOsc2Level(o2l); synthEngine.setOsc3Level(o3l);
    synthEngine.setOsc1Pan(juce::jlimit(0.f,1.f, g("osc1_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Pan)*0.5f));
    synthEngine.setOsc2Pan(juce::jlimit(0.f,1.f, g("osc2_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Pan)*0.5f));
    synthEngine.setOsc3Pan(juce::jlimit(0.f,1.f, g("osc3_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Pan)*0.5f));
    synthEngine.setOsc1TablePos(juce::jlimit(0.f,1.f, g("osc1_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Table)*0.5f));
    synthEngine.setOsc2TablePos(juce::jlimit(0.f,1.f, g("osc2_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Table)*0.5f));
    synthEngine.setOsc3TablePos(juce::jlimit(0.f,1.f, g("osc3_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Table)*0.5f));
    synthEngine.setOsc1Warp(juce::jlimit(0.f,1.f, g("osc1_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Warp)*0.5f));
    synthEngine.setOsc2Warp(juce::jlimit(0.f,1.f, g("osc2_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Warp)*0.5f));
    synthEngine.setOsc3Warp(juce::jlimit(0.f,1.f, g("osc3_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Warp)*0.5f));
    synthEngine.setOsc1Fold(juce::jlimit(0.f,1.f, g("osc1_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Fold)*0.5f));
    synthEngine.setOsc2Fold(juce::jlimit(0.f,1.f, g("osc2_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Fold)*0.5f));
    synthEngine.setOsc3Fold(juce::jlimit(0.f,1.f, g("osc3_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Fold)*0.5f));
    synthEngine.setOsc1Drive(juce::jlimit(0.f,1.f,g("osc1_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Drive)*0.5f));
    synthEngine.setOsc2Drive(juce::jlimit(0.f,1.f,g("osc2_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Drive)*0.5f));
    synthEngine.setOsc3Drive(juce::jlimit(0.f,1.f,g("osc3_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Drive)*0.5f));
    synthEngine.setOsc1Phase(g("osc1_phase")); synthEngine.setOsc2Phase(g("osc2_phase")); synthEngine.setOsc3Phase(g("osc3_phase"));
    synthEngine.setOsc1Rand(g("osc1_rand")); synthEngine.setOsc2Rand(g("osc2_rand")); synthEngine.setOsc3Rand(g("osc3_rand"));
    synthEngine.setOsc1Octave((int)g("osc1_octave")); synthEngine.setOsc2Octave((int)g("osc2_octave")); synthEngine.setOsc3Octave((int)g("osc3_octave"));
    synthEngine.setOsc1Semi((int)g("osc1_semi")); synthEngine.setOsc2Semi((int)g("osc2_semi")); synthEngine.setOsc3Semi((int)g("osc3_semi"));
    synthEngine.setOsc1CoarsePitch((int)g("osc1_coarse")); synthEngine.setOsc2CoarsePitch((int)g("osc2_coarse")); synthEngine.setOsc3CoarsePitch((int)g("osc3_coarse"));
    synthEngine.setUnison((int)g("unison_voices"));
    synthEngine.setUnisonDetune(g("unison_detune"));
    synthEngine.setUnisonSpread(g("unison_spread"));
    // Per-osc unison — always honour UNI/DET/SPR knobs (1..7)
    {
        const int u1 = juce::jlimit (1, 7, (int) std::lround (g("osc1_unison")));
        const int u2 = juce::jlimit (1, 7, (int) std::lround (g("osc2_unison")));
        const int u3 = juce::jlimit (1, 7, (int) std::lround (g("osc3_unison")));
        const float d1 = juce::jmax (0.f, g("osc1_udet"));
        const float d2 = juce::jmax (0.f, g("osc2_udet"));
        const float d3 = juce::jmax (0.f, g("osc3_udet"));
        const float s1 = juce::jlimit (0.f, 1.f, g("osc1_uspread"));
        const float s2 = juce::jlimit (0.f, 1.f, g("osc2_uspread"));
        const float s3 = juce::jlimit (0.f, 1.f, g("osc3_uspread"));
        synthEngine.setOsc1Unison (u1, d1, s1);
        synthEngine.setOsc2Unison (u2, d2, s2);
        synthEngine.setOsc3Unison (u3, d3, s3);
    }
    synthEngine.setFm2to1(juce::jlimit(0.f,1.f, g("fm_2to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Fm2to1)*0.5f));
    synthEngine.setFm3to1(juce::jlimit(0.f,1.f, g("fm_3to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Fm3to1)*0.5f));
    synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(juce::jlimit(0.f,1.f, g("pm_2to1")+modMatrix.getModulation(salek::ModMatrix::Dest::Pitch)*0.35f));
    synthEngine.setRm2to1(g("rm_2to1"));
    synthEngine.setAm2to1(g("am_2to1"));
    float cut = g("filter_cutoff");
    float reso = g("filter_reso");
    // Smart macros: 1=Bright 2=Space 3=Destroy 4=Width
    const float mBright = g("macro1");
    const float mSpace  = g("macro2");
    const float mDest   = g("macro3");
    const float mWidth  = g("macro4");
    cut *= (0.75f + mBright * 1.6f);
    reso = juce::jlimit (0.f, 0.98f, reso + mBright * 0.15f);
    // space → delay/reverb (applied later via param push if available)
    // push smart macros into FX (non-destructive scale)
    if (auto* p = apvts.getParameter ("delay_mix"))
        juce::ignoreUnused (p);
    // runtime scale in render path:
    // stored as members for voice/FX - apply on buffer FX stage below
    const float spaceScale = 0.15f + mSpace * 0.85f;
    const float destScale  = mDest;
    const float widthScale = 0.3f + mWidth * 0.7f;
    cut *= std::pow(2.0f, modMatrix.getModulation(salek::ModMatrix::Dest::FilterCutoff) * 1.0f);
    synthEngine.setFilterCutoff(juce::jlimit(20.f, 20000.f, cut));
    synthEngine.setFilterResonance(juce::jlimit(0.f,1.f, g("filter_reso")+modMatrix.getModulation(salek::ModMatrix::Dest::FilterReso)*0.5f));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterEnvAmt(juce::jlimit(0.f,1.f,g("filter_env")+modMatrix.getModulation(salek::ModMatrix::Dest::FilterEnv)*0.5f));
    synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setFilterRoute((int)g("filter_route"));
    synthEngine.setNoiseLevel(juce::jlimit(0.f,1.f, g("noise_level")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Level)*0.25f));
    synthEngine.setSubLevel(juce::jlimit(0.f,1.f, g("sub_level")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Level)*0.25f));
    synthEngine.setGlide(juce::jlimit(0.f,1.f, g("glide")+std::abs(modMatrix.getModulation(salek::ModMatrix::Dest::Pitch))*0.15f));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    synthEngine.setScaleMode((int)g("scale_mode"));
    synthEngine.setKoronCents(g("koron_cents"));

    auto setLfo = [] (salek::LFO& lfo, float rate, float amt, int wave)
    {
        lfo.setRate (rate);
        lfo.setAmount (1.0f);
        static const salek::LFO::Wave waves[] = {
            salek::LFO::Wave::Sine, salek::LFO::Wave::Triangle, salek::LFO::Wave::Saw,
            salek::LFO::Wave::Square, salek::LFO::Wave::SAndH, salek::LFO::Wave::Custom,
            salek::LFO::Wave::SmoothRnd, salek::LFO::Wave::Chaos, salek::LFO::Wave::Pulse,
            salek::LFO::Wave::Exp, salek::LFO::Wave::Sine3, salek::LFO::Wave::SoftSquare
        };
        lfo.setWave (waves[juce::jlimit (0, 11, wave)]);
        juce::ignoreUnused (amt);
    };
    setLfo (lfo1, g("lfo_rate"), g("lfo_amount"), (int) g("lfo_wave"));
    setLfo (lfo2, g("lfo2_rate"), g("lfo2_amount"), (int) g("lfo2_wave"));
    setLfo (lfo3, g("lfo3_rate"), g("lfo3_amount"), (int) g("lfo3_wave"));

    float v1 = lfo1.processBlock (samplesThisBlock) * g("lfo_amount");
    float v2 = lfo2.processBlock (samplesThisBlock) * g("lfo2_amount");
    float v3 = lfo3.processBlock (samplesThisBlock) * g("lfo3_amount");
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO1, v1);
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO2, v2);
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO3, v3);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro1, g("macro1") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro2, g("macro2") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro3, g("macro3") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro4, g("macro4") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro5, g("macro5") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro6, g("macro6") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro7, g("macro7") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro8, g("macro8") * 2.f - 1.f);
    // Realtime-safe deterministic random source: no global RNG, lock, or allocation in audio callback.
    static thread_local uint32_t shaeRandState = 0x9E3779B9u;
    shaeRandState ^= shaeRandState << 13;
    shaeRandState ^= shaeRandState >> 17;
    shaeRandState ^= shaeRandState << 5;
    const float shaeRandom = (static_cast<float> (shaeRandState & 0x00FFFFFFu) / 16777215.0f) * 2.0f - 1.0f;
    modMatrix.setSourceValue (salek::ModMatrix::Source::Random, shaeRandom);
    delay.setMix(juce::jlimit(0.f,1.f,g("delay_mix")*spaceScale+modMatrix.getModulation(salek::ModMatrix::Dest::DelayMix)*0.5f));
    delay.setTimeMs(g ("delay_time"));
    delay.setFeedback(g("delay_fb"));
    delay.setTone(g ("delay_tone"));
    delay.setMode((int) g("delay_mode"));
    chorus.setMix(juce::jlimit(0.f,1.f,g("chorus_mix")*widthScale+modMatrix.getModulation(salek::ModMatrix::Dest::ChorusMix)*0.5f)); chorus.setRate(g("chorus_rate")); chorus.setDepth(g("chorus_depth"));
    reverb.setMix(juce::jlimit(0.f,1.f,g("reverb_mix")*spaceScale+modMatrix.getModulation(salek::ModMatrix::Dest::ReverbMix)*0.5f)); reverb.setSize(g("reverb_size")); reverb.setDecay(g("reverb_decay"));
    reverb.setMode ((int) g("reverb_mode"));
    reverb.setDamping (g ("reverb_damp"));
    phaser.setMix(juce::jlimit(0.f,1.f,g("phaser_mix")+modMatrix.getModulation(salek::ModMatrix::Dest::PhaserMix)*0.5f)); phaser.setRate(g("phaser_rate")); phaser.setDepth(g("phaser_depth"));
    distortion.setMix(g("dist_mix")); distortion.setDrive(juce::jlimit(0.f,1.f,g("dist_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::DistDrive)*0.5f)); distortion.setBitcrush(g("dist_crush"));
    distortion.setMode ((int) g("dist_mode"));
    compressor.setThresholdDb(g("comp_threshold")); compressor.setRatio(g("comp_ratio")); compressor.setMix(g("comp_mix"));
    compressor.setDepth(g("comp_depth")); compressor.setAttackMs(g("comp_attack")); compressor.setReleaseMs(g("comp_release"));
    compressor.setMakeupDb(g("comp_gain"));
    // Independent LO / MID / HI thresholds (overrides global offsets when set)
    compressor.setBandThresholdDb (0, g("comp_thr_lo"));
    compressor.setBandThresholdDb (1, g("comp_thr_mid"));
    compressor.setBandThresholdDb (2, g("comp_thr_hi"));
    eq.setLowGainDb(g("eq_low")); eq.setMidGainDb(g("eq_mid")); eq.setHighGainDb(g("eq_high"));
    spatial.setAzimuth(g("spatial_azim")); spatial.setDistance(g("spatial_dist"));
    spatial.setSize(g("spatial_size")); spatial.setElevation(g("spatial_elev"));

    magic.setMode ((int) g("magic_mode"));
    {
        float mx = juce::jlimit (0.f, 1.f, g("magic_x") + modMatrix.getModulation (salek::ModMatrix::Dest::MagicX) * 0.5f);
        float my = juce::jlimit (0.f, 1.f, g("magic_y") + modMatrix.getModulation (salek::ModMatrix::Dest::MagicY) * 0.5f);
        const bool seqMotion = g ("seq_on") > 0.5f && g ("seq_magic_target") > 0.5f;
        if (seqMotion)
        {
            const float step = stepSequencer.getCurrentMod();
            const float bipolar = step * 2.0f - 1.0f;
            const float depth = g ("seq_magic_depth");
            const int target = (int) g ("seq_magic_target");
            if (target == 1 || target == 3) mx += bipolar * depth;
            if (target == 2 || target == 3) my += bipolar * depth;
        }
        magic.setXY (mx, my);
        // A selected SEQ→Magic lane is a performance source: it can animate
        // the held effect without needing a MIDI note to start the sequencer.
        magic.setActive (g ("magic_on") > 0.5f || seqMotion);
    }
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    const auto callbackStart = std::chrono::steady_clock::now();
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (presetVoiceResetRequested.exchange (false, std::memory_order_acquire))
    {
        keyboardState.allNotesOff (0);
        synthEngine.allNotesOff();
    }

    applyParamsToEngine (buffer.getNumSamples());
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    const bool seqOn = apvts.getRawParameterValue("seq_on")->load() > 0.5f;
    const bool arpOn = apvts.getRawParameterValue("arp_on")->load() > 0.5f;
    stepSequencer.setEnabled (seqOn);
    arpeggiator.setEnabled (arpOn);

    // Leave MIDI untouched on the common path when both generators are off;
    // copying it into temporary buffers every audio block caused avoidable
    // allocator and event-copy work during ordinary synth playback.
    if (seqOn || arpOn)
    {
        juce::MidiBuffer routed;
        routed.addEvents (midi, 0, buffer.getNumSamples(), 0);

        if (seqOn)
        {
            stepSequencer.setRateDivisor ((int) apvts.getRawParameterValue("seq_rate")->load());
            stepSequencer.setPatternLength ((int) apvts.getRawParameterValue("seq_length")->load());
            stepSequencer.setSwing (apvts.getRawParameterValue("seq_swing")->load());
            stepSequencer.setGate (apvts.getRawParameterValue("seq_gate")->load());
            for (const auto metadata : midi)
            {
                const auto msg = metadata.getMessage();
                if (msg.isNoteOn()) stepSequencer.setRootNote (msg.getNoteNumber());
            }
            stepSequencer.process(buffer.getNumSamples(), routed);
        }

        if (arpOn)
        {
            arpeggiator.setRateDivisor ((int) apvts.getRawParameterValue("arp_rate")->load());
            arpeggiator.setOctaves ((int) apvts.getRawParameterValue("arp_octaves")->load());
            arpeggiator.setGate (apvts.getRawParameterValue("arp_gate")->load());
            arpeggiator.setSwing (apvts.getRawParameterValue("arp_swing")->load());
            juce::MidiBuffer arpIn;
            for (const auto metadata : routed)
            {
                const auto msg = metadata.getMessage();
                if (msg.isNoteOn())
                    arpeggiator.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
                else if (msg.isNoteOff())
                    arpeggiator.noteOff (msg.getNoteNumber());
                else
                    arpIn.addEvent (msg, metadata.samplePosition);
            }
            routed.clear();
            routed.addEvents (arpIn, 0, buffer.getNumSamples(), 0);
            arpeggiator.process (buffer.getNumSamples(), routed);
        }

        midi.swapWith (routed);
    }

    synthEngine.processBlock(buffer, midi);

    auto bypassed = [&](const char* id) -> bool {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load() > 0.5f;
        return false;
    };
    auto g = [&](const char* id) -> float {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return 0.f;
    };
   
    const float bassify = juce::jlimit(0.f,1.f, apvts.getRawParameterValue("bassify")->load() + modMatrix.getModulation(salek::ModMatrix::Dest::Bassify)*0.5f);
    if (bassify > 1e-4f && ! bypassed ("bassify_bypass"))
    {
        static float lp1L = 0.f, lp1R = 0.f, lp2L = 0.f, lp2R = 0.f;
        const float a1 = 0.04f + bassify * 0.06f;
        const float a2 = 0.12f + bassify * 0.10f;
        const float amount = bassify;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float L = buffer.getSample (0, i);
            float R = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : L;
            lp1L += a1 * (L - lp1L); lp1R += a1 * (R - lp1R);
            lp2L += a2 * (lp1L - lp2L); lp2R += a2 * (lp1R - lp2R);
            float sub = 0.5f * (lp2L + lp2R);
            float grit = amount * 0.4f;
            sub = sub + grit * sub * sub * (sub >= 0.f ? 1.f : -1.f);
            sub = std::tanh (sub * (1.2f + amount * 1.6f));
            float dryKeep = 1.f - amount * 0.25f;
            float wet = sub * amount * 0.85f;
            buffer.setSample (0, i, L * dryKeep + wet);
            if (buffer.getNumChannels() > 1)
                buffer.setSample (1, i, R * dryKeep + wet);
        }
    }

    // Mix=0 early-out saves a lot with 2+ instances in FL
    if (! bypassed ("chorus_bypass") && g ("chorus_mix") > 1e-4f)  chorus.process (buffer);
    if (! bypassed ("phaser_bypass") && g ("phaser_mix") > 1e-4f)  phaser.process (buffer);
    if (! bypassed ("dist_bypass") && g ("dist_mix") > 1e-4f)      distortion.process (buffer);
    if (! bypassed ("eq_bypass"))
    {
        if (std::abs (g ("eq_low")) > 0.05f || std::abs (g ("eq_mid")) > 0.05f || std::abs (g ("eq_high")) > 0.05f)
            eq.process (buffer);
    }
    if (! bypassed ("comp_bypass") && g ("comp_mix") > 1e-4f)    compressor.process (buffer);
    if (! bypassed ("delay_bypass") && g ("delay_mix") > 1e-4f)   delay.process (buffer);
    if (! bypassed ("reverb_bypass") && g ("reverb_mix") > 1e-4f) reverb.process (buffer);
    {
        const float az = std::abs (g ("spatial_azim"));
        const float ds = g ("spatial_dist");
        if (az > 0.5f || ds > 1e-3f || std::abs (g ("spatial_elev")) > 0.5f)
            spatial.process (buffer);
    }

    if (auto* gm = apvts.getRawParameterValue ("granular_mix"))
    {
        const float gMix = gm->load();
        if (gMix > 1e-4f)
        {
            granular.setMix (gMix);
            if (auto* p = apvts.getRawParameterValue ("granular_density")) granular.setDensity (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_size"))    granular.setGrainSize (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pos"))     granular.setPosition (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pitch"))   granular.setPitch (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_freeze"))  granular.setFreeze (p->load() > 0.5f);
            if (auto* p = apvts.getRawParameterValue ("granular_spray"))   granular.setSpray (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pitchspray")) granular.setPitchSpray (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_feedback")) granular.setFeedback (p->load());
            granular.process (buffer);
        }
    }

    magic.process (buffer);

    float gain = juce::jlimit(0.f, 2.f, apvts.getRawParameterValue("master_gain")->load()
        + modMatrix.getModulation(salek::ModMatrix::Dest::Amp)*0.35f);
    const float drive = juce::jlimit(0.f, 1.f, apvts.getRawParameterValue("master_drive")->load()
        + modMatrix.getModulation(salek::ModMatrix::Dest::DistDrive)*0.25f);
    // Default is a transparent gain stage. Master Drive adds one intentional
    // normalized saturator; the SHAE safety limiter remains the final ceiling.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = d[i] * gain;
            if (drive > 1.0e-4f)
                x = shae::softClip (x, 1.0f + drive * 4.0f);
            d[i] = x;
        }
    }

    // SHAE final safety stage: DC blocking + finite-value protection + conservative ceiling.
    shaeSafety.process (buffer);

    float peak = 0.f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    outputPeak.store (peak);
    visualFifo.pushStereo (buffer);

    const float elapsedMs = std::chrono::duration<float, std::milli> (
        std::chrono::steady_clock::now() - callbackStart).count();
    const double sampleRate = getSampleRate();
    const float budgetMs = sampleRate > 0.0
        ? (1000.0f * (float) buffer.getNumSamples() / (float) sampleRate) : 0.0f;
    const float load = budgetMs > 0.0f ? juce::jlimit (0.f, 999.f, elapsedMs * 100.f / budgetMs) : 0.f;
    const float prior = audioLoadPercent.load (std::memory_order_relaxed);
    audioLoadPercent.store (prior + 0.18f * (load - prior), std::memory_order_relaxed);
    audioCallbackMs.store (elapsedMs, std::memory_order_relaxed);
    float peakLoad = audioPeakLoadPercent.load (std::memory_order_relaxed);
    while (load > peakLoad && ! audioPeakLoadPercent.compare_exchange_weak (
               peakLoad, load, std::memory_order_relaxed, std::memory_order_relaxed)) {}
}

void SalekHightechAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement root ("SALEK_STATE");
    root.setAttribute ("version", 2);
    root.setAttribute ("program", currentProgram);
    if (auto ap = apvts.copyState().createXml())
        root.addChildElement (new juce::XmlElement (*ap));
    // Persist LFO/mod routes with project + host preset save
    if (auto mx = modMatrix.toXml())
        root.addChildElement (mx.release());
    copyXmlToBinary (root, destData);
}
void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName ("SALEK_STATE"))
        {
            if (auto* ap = xml->getChildByName (apvts.state.getType()))
                apvts.replaceState (juce::ValueTree::fromXml (*ap));
            // Restore mod routes (empty/missing child = clear matrix)
            modMatrix.fromXml (xml->getChildByName ("MOD_MATRIX"));
            const int prog = xml->getIntAttribute ("program", -1);
            if (prog >= 0 && prog < (int) factoryPresets.size())
                currentProgram = prog;
        }
        else if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
    }
}

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor()
{
    return new SalekHightechAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SalekHightechAudioProcessor();
}
