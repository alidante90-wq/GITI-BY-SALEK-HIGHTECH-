#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"
#include <map>
#include <cmath>
// Full DNA morphing lives in main SALEK repo; this header is enough for compile when only presets are needed.
namespace giti { namespace dna {
struct Profile { float spectralFocus=0.5f, harmonicity=0.5f, modulation=0.5f, transient=0.5f, spatial=0.5f, aggression=0.5f, lowWeight=0.5f, highWeight=0.5f, instability=0.5f; };
inline void applyToPreset(const Edition&, const juce::String&, std::map<juce::String,float>&) {}
}}
