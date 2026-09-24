
SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    initFactoryPresets();
    loadUserPresetsFromDisk();
}

SalekHightechAudioProcessor::~SalekHightechAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout SalekHightechAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto F = [&](const char* id, const char* n, float a, float b, float d) {
        p.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, n, juce::NormalisableRange<float>(a,b), d)); };
    auto Fs = [&](const char* id, const char* n, float a, float b, float skew, float d) {
        p.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, n,
            juce::NormalisableRange<float>(a, b, 0.0f, skew), d)); };
    auto I = [&](const char* id, const char* n, int a, int b, int d) {
        p.push_back (std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id,1}, n, a, b, d)); };
    auto C = [&](const char* id, const char* n, juce::StringArray ch, int d) {
        p.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1}, n, ch, d)); };
    auto B = [&](const char* id, const char* n, bool d) {
        p.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1}, n, d)); };

    F("osc1_level","OSC1 Level",0,1,0.85f); F("osc2_level","OSC2 Level",0,1,0.0f); F("osc3_level","OSC3 Level",0,1,0.0f);
    F("osc1_table","OSC1 Table",0,1,0); F("osc2_table","OSC2 Table",0,1,0); F("osc3_table","OSC3 Table",0,1,0);
    F("osc1_warp","OSC1 Warp",0,1,0); F("osc2_warp","OSC2 Warp",0,1,0); F("osc3_warp","OSC3 Warp",0,1,0);
    F("osc1_fold","OSC1 Fold",0,1,0); F("osc2_fold","OSC2 Fold",0,1,0); F("osc3_fold","OSC3 Fold",0,1,0);
    F("osc1_drive","OSC1 Drive",0,1,0); F("osc2_drive","OSC2 Drive",0,1,0); F("osc3_drive","OSC3 Drive",0,1,0);
    F("osc1_phase","OSC1 Phase",0,1,0); F("osc2_phase","OSC2 Phase",0,1,0); F("osc3_phase","OSC3 Phase",0,1,0);
    F("osc1_rand","OSC1 Rand",0,1,0); F("osc2_rand","OSC2 Rand",0,1,0); F("osc3_rand","OSC3 Rand",0,1,0);
    F("osc1_pan","OSC1 Pan",0,1,0.5f); F("osc2_pan","OSC2 Pan",0,1,0.5f); F("osc3_pan","OSC3 Pan",0,1,0.5f);
    I("osc1_octave","OSC1 Oct",-3,3,0); I("osc2_octave","OSC2 Oct",-3,3,0); I("osc3_octave","OSC3 Oct",-3,3,0);
    I("osc1_semi","OSC1 Semi",-12,12,0); I("osc2_semi","OSC2 Semi",-12,12,0); I("osc3_semi","OSC3 Semi",-12,12,0);
    I("osc1_coarse","OSC1 Coarse Pitch",-48,48,0); I("osc2_coarse","OSC2 Coarse Pitch",-48,48,0); I("osc3_coarse","OSC3 Coarse Pitch",-48,48,0);
    F("osc1_fine","OSC1 Fine",-100,100,0); F("osc2_fine","OSC2 Fine",-100,100,0); F("osc3_fine","OSC3 Fine",-100,100,0);
    F("osc1_detune","OSC1 Detune",-50,50,0); F("osc2_detune","OSC2 Detune",-50,50,0); F("osc3_detune","OSC3 Detune",-50,50,0);
    I("unison_voices","Unison",1,7,1); F("unison_detune","Uni Detune",0,50,12); F("unison_spread","Uni Spread",0,1,0.7f);
    I("osc1_unison","OSC1 Unison",1,7,1); F("osc1_udet","OSC1 UDet",0,50,12); F("osc1_uspread","OSC1 USpr",0,1,0.7f);
    I("osc2_unison","OSC2 Unison",1,7,1); F("osc2_udet","OSC2 UDet",0,50,12); F("osc2_uspread","OSC2 USpr",0,1,0.7f);
    I("osc3_unison","OSC3 Unison",1,7,1); F("osc3_udet","OSC3 UDet",0,50,12); F("osc3_uspread","OSC3 USpr",0,1,0.7f);
    F("fm_2to1","FM 2to1",0,1,0); F("fm_3to1","FM 3to1",0,1,0); F("fm_3to2","FM 3to2",0,1,0);
    F("pm_2to1","PM 2to1",0,1,0); F("rm_2to1","RM 2to1",0,1,0); F("am_2to1","AM 2to1",0,1,0);
    p.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_cutoff",1}, "Cutoff",
        juce::NormalisableRange<float>(10.0f, 20000.0f, 0.1f, 0.3f), 12000.0f));
    F("filter_reso","Reso",0,1,0.0f);
    F("filter_drive","F Drive",0,1,0); F("filter_env","F Env",0,1,0.0f);
    C("filter_mode","F Mode",{
        "LP12","LP24","HP12","HP24","BP","Notch","Peak","AllPass",
        "AcidLP","Ladder","Comb","Formant","BandRej","LoShelf","HiShelf","PhaserN",
        "DiodeLP","Sallen","Vocal","RingMod","BP-Q","LP+HP","Notch2","Wide"
    },0);
    // Stronger low-end skew so short A/D/R (plucks, dubstep hits) are easy to dial
    Fs("amp_attack","Attack",0.0005f,4.0f,0.18f,0.005f);
    Fs("amp_decay","Decay",0.003f,6.0f,0.22f,0.12f);
    F("amp_sustain","Sustain",0,1,1.0f);
    Fs("amp_release","Release",0.003f,10.0f,0.20f,0.22f);
    F("lfo_rate","LFO1 Rate",0.01f,30,1); F("lfo_amount","LFO1 Amt",0,1,0.0f);
    C("lfo_wave","LFO1 Wave",{"Sine","Triangle","Saw","Square","S&H","Custom","SmoothRnd","Chaos","Pulse","Exp","Sine3","SoftSq"},0);
    F("lfo2_rate","LFO2 Rate",0.01f,30,0.5f); F("lfo2_amount","LFO2 Amt",0,1,0.0f);
    C("lfo2_wave","LFO2 Wave",{"Sine","Triangle","Saw","Square","S&H","Custom","SmoothRnd","Chaos","Pulse","Exp","Sine3","SoftSq"},0);
    F("lfo3_rate","LFO3 Rate",0.01f,30,2.f); F("lfo3_amount","LFO3 Amt",0,1,0.0f);
    C("lfo3_wave","LFO3 Wave",{"Sine","Triangle","Saw","Square","S&H","Custom","SmoothRnd","Chaos","Pulse","Exp","Sine3","SoftSq"},0);
    F("macro1","Macro 1",0,1,0); F("macro2","Macro 2",0,1,0); F("macro3","Macro 3",0,1,0); F("macro4","Macro 4",0,1,0);
    F("macro5","Macro 5",0,1,0); F("macro6","Macro 6",0,1,0); F("macro7","Macro 7",0,1,0); F("macro8","Macro 8",0,1,0);
    F("delay_mix","Delay Mix",0,1,0); F("delay_time","Delay Time",50,800,280); F("delay_fb","Delay FB",0,0.95f,0.35f); F("delay_tone","Delay Tone",0,1,0.72f);
    C("delay_mode","Delay Mode",{"Stereo","PingPong","Mono"},0);
    F("delay_time_l","Delay Time L",20,900,280); F("delay_time_r","Delay Time R",20,900,320);
    F("chorus_mix","Chorus Mix",0,1,0); F("chorus_rate","Chorus Rate",0.05f,5,0.35f); F("chorus_depth","Chorus Depth",0,1,0.5f);
    F("reverb_mix","Reverb Mix",0,1,0.20f); F("reverb_size","Reverb Size",0,1,0.5f); F("reverb_decay","Reverb Decay",0.1f,0.95f,0.55f); F("reverb_damp","Reverb Damping",0,1,0.35f);
    C("reverb_mode","Reverb Mode",{"Room","Hall","Plate","Chamber","Spring"},0);
    F("master_drive","Master Drive",0,1,0); F("master_gain","Master Gain",0,1,0.8f);
    B("arp_on","Arp On",false); I("arp_rate","Arp Rate",1,16,4); I("arp_octaves","Arp Oct",1,4,1); F("arp_gate","Arp Gate",0.05f,1.0f,0.72f); F("arp_swing","Arp Swing",0.0f,0.75f,0.0f);
    B("seq_on","Seq On",false); I("seq_rate","Seq Rate",1,8,4); I("seq_length","Seq Length",1,16,16); F("seq_swing","Seq Swing",0.0f,0.75f,0.0f); F("seq_gate","Seq Gate",0.05f,1.0f,1.0f);
    F("comp_threshold","Comp Thresh",-40,0,-12); F("comp_ratio","Comp Ratio",1,20,4); F("comp_mix","Comp Mix",0,1,0.55f); F("comp_gain","Comp Gain",-12,24,0);
    F("comp_depth","Comp Depth",0,1,0.55f); F("comp_attack","Comp Attack",0.5f,50,8); F("comp_release","Comp Release",10,300,80);
    F("comp_thr_lo","Comp Thr LO",-60,0,-18); F("comp_thr_mid","Comp Thr MID",-60,0,-12); F("comp_thr_hi","Comp Thr HI",-60,0,-8);
    F("eq_low","EQ Low",-12,12,0); F("eq_mid","EQ Mid",-12,12,0); F("eq_high","EQ High",-12,12,0);
    F("spatial_azim","Spatial Azim",-180,180,0); F("spatial_dist","Spatial Dist",0,1,0);
    F("spatial_size","Spatial Size",0,1,0.5f); F("spatial_elev","Spatial Elev",-90,90,0);
    F("input_mix","Input Mix",0,1,0);
    F("phaser_mix","Phaser Mix",0,1,0); F("phaser_rate","Phaser Rate",0.05f,8,0.4f); F("phaser_depth","Phaser Depth",0,1,0.6f);
    F("dist_mix","Dist Mix",0,1,0); F("dist_drive","Dist Drive",0,1,0.3f); F("dist_crush","Bitcrush",0,1,0);
    C("dist_mode","Dist Mode",{"Tube","Sat","Hard","Fold","Bit","Rect"},0);
    F("bassify","Bassify",0,1,0);
    B("chorus_bypass","Chorus Bypass",false);
    B("delay_bypass","Delay Bypass",false);
    B("reverb_bypass","Reverb Bypass",false);
    B("phaser_bypass","Phaser Bypass",false);
    B("dist_bypass","Dist Bypass",false);
    B("comp_bypass","Comp Bypass",false);
    B("eq_bypass","EQ Bypass",false);
    B("bassify_bypass","Bassify Bypass",false);
    F("granular_mix","Granular Mix",0,1,0);
    F("granular_density","Gran Density",0.05f,1,0.4f);
    F("granular_size","Gran Size",0.01f,1,0.25f);
    F("granular_pos","Gran Pos",0,1,0.5f);
    F("granular_pitch","Gran Pitch",0.25f,4,1);
    B("granular_freeze","Gran Freeze",false);
    F("granular_spray","Gran Spray",0,1,0.25f); F("granular_pitchspray","Gran PitchSpray",0,1,0.15f);
    F("granular_feedback","Gran Feedback",0,0.95f,0.2f);
    C("scale_mode","Scale",{"Equal 12-TET","Shur","Segah","Homayun","Mahur","Free Koron"},0);
    F("koron_cents","Koron Cents",-50,50,0);
    F("magic_x","Magic X",0,1,0.5f); F("magic_y","Magic Y",0,1,0.5f);
    B("magic_on","Magic On",false);
    C("magic_mode","Magic Mode",{"Loop","Glitch","FlangeVerb","Psychedelic","Void","Slice","Acid","Crash","Noise"},0);
    // Voice / filter extras
    F("glide","Glide",0,1,0);
    C("voice_mode","Voice Mode",{"Poly","Mono","Legato"},0);
    I("poly_voices","Voices",1,16,8);
    F("noise_level","Noise",0,1,0);
    F("sub_level","Sub",0,1,0);
    C("filter_route","Filter Route",{"All","OSC1","OSC2","OSC3","1+2","1+3","2+3"},0);
    return { p.begin(), p.end() };
}

#include "PluginProcessorPresets.inl"

void SalekHightechAudioProcessor::loadFactoryPreset(int index)
{
    if (index < 0 || index >= (int)factoryPresets.size()) return;
    currentProgram = index;
    for (auto* param : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(param))
            rp->setValueNotifyingHost(rp->getDefaultValue());
    for (auto& kv : factoryPresets[(size_t)index].values)
        if (auto* p = apvts.getParameter(kv.first))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
                rp->setValueNotifyingHost(rp->convertTo0to1(kv.second));
    keyboardState.allNotesOff (0);
    synthEngine.allNotesOff();
}

juce::StringArray SalekHightechAudioProcessor::getPresetNames() const
{
    juce::StringArray n; for (auto& pr : factoryPresets) n.add(pr.name); return n;
}
void SalekHightechAudioProcessor::setCurrentProgram(int index) { loadFactoryPreset(index); }
const juce::String SalekHightechAudioProcessor::getProgramName(int index)
{
    return (index >= 0 && index < (int)factoryPresets.size()) ? factoryPresets[(size_t)index].name : juce::String();
}

void SalekHightechAudioProcessor::prepareToPlay(double sr, int spb)
{
    synthEngine.prepareToPlay(sr, spb);
    delay.prepare(sr, spb);
    chorus.prepare(sr, spb);
    reverb.prepare(sr, spb);
    compressor.prepare(sr, spb);
    eq.prepare(sr, spb);
    spatial.prepare(sr, spb);
    phaser.prepare(sr, spb);
    distortion.prepare(sr, spb);
    magic.prepare(sr, spb);
    granular.prepare(sr);
    arpeggiator.prepare(sr);
    stepSequencer.prepare(sr);
    stepSequencer.initDefaultPattern();
    keyboardState.allNotesOff (0);
    synthEngine.allNotesOff();
    arpeggiator.setEnabled (false);
    stepSequencer.setEnabled (false);
    visualFifo.prepare ((int) sr * 2);
    shaeSafety.prepare (sr, 2);
    lfo1.prepare (sr); lfo2.prepare (sr); lfo3.prepare (sr);
}

bool SalekHightechAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    auto in  = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    if (in != juce::AudioChannelSet::disabled()
        && in != juce::AudioChannelSet::mono()
        && in != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

#include "PluginProcessorUserPresets.inl"
#include "PluginProcessorBankIO.inl"
