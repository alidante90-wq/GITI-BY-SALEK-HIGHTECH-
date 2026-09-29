#include "GITI/GitiPresetBank.h"

void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };

    // INIT — pure soft 1-osc sine, no LFO/FX/mod, flat sustain (dry beep)
    add("Init/Init", {
        {"osc1_level",0.85f},{"osc2_level",0.0f},{"osc3_level",0.0f},
        {"osc1_table",0.0f},{"osc2_table",0.0f},{"osc3_table",0.0f},
        {"osc1_warp",0},{"osc2_warp",0},{"osc3_warp",0},
        {"osc1_fold",0},{"osc2_fold",0},{"osc3_fold",0},
        {"osc1_drive",0},{"osc2_drive",0},{"osc3_drive",0},
        {"osc1_octave",0},{"osc2_octave",0},{"osc3_octave",0},
        {"osc1_semi",0},{"osc2_semi",0},{"osc3_semi",0},
        {"osc1_fine",0},{"osc2_fine",0},{"osc3_fine",0},
        {"osc1_unison",1},{"osc2_unison",1},{"osc3_unison",1},
        {"osc1_udet",0},{"osc2_udet",0},{"osc3_udet",0},
        {"osc1_uspread",0},{"osc2_uspread",0},{"osc3_uspread",0},
        {"osc1_phase",0},{"osc1_rand",0},{"osc1_pan",0.5f},
        {"osc2_phase",0},{"osc2_rand",0},{"osc2_pan",0.5f},
        {"osc3_phase",0},{"osc3_rand",0},{"osc3_pan",0.5f},
        {"fm_2to1",0},{"fm_3to1",0},{"fm_3to2",0},{"pm_2to1",0},{"rm_2to1",0},{"am_2to1",0},
        {"filter_cutoff",12000},{"filter_reso",0},{"filter_drive",0},{"filter_env",0},
        {"amp_attack",0.005f},{"amp_decay",0.05f},{"amp_sustain",1.0f},{"amp_release",0.08f},
        {"lfo_amount",0},{"lfo2_amount",0},{"lfo3_amount",0},
        {"noise_level",0},{"sub_level",0},{"glide",0},
        {"delay_mix",0},{"reverb_mix",0},{"chorus_mix",0},{"phaser_mix",0},{"dist_mix",0},
        {"bassify",0},{"master_drive",0},{"comp_mix",0},{"magic_on",0},
        {"granular_mix",0},{"arp_on",0},{"seq_on",0}
    });

    // HIGHTECH
    add("HIGHTECH/Neon Razor", {
        {"osc1_level",0.95f},{"osc1_table",0.62f},{"osc1_warp",0.18f},{"osc1_fold",0.12f},{"osc1_drive",0.25f},
        {"osc1_unison",5},{"osc1_udet",16},{"osc1_uspread",0.9f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.4f},{"osc2_table",0.48f},{"osc2_semi",7},{"osc2_unison",1},{"osc2_octave",0},
        {"osc3_level",0},{"fm_2to1",0.28f},{"fm_3to1",0},{"fm_3to2",0},
        {"filter_cutoff",4200},{"filter_reso",0.48f},{"filter_env",0.72f},{"filter_drive",0.2f},
        {"amp_attack",0.002f},{"amp_decay",0.22f},{"amp_sustain",0.55f},{"amp_release",0.28f},
        {"lfo_amount",0.35f},{"lfo_rate",4.2f},{"lfo2_amount",0},{"lfo3_amount",0},
        {"noise_level",0},{"sub_level",0},
        {"delay_mix",0.28f},{"delay_mode",1},{"delay_fb",0.35f},{"reverb_mix",0.18f},{"chorus_mix",0.15f},
        {"dist_mix",0},{"master_drive",0.38f},{"comp_mix",0.5f},{"comp_threshold",-16}
    });
    add("HIGHTECH/Plasma Core", {
        {"osc1_level",0.9f},{"osc1_table",0.78f},{"osc1_fold",0.35f},{"osc1_warp",0.1f},{"osc1_drive",0.2f},
        {"osc1_unison",4},{"osc1_udet",12},{"osc1_uspread",0.85f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.55f},{"osc2_table",0.2f},{"osc2_octave",-1},{"osc2_semi",0},{"osc2_unison",2},{"osc2_udet",8},
        {"osc3_level",0.15f},{"osc3_table",0.5f},{"osc3_octave",0},
        {"fm_2to1",0.55f},{"fm_3to1",0.15f},{"fm_3to2",0},
        {"filter_cutoff",2800},{"filter_reso",0.7f},{"filter_env",0.88f},{"filter_drive",0.25f},
        {"amp_attack",0.001f},{"amp_decay",0.18f},{"amp_sustain",0.4f},{"amp_release",0.2f},
        {"lfo_amount",0.5f},{"lfo_rate",0.4f},{"lfo_wave",7},
        {"delay_mix",0.25f},{"dist_mix",0.22f},{"dist_drive",0.35f},{"master_drive",0.42f},
        {"noise_level",0},{"sub_level",0.1f}
    });
    add("HIGHTECH/Tehran Night", {
        {"osc1_level",0.88f},{"osc1_table",0.55f},{"osc1_warp",0.3f},{"osc1_fold",0.08f},
        {"osc1_unison",6},{"osc1_udet",22},{"osc1_uspread",1.0f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.35f},{"osc2_table",0.7f},{"osc2_semi",5},{"osc2_unison",3},{"osc2_udet",10},
        {"osc3_level",0.25f},{"osc3_table",0.3f},{"osc3_octave",-1},{"osc3_semi",0},{"osc3_unison",1},
        {"fm_2to1",0.2f},{"fm_3to1",0},{"fm_3to2",0.15f},
        {"filter_cutoff",3500},{"filter_reso",0.55f},{"filter_env",0.65f},{"filter_drive",0.15f},
        {"amp_attack",0.004f},{"amp_decay",0.3f},{"amp_sustain",0.5f},{"amp_release",0.35f},
        {"lfo_amount",0.25f},{"lfo_rate",0.8f},
        {"chorus_mix",0.28f},{"delay_mix",0.3f},{"reverb_mix",0.25f},{"reverb_size",0.7f},{"master_drive",0.35f}
    });
    add("HIGHTECH/Crystal Grid", {
        {"osc1_level",0.9f},{"osc1_table",0.4f},{"osc1_fold",0.15f},{"osc1_warp",0.05f},{"osc1_drive",0.1f},
        {"osc1_unison",3},{"osc1_udet",8},{"osc1_uspread",0.7f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0},{"osc3_level",0},{"fm_2to1",0},
        {"filter_cutoff",6500},{"filter_reso",0.25f},{"filter_env",0.4f},{"filter_drive",0},
        {"amp_attack",0.001f},{"amp_decay",0.35f},{"amp_sustain",0.0f},{"amp_release",0.4f},
        {"lfo_amount",0},{"noise_level",0},{"sub_level",0},
        {"reverb_mix",0.45f},{"reverb_size",0.85f},{"delay_mix",0.35f},{"delay_mode",1},{"chorus_mix",0.2f}
    });
    add("HIGHTECH/Wireframe Lead", {
        {"osc1_level",1.0f},{"osc1_table",0.88f},{"osc1_drive",0.4f},{"osc1_fold",0.2f},{"osc1_warp",0.15f},
        {"osc1_unison",4},{"osc1_udet",14},{"osc1_uspread",0.8f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.2f},{"osc2_table",0.6f},{"osc2_semi",12},{"osc2_unison",1},
        {"osc3_level",0},{"fm_2to1",0.15f},
        {"filter_cutoff",5200},{"filter_reso",0.4f},{"filter_env",0.6f},{"filter_drive",0.2f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.65f},{"amp_release",0.3f},
        {"lfo_amount",0.2f},{"lfo_rate",5.0f},
        {"phaser_mix",0.2f},{"delay_mix",0.22f},{"master_drive",0.4f}
    });

    // PSY / FULLON / DARK
    add("PSY/Forest Morning", {
        {"osc1_level",0.9f},{"osc1_table",0.22f},{"osc1_warp",0},{"osc1_fold",0},{"osc1_drive",0.1f},
        {"osc1_unison",5},{"osc1_udet",22},{"osc1_uspread",0.85f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.15f},{"osc2_table",0.15f},{"osc2_octave",-1},{"osc2_unison",1},
        {"osc3_level",0},{"fm_2to1",0.1f},
        {"filter_cutoff",900},{"filter_reso",0.72f},{"filter_env",0.85f},{"filter_drive",0.3f},
        {"amp_attack",0.004f},{"amp_decay",0.28f},{"amp_sustain",0.35f},{"amp_release",0.18f},
        {"lfo_amount",0.15f},{"lfo_rate",0.25f},
        {"delay_mix",0.22f},{"reverb_mix",0.18f},{"master_drive",0.28f},{"sub_level",0.2f},{"noise_level",0}
    });
    add("PSYTRANCE/Squelch Acid", {
        {"osc1_level",1.0f},{"osc1_table",0.7f},{"osc1_fold",0.05f},{"osc1_warp",0.2f},{"osc1_drive",0.15f},
        {"osc1_unison",1},{"osc1_udet",0},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0},{"osc3_level",0},{"fm_2to1",0},
        {"filter_cutoff",380},{"filter_reso",0.94f},{"filter_drive",0.6f},{"filter_env",1.0f},
        {"amp_attack",0.001f},{"amp_decay",0.16f},{"amp_sustain",0.08f},{"amp_release",0.1f},
        {"lfo_amount",0.4f},{"lfo_rate",6.0f},
        {"dist_mix",0.15f},{"master_drive",0.45f},{"noise_level",0},{"sub_level",0}
    });
    add("FULLON/Peak Hour", {
        {"osc1_level",0.95f},{"osc1_table",0.5f},{"osc1_drive",0.3f},{"osc1_fold",0.1f},{"osc1_warp",0.12f},
        {"osc1_unison",6},{"osc1_udet",28},{"osc1_uspread",1.0f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.3f},{"osc2_table",0.45f},{"osc2_semi",7},{"osc2_unison",2},{"osc2_udet",12},
        {"osc3_level",0.2f},{"osc3_octave",-1},{"osc3_table",0.1f},{"osc3_unison",1},
        {"fm_2to1",0.2f},{"fm_3to1",0.1f},
        {"filter_cutoff",2800},{"filter_reso",0.6f},{"filter_env",0.75f},{"filter_drive",0.2f},
        {"amp_attack",0.002f},{"amp_decay",0.18f},{"amp_sustain",0.45f},{"amp_release",0.15f},
        {"lfo_amount",0.3f},{"lfo_rate",8.0f},
        {"bassify",0.35f},{"delay_mix",0.2f},{"master_drive",0.48f},{"comp_mix",0.6f},{"comp_threshold",-14},
        {"sub_level",0.25f},{"noise_level",0}
    });
    add("DARKPSY/Void Crawl", {
        {"osc1_level",0.85f},{"osc1_table",0.85f},{"osc1_fold",0.4f},{"osc1_warp",0.25f},{"osc1_drive",0.3f},
        {"osc1_unison",3},{"osc1_udet",15},{"osc1_uspread",0.9f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.5f},{"osc2_table",0.9f},{"osc2_unison",2},{"osc2_udet",10},{"osc2_octave",0},
        {"osc3_level",0.2f},{"osc3_table",0.7f},{"osc3_octave",-1},
        {"fm_2to1",0.6f},{"fm_3to1",0.25f},
        {"filter_cutoff",600},{"filter_reso",0.85f},{"filter_env",0.95f},{"filter_drive",0.4f},
        {"amp_attack",0.01f},{"amp_decay",0.4f},{"amp_sustain",0.3f},{"amp_release",0.5f},
        {"lfo_amount",0.45f},{"lfo_rate",0.2f},
        {"reverb_mix",0.4f},{"reverb_size",0.9f},{"delay_mix",0.35f},{"dist_mix",0.3f},{"master_drive",0.4f},
        {"noise_level",0.1f},{"sub_level",0.15f}
    });

    // BASS / LEAD / PAD
    add("BASS/Sub Pressure", {
        {"osc1_level",1.0f},{"osc1_table",0.0f},{"osc1_octave",-2},{"osc1_semi",0},{"osc1_unison",1},
        {"osc1_warp",0},{"osc1_fold",0},{"osc1_drive",0.15f},
        {"osc2_level",0},{"osc3_level",0},{"fm_2to1",0},
        {"filter_cutoff",120},{"filter_reso",0.15f},{"filter_env",0.2f},{"filter_drive",0.1f},
        {"amp_attack",0.01f},{"amp_decay",0.3f},{"amp_sustain",0.85f},{"amp_release",0.25f},
        {"lfo_amount",0},{"bassify",0.6f},{"comp_mix",0.7f},{"comp_threshold",-12},{"master_drive",0.3f},
        {"sub_level",0.4f},{"noise_level",0},{"delay_mix",0},{"reverb_mix",0}
    });
    add("BASS/Reese Wide", {
        {"osc1_level",0.95f},{"osc1_table",0.3f},{"osc1_octave",-1},{"osc1_semi",0},
        {"osc1_unison",6},{"osc1_udet",30},{"osc1_uspread",1.0f},{"osc1_warp",0.1f},{"osc1_fold",0},
        {"osc2_level",0.4f},{"osc2_table",0.35f},{"osc2_semi",7},{"osc2_octave",-1},{"osc2_unison",3},{"osc2_udet",18},
        {"osc3_level",0},{"fm_2to1",0.1f},
        {"filter_cutoff",750},{"filter_reso",0.35f},{"filter_env",0.3f},
        {"amp_attack",0.04f},{"amp_decay",0.45f},{"amp_sustain",0.75f},{"amp_release",0.4f},
        {"lfo_amount",0.15f},{"lfo_rate",0.3f},
        {"chorus_mix",0.25f},{"master_drive",0.3f},{"sub_level",0.2f},{"noise_level",0}
    });
    add("LEAD/Hi-Tech Solo", {
        {"osc1_level",0.95f},{"osc1_table",0.6f},{"osc1_fold",0.2f},{"osc1_warp",0.1f},{"osc1_drive",0.2f},
        {"osc1_unison",3},{"osc1_udet",10},{"osc1_uspread",0.75f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.25f},{"osc2_table",0.55f},{"osc2_semi",12},{"osc2_unison",1},
        {"osc3_level",0},{"fm_2to1",0.2f},
        {"filter_cutoff",5500},{"filter_reso",0.4f},{"filter_env",0.5f},{"filter_drive",0.1f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.6f},{"amp_release",0.3f},
        {"lfo_amount",0.2f},{"lfo_rate",4.5f},
        {"delay_mix",0.3f},{"delay_mode",1},{"reverb_mix",0.2f},{"noise_level",0},{"sub_level",0}
    });
    add("LEAD/FM Splitter", {
        {"osc1_level",0.85f},{"osc1_table",0.5f},{"osc1_fold",0.1f},{"osc1_warp",0},{"osc1_drive",0.15f},
        {"osc1_unison",2},{"osc1_udet",6},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.7f},{"osc2_table",0.3f},{"osc2_unison",1},{"osc2_octave",0},{"osc2_semi",0},
        {"osc3_level",0.3f},{"osc3_table",0.4f},{"osc3_semi",7},{"osc3_unison",1},
        {"fm_2to1",0.75f},{"fm_3to1",0.2f},{"fm_3to2",0.15f},{"pm_2to1",0.1f},
        {"filter_cutoff",4000},{"filter_reso",0.35f},{"filter_env",0.55f},{"filter_drive",0.15f},
        {"amp_attack",0.002f},{"amp_decay",0.2f},{"amp_sustain",0.45f},{"amp_release",0.25f},
        {"lfo_amount",0.25f},{"lfo_rate",3.0f},
        {"delay_mix",0.25f},{"master_drive",0.35f},{"noise_level",0},{"sub_level",0}
    });
    add("PAD/Starfield", {
        {"osc1_level",0.7f},{"osc1_table",0.25f},{"osc1_unison",5},{"osc1_udet",20},{"osc1_uspread",0.95f},
        {"osc1_warp",0.05f},{"osc1_fold",0},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.4f},{"osc2_table",0.4f},{"osc2_semi",7},{"osc2_unison",4},{"osc2_udet",14},
        {"osc3_level",0.25f},{"osc3_table",0.2f},{"osc3_octave",-1},{"osc3_unison",2},{"osc3_udet",8},
        {"fm_2to1",0.05f},
        {"filter_cutoff",2500},{"filter_reso",0.2f},{"filter_env",0.15f},
        {"amp_attack",0.4f},{"amp_decay",0.6f},{"amp_sustain",0.85f},{"amp_release",1.2f},
        {"lfo_amount",0.2f},{"lfo_rate",0.15f},
        {"chorus_mix",0.4f},{"reverb_mix",0.5f},{"reverb_size",0.8f},{"noise_level",0},{"sub_level",0.05f}
    });
    add("PLUCK/Crystal Drop", {
        {"osc1_level",0.95f},{"osc1_table",0.7f},{"osc1_fold",0.15f},{"osc1_warp",0.08f},{"osc1_drive",0.1f},
        {"osc1_unison",2},{"osc1_udet",5},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0},{"osc3_level",0},{"fm_2to1",0},
        {"filter_cutoff",6000},{"filter_reso",0.3f},{"filter_env",0.5f},
        {"amp_attack",0.001f},{"amp_decay",0.28f},{"amp_sustain",0.0f},{"amp_release",0.25f},
        {"lfo_amount",0},{"reverb_mix",0.4f},{"delay_mix",0.2f},{"noise_level",0},{"sub_level",0}
    });

    // SALEK signature
    add("SALEK/Persian Glass", {
        {"osc1_level",0.8f},{"osc1_table",0.67f},{"osc1_warp",0.19f},{"osc1_fold",0.08f},{"osc1_drive",0.1f},
        {"osc1_unison",3},{"osc1_udet",10},{"osc1_uspread",0.7f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.63f},{"osc2_table",0.16f},{"osc2_unison",2},{"osc2_udet",8},{"osc2_semi",5},
        {"osc3_level",0.4f},{"osc3_table",0.66f},{"osc3_unison",1},{"osc3_octave",0},
        {"fm_2to1",0.15f},{"fm_3to1",0.1f},
        {"filter_cutoff",5100},{"filter_reso",0.0f},{"filter_env",0.4f},{"filter_drive",0},
        {"amp_attack",0.01f},{"amp_decay",0.07f},{"amp_sustain",0.32f},{"amp_release",0.04f},
        {"lfo_amount",0.1f},{"lfo_rate",2.0f},
        {"chorus_mix",0.25f},{"delay_mix",0.2f},{"reverb_mix",0.25f},{"noise_level",0},{"sub_level",0}
    });
    add("SALEK/Toronowla Night", {
        {"osc1_level",0.9f},{"osc1_table",0.55f},{"osc1_fold",0.45f},{"osc1_warp",0.2f},{"osc1_drive",0.25f},
        {"osc1_unison",4},{"osc1_udet",14},{"osc1_uspread",0.85f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.6f},{"osc2_table",0.4f},{"osc2_unison",2},{"osc2_udet",10},{"osc2_semi",7},
        {"osc3_level",0.25f},{"osc3_table",0.3f},{"osc3_octave",-1},
        {"fm_2to1",0.55f},{"fm_3to1",0.2f},
        {"filter_cutoff",3400},{"filter_reso",0.5f},{"filter_env",0.75f},{"filter_drive",0.2f},
        {"amp_attack",0.003f},{"amp_decay",0.25f},{"amp_sustain",0.5f},{"amp_release",0.35f},
        {"lfo_amount",0.3f},{"lfo_rate",1.5f},
        {"delay_mix",0.28f},{"reverb_mix",0.25f},{"master_drive",0.4f},{"noise_level",0},{"sub_level",0.1f}
    });
    add("SALEK/Milad Tower", {
        {"osc1_level",0.9f},{"osc1_table",0.58f},{"osc1_unison",4},{"osc1_udet",15},{"osc1_uspread",0.8f},
        {"osc1_warp",0.12f},{"osc1_fold",0.1f},{"osc1_drive",0.15f},{"osc1_octave",0},{"osc1_semi",0},
        {"osc2_level",0.4f},{"osc2_table",0.5f},{"osc2_semi",12},{"osc2_unison",2},{"osc2_udet",8},
        {"osc3_level",0.2f},{"osc3_table",0.35f},{"osc3_octave",0},{"osc3_semi",7},
        {"fm_2to1",0.25f},{"fm_3to1",0.1f},
        {"filter_cutoff",3800},{"filter_reso",0.42f},{"filter_env",0.6f},{"filter_drive",0.15f},
        {"amp_attack",0.005f},{"amp_decay",0.28f},{"amp_sustain",0.55f},{"amp_release",0.4f},
        {"lfo_amount",0.25f},{"lfo_rate",2.5f},
        {"delay_mix",0.32f},{"reverb_mix",0.35f},{"chorus_mix",0.2f},{"master_drive",0.36f}
    });
#include "PluginProcessorPresetsEngineered.inl"

    for (auto& p : salek::giti::makePresets()) add (p.name, std::move (p.values));
}
