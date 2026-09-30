#!/usr/bin/env python3
"""Patch getStateInformation / setStateInformation to persist ModMatrix routes."""
from pathlib import Path
import sys

path = Path("Source/PluginProcessor_full_p2.inl")
if not path.exists():
    print("ERROR: run from repo root"); sys.exit(1)

text = path.read_text(encoding="utf-8", errors="replace")
if "modMatrix.toXml" in text and "modMatrix.fromXml" in text:
    print("Already patched."); sys.exit(0)

old_get = '''void SalekHightechAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement root ("SALEK_STATE");
    root.setAttribute ("version", 1);
    root.setAttribute ("program", currentProgram);
    if (auto ap = apvts.copyState().createXml())
        root.addChildElement (new juce::XmlElement (*ap));
    copyXmlToBinary (root, destData);
}'''

new_get = '''void SalekHightechAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
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
}'''

old_set = '''void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName ("SALEK_STATE"))
        {
            if (auto* ap = xml->getChildByName (apvts.state.getType()))
                apvts.replaceState (juce::ValueTree::fromXml (*ap));
            const int prog = xml->getIntAttribute ("program", -1);
            if (prog >= 0 && prog < (int) factoryPresets.size())
                currentProgram = prog;
        }
        else if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
    }
}'''

new_set = '''void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
}'''

if old_get not in text:
    print("ERROR: getStateInformation block not found"); sys.exit(1)
if old_set not in text:
    print("ERROR: setStateInformation block not found"); sys.exit(1)

text = text.replace(old_get, new_get).replace(old_set, new_set)
path.write_text(text, encoding="utf-8")
print("OK patched", path)
print('git add Source/Modulation/ModMatrix.h Source/PluginProcessor_full_p2.inl')
print('git commit -m "SHAE: persist ModMatrix routes in plugin state"')
print('git push origin giti-by-salek-hightech')
