#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"
#include "../Oscillators/WavetableOscillator.h"
#include "../Wavetables/WavetableData.h"
#include "../Filters/StateVariableFilter.h"
#include "../Modulation/LFO.h"
#include "../DSP/SmoothedValue.h"
#include "PersianScale.h"

namespace salek {

class SynthVoice : public juce::SynthesiserVoice {
public:
    SynthVoice() {
        osc1.setWavetable(&sharedWavetable); osc2.setWavetable(&sharedWavetable); osc3.setWavetable(&sharedWavetable);
        osc1.setLevel(0.7f); osc2.setLevel(0.5f); osc3.setLevel(0.4f); osc3Octave = -1;
        for (int u = 0; u < maxUnison; ++u) {
            uniOsc1[u].setWavetable(&sharedWavetable); uniOsc1[u].setLevel(0.7f);
            uniOsc2[u].setWavetable(&sharedWavetable); uniOsc2[u].setLevel(0.5f);
            uniOsc3[u].setWavetable(&sharedWavetable); uniOsc3[u].setLevel(0.4f);
        }
    }
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*>(s) != nullptr; }
    void prepareToPlay(double sr, int) {
        currentSampleRate = sr; osc1.prepare(sr); osc2.prepare(sr); osc3.prepare(sr);
        for (int u = 0; u < maxUnison; ++u) { uniOsc1[u].prepare(sr); uniOsc2[u].prepare(sr); uniOsc3[u].prepare(sr); }
        filterL.prepare(sr); filterR.prepare(sr); lfo.prepare(sr); adsr.setSampleRate(sr); adsr.setParameters(adsrParams);
        cutoffSmoother.reset(baseCutoff); cutoffSmoother.setTimeMs(5.f, sr);
    }
    void startNote(int note, float vel, juce::SynthesiserSound*, int) override {
        const bool wasActive = isVoiceActive() && isNoteOn;
        currentMidiNote = note; currentVelocity = vel; isNoteOn = true;
        glideActive = wasActive && glideAmt > 0.0001f;
        if (! glideActive)
        {
            osc1.reset(); osc2.reset(); osc3.reset();
            for (int u = 0; u < maxUnison; ++u) { uniOsc1[u].reset(); uniOsc2[u].reset(); uniOsc3[u].reset(); }
        }
        // Random phase on note-on so repeated notes don't phase-lock ("دیییو دیییو")
        auto rnd = [] (float amt) {
            if (amt < 1e-4f) return 0.f;
            return amt * (juce::Random::getSystemRandom().nextFloat());
        };
        osc1.setPhaseOffset (juce::jlimit (0.f, 1.f, osc1Phase + rnd (osc1Rand)));
        osc2.setPhaseOffset (juce::jlimit (0.f, 1.f, osc2Phase + rnd (osc2Rand)));
        osc3.setPhaseOffset (juce::jlimit (0.f, 1.f, osc3Phase + rnd (osc3Rand)));
        for (int u = 0; u < maxUnison; ++u) {
            uniOsc1[u].setPhaseOffset (juce::jlimit (0.f, 1.f, osc1Phase + rnd (osc1Rand)));
            uniOsc2[u].setPhaseOffset (juce::jlimit (0.f, 1.f, osc2Phase + rnd (osc2Rand)));
            uniOsc3[u].setPhaseOffset (juce::jlimit (0.f, 1.f, osc3Phase + rnd (osc3Rand)));
        }
        updateFrequencies(); adsr.noteOn();
    }
    void stopNote(float, bool allowTailOff) override {
        if (allowTailOff) adsr.noteOff();
        else { clearCurrentNote(); adsr.reset(); isNoteOn = false; }
    }
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    static const Wavetable& getSharedWavetable() noexcept { return sharedWavetable; }
    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override;

    void setOsc1TablePos(float v){osc1.setTablePosition(v); for(int u=0;u<maxUnison;++u)uniOsc1[u].setTablePosition(v);}
    void setOsc2TablePos(float v){osc2.setTablePosition(v); for(int u=0;u<maxUnison;++u)uniOsc2[u].setTablePosition(v);} void setOsc3TablePos(float v){osc3.setTablePosition(v); for(int u=0;u<maxUnison;++u)uniOsc3[u].setTablePosition(v);}
    void setOsc1Level(float v){const bool was=osc1Level>1e-5f; osc1Level=v; osc1.setLevel(v); for(int u=0;u<maxUnison;++u)uniOsc1[u].setLevel(v); if(was!=(v>1e-5f)) uniFreqDirty=true;}
    void setOsc2Level(float v){const bool was=osc2Level>1e-5f; osc2Level=v; osc2.setLevel(v); for(int u=0;u<maxUnison;++u)uniOsc2[u].setLevel(v); if(was!=(v>1e-5f)) uniFreqDirty=true;} void setOsc3Level(float v){const bool was=osc3Level>1e-5f; osc3Level=v; osc3.setLevel(v); for(int u=0;u<maxUnison;++u)uniOsc3[u].setLevel(v); if(was!=(v>1e-5f)) uniFreqDirty=true;}
    void setOsc1Octave(int v){osc1Octave=v; updateFrequencies();} void setOsc2Octave(int v){osc2Octave=v; updateFrequencies();} void setOsc3Octave(int v){osc3Octave=v; updateFrequencies();}
    void setOsc1Semi(int v){osc1Semi=v; updateFrequencies();} void setOsc2Semi(int v){osc2Semi=v; updateFrequencies();} void setOsc3Semi(int v){osc3Semi=v; updateFrequencies();}
    void setOsc1CoarsePitch(int v){osc1Coarse=juce::jlimit(-48,48,v); updateFrequencies();} void setOsc2CoarsePitch(int v){osc2Coarse=juce::jlimit(-48,48,v); updateFrequencies();} void setOsc3CoarsePitch(int v){osc3Coarse=juce::jlimit(-48,48,v); updateFrequencies();}
    void setOsc1Fine(float v){osc1Fine=v; updateFrequencies();} void setOsc2Fine(float v){osc2Fine=v; updateFrequencies();} void setOsc3Fine(float v){osc3Fine=v; updateFrequencies();}
    void setOsc1Detune(float v){osc1.setDetuneCents(v);} void setOsc2Detune(float v){osc2.setDetuneCents(v);} void setOsc3Detune(float v){osc3.setDetuneCents(v);}
    void setOsc1Warp(float v){osc1.setWarp(v); for(int u=0;u<maxUnison;++u)uniOsc1[u].setWarp(v);}
    void setOsc2Warp(float v){osc2.setWarp(v); for(int u=0;u<maxUnison;++u)uniOsc2[u].setWarp(v);} void setOsc3Warp(float v){osc3.setWarp(v); for(int u=0;u<maxUnison;++u)uniOsc3[u].setWarp(v);}
    void setOsc1Fold(float v){osc1.setFold(v); for(int u=0;u<maxUnison;++u)uniOsc1[u].setFold(v);}
    void setOsc2Fold(float v){osc2.setFold(v); for(int u=0;u<maxUnison;++u)uniOsc2[u].setFold(v);} void setOsc3Fold(float v){osc3.setFold(v); for(int u=0;u<maxUnison;++u)uniOsc3[u].setFold(v);}
    void setOsc1Drive(float v){osc1.setDrive(v); for(int u=0;u<maxUnison;++u)uniOsc1[u].setDrive(v);}
    void setOsc2Drive(float v){osc2.setDrive(v); for(int u=0;u<maxUnison;++u)uniOsc2[u].setDrive(v);} void setOsc3Drive(float v){osc3.setDrive(v); for(int u=0;u<maxUnison;++u)uniOsc3[u].setDrive(v);}
    void setOsc1Phase(float v){osc1Phase=juce::jlimit(0.f,1.f,v); osc1.setPhaseOffset(osc1Phase);}
    void setOsc2Phase(float v){osc2Phase=juce::jlimit(0.f,1.f,v); osc2.setPhaseOffset(osc2Phase);}
    void setOsc3Phase(float v){osc3Phase=juce::jlimit(0.f,1.f,v); osc3.setPhaseOffset(osc3Phase);}
    void setOsc1Rand(float v){osc1Rand=juce::jlimit(0.f,1.f,v);}
    void setOsc2Rand(float v){osc2Rand=juce::jlimit(0.f,1.f,v);}
    void setOsc3Rand(float v){osc3Rand=juce::jlimit(0.f,1.f,v);}
    void setOsc1Pan(float v){osc1Pan=juce::jlimit(0.f,1.f,v);}
    void setOsc2Pan(float v){osc2Pan=juce::jlimit(0.f,1.f,v);}
    void setOsc3Pan(float v){osc3Pan=juce::jlimit(0.f,1.f,v);}
    void setUnison(int v){unisonVoices=juce::jlimit(1,maxUnison,v); uniVoices1=uniVoices2=uniVoices3=unisonVoices;}
    void setUnisonDetune(float c){unisonDetune=juce::jlimit(0.f,100.f,c); uniDet1=uniDet2=uniDet3=unisonDetune;}
    void setUnisonSpread(float s){unisonSpread=juce::jlimit(0.f,1.f,s); uniSpr1=uniSpr2=uniSpr3=unisonSpread;}
    void setOsc1Unison(int v,float d,float s){uniVoices1=juce::jlimit(1,maxUnison,v); uniDet1=d; uniSpr1=s; uniFreqDirty=true;}
    void setOsc2Unison(int v,float d,float s){uniVoices2=juce::jlimit(1,maxUnison,v); uniDet2=d; uniSpr2=s; uniFreqDirty=true;}
    void setOsc3Unison(int v,float d,float s){uniVoices3=juce::jlimit(1,maxUnison,v); uniDet3=d; uniSpr3=s; uniFreqDirty=true;}
    void setFm2to1(float v){fm2to1=v;} void setFm3to1(float v){fm3to1=v;} void setFm3to2(float v){fm3to2=v;}
    void setPm2to1(float v){pm2to1=v;} void setPm3to1(float v){pm3to1=v;} void setAm2to1(float v){am2to1=v;} void setRm2to1(float v){rm2to1=v;}
    void setFilterCutoff(float) {} void setFilterBaseCutoff(float hz){baseCutoff=hz; filterL.setCutoff(hz); filterR.setCutoff(hz);}
    void setFilterResonance(float r){filterL.setResonance(r); filterR.setResonance(r);} void setFilterDrive(float d){filterL.setDrive(d); filterR.setDrive(d);}
    void setFilterMode(int m){ filterL.setModeIndex (m); filterR.setModeIndex (m); }
    void setFilterEnvAmt(float a){filterEnvAmt=a;}
    void setNoiseLevel(float v){noiseLevel=juce::jlimit(0.f,1.f,v);}
    void setSubLevel(float v){subLevel=juce::jlimit(0.f,1.f,v);}
    void setFilterRoute(int r){filterRoute=juce::jlimit(0,6,r);}
    void setGlide(float v){ glideAmt=juce::jlimit(0.f,1.f,v); updateGlideCoefficient(); }
    void setAmpAttack(float s){adsrParams.attack=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setAmpDecay(float s){adsrParams.decay=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setAmpSustain(float s){adsrParams.sustain=juce::jlimit(0.f,1.f,s); adsr.setParameters(adsrParams);}
    void setAmpRelease(float s){adsrParams.release=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setLfoRate(float hz){lfo.setRate(hz);} void setLfoAmount(float a){lfo.setAmount(a);}
    void setLfoWave(int w){ using W=LFO::Wave; lfo.setWave(w==1?W::Triangle:w==2?W::Saw:w==3?W::Square:w==4?W::SAndH:W::Sine); }
    void labRegenerate(){sharedWavetable.generateDefaultTables();}
    void labProcessFrames(float fold, float drive){ for(int i=0;i<sharedWavetable.getNumFrames();++i) sharedWavetable.processFrame(i,fold,drive); }
    void labMorph(float t){ if(sharedWavetable.getNumFrames()>=4) sharedWavetable.morphFrames(0,3,1,t); }
    void setScaleMode(int m){ scaleMode = juce::jlimit(0, PersianScale::NumModes-1, m); updateFrequencies(); }
    void setKoronCents(float c){ koronCents = juce::jlimit(-50.f, 50.f, c); updateFrequencies(); }
    void setScaleRoot(int r){ scaleRoot = r; updateFrequencies(); }

private:
    void updateFrequencies();
    void updateGlideFrequencies() noexcept;
    void updateGlideCoefficient() noexcept;
    void refreshUnisonTuning() noexcept;
    float noteToHz(int note, int oct, int semi, float fine) const {
        return PersianScale::noteToHz (note, oct, semi, fine, scaleMode, koronCents, scaleRoot);
    }
    static constexpr int maxUnison = 7;
    static constexpr int maxUniBudget = 12; // soft cap across OSC1+2+3
    WavetableOscillator osc1, osc2, osc3;
    WavetableOscillator uniOsc1[maxUnison], uniOsc2[maxUnison], uniOsc3[maxUnison];
    int unisonVoices = 1;
    float unisonDetune = 12.f, unisonSpread = 0.7f;
    int uniVoices1=1, uniVoices2=1, uniVoices3=1;
    float uniDet1=12.f, uniDet2=12.f, uniDet3=12.f;
    float uniSpr1=0.7f, uniSpr2=0.7f, uniSpr3=0.7f;
    inline static Wavetable sharedWavetable;
    StateVariableFilter filterL, filterR;
    LFO lfo;
    SmoothedValue cutoffSmoother;
    double currentSampleRate = 44100;
    float currentVelocity = 0;
    int currentMidiNote = 60;
    int scaleMode = 0;
    float koronCents = 0.f;
    int scaleRoot = 60;
    int osc1Octave=0, osc2Octave=0, osc3Octave=-1;
    int osc1Semi=0, osc2Semi=0, osc3Semi=0;
    int osc1Coarse=0, osc2Coarse=0, osc3Coarse=0;
    float osc1Fine=0, osc2Fine=0, osc3Fine=0;
    float osc1Phase=0, osc2Phase=0, osc3Phase=0;
    float osc1Rand=0, osc2Rand=0, osc3Rand=0;
    float osc1Pan=0.5f, osc2Pan=0.5f, osc3Pan=0.5f;
    float fm2to1=0, fm3to1=0, fm3to2=0, pm2to1=0, pm3to1=0, am2to1=0, rm2to1=0;
    float filterEnvAmt=0.5f, baseCutoff=8000;
    float osc1Level=0.7f, osc2Level=0.5f, osc3Level=0.4f;
    bool uniFreqDirty = true;
    float uniDetTbl1[maxUnison]{}, uniDetTbl2[maxUnison]{}, uniDetTbl3[maxUnison]{};
    float uniPanTbl1[maxUnison]{}, uniPanTbl2[maxUnison]{}, uniPanTbl3[maxUnison]{};
    int nUniEff1=1, nUniEff2=1, nUniEff3=1;
    float noiseLevel=0.f, subLevel=0.f, glideAmt=0.f;
    float glideHz1=440.f, glideHz2=440.f, glideHz3=440.f;
    float glideCoeff = 1.0f;
    bool glideActive=false;
    int filterRoute=0; // 0=All,1=O1,2=O2,3=O3,4=1+2,5=1+3,6=2+3
    juce::Random noiseRng;
    float subPhase=0.f;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams{0.01f, 0.1f, 0.8f, 0.2f};
    bool isNoteOn = false;
};
}
