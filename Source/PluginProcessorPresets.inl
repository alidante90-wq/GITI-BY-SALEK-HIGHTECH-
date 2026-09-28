#include "GitiSonicDNA.h"

void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){
        factoryPresets.push_back({n, std::move(v)});
    };

    // Core + GITI 001-050 live in Engineered (self-contained)
#include "PluginProcessorPresetsEngineered.inl"
}
