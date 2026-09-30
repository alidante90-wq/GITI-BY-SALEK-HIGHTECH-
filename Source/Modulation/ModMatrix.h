#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

class ModMatrix
{
public:
    static constexpr int MaxRoutes = 64;

    enum class Source : int
    {
        LFO1 = 0,
        LFO2,
        LFO3,
        Env1,
        Velocity,
        ModWheel,
        Macro1,
        Macro2,
        Macro3,
        Macro4,
        Macro5,
        Macro6,
        Macro7,
        Macro8,
        Random,
        NumSources
    };

    enum class Dest : int
    {
        FilterCutoff = 0,
        FilterReso,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        Osc1Table,
        Osc2Table,
        Osc3Table,
        Osc1Warp,
        Osc2Warp,
        Osc3Warp,
        Osc1Fold,
        Osc2Fold,
        Osc3Fold,
        Fm2to1,
        Fm3to1,
        Pitch,
        Amp,
        Osc1Pan,
        Osc2Pan,
        Osc3Pan,
        Osc1Drive,
        Osc2Drive,
        Osc3Drive,
        DelayMix,
        ReverbMix,
        DistDrive,
        ChorusMix,
        FilterEnv,
        PhaserMix,
        Bassify,
        MagicX,
        MagicY,
        NumDests
    };

    static const char* sourceName (Source s)
    {
        static const char* n[] = {
            "LFO1","LFO2","LFO3","ENV","VEL","MW",
            "MAC1","MAC2","MAC3","MAC4","MAC5","MAC6","MAC7","MAC8","RND"
        };
        int i = (int) s;
        return (i >= 0 && i < (int) Source::NumSources) ? n[i] : "?";
    }

    static const char* destName (Dest d)
    {
        static const char* n[] = {
            "CUT","RESO","O1LVL","O2LVL","O3LVL",
            "O1TBL","O2TBL","O3TBL","O1WRP","O2WRP","O3WRP",
            "O1FLD","O2FLD","O3FLD","FM21","FM31","PITCH","AMP",
            "O1PAN","O2PAN","O3PAN","O1DRV","O2DRV","O3DRV",
            "DLYMX","REVMX","DIST","CHOR","FENV","PHSR","BASS","MAGX","MAGY"
        };
        int i = (int) d;
        return (i >= 0 && i < (int) Dest::NumDests) ? n[i] : "?";
    }

    struct Route
    {
        Source source = Source::LFO1;
        Dest   dest   = Dest::FilterCutoff;
        float  amount = 0.0f;
        bool   active = false;
    };

    void clear() noexcept
    {
        for (auto& r : routes) r.active = false;
    }

    int addRoute (Source src, Dest dst, float amount) noexcept
    {
        for (int i = 0; i < MaxRoutes; ++i)
        {
            if (routes[static_cast<size_t>(i)].active
                && routes[static_cast<size_t>(i)].source == src
                && routes[static_cast<size_t>(i)].dest == dst)
            {
                routes[static_cast<size_t>(i)].amount = juce::jlimit (-1.0f, 1.0f, amount);
                return i;
            }
        }
        for (int i = 0; i < MaxRoutes; ++i)
        {
            if (! routes[static_cast<size_t>(i)].active)
            {
                routes[static_cast<size_t>(i)] = { src, dst, juce::jlimit (-1.0f, 1.0f, amount), true };
                return i;
            }
        }
        return -1;
    }

    void removeRoute (int slot) noexcept
    {
        if (slot >= 0 && slot < MaxRoutes)
            routes[static_cast<size_t>(slot)].active = false;
    }

    void removeRoute (Source src, Dest dst) noexcept
    {
        for (int i = 0; i < MaxRoutes; ++i)
            if (routes[static_cast<size_t>(i)].active
                && routes[static_cast<size_t>(i)].source == src
                && routes[static_cast<size_t>(i)].dest == dst)
                routes[static_cast<size_t>(i)].active = false;
    }

    void setSourceValue (Source s, float v) noexcept
    {
        sourceValues[static_cast<size_t>(s)] = v;
    }

    float getSourceValue (Source s) const noexcept
    {
        return sourceValues[static_cast<size_t>(s)];
    }

    float getModulation (Dest d) const noexcept
    {
        float sum = 0.0f;
        for (const auto& r : routes)
        {
            if (r.active && r.dest == d)
                sum += sourceValues[static_cast<size_t>(r.source)] * r.amount;
        }
        return sum;
    }

    const std::array<Route, MaxRoutes>& getRoutes() const noexcept { return routes; }
    Route& getRoute (int i) noexcept { return routes[static_cast<size_t>(juce::jlimit(0, MaxRoutes-1, i))]; }

    /** Serialize active routes into <MOD_MATRIX> for project/preset state. */
    std::unique_ptr<juce::XmlElement> toXml() const
    {
        auto el = std::make_unique<juce::XmlElement> ("MOD_MATRIX");
        el->setAttribute ("version", 1);
        for (int i = 0; i < MaxRoutes; ++i)
        {
            const auto& r = routes[static_cast<size_t> (i)];
            if (! r.active) continue;
            auto* row = el->createNewChildElement ("ROUTE");
            row->setAttribute ("src", (int) r.source);
            row->setAttribute ("dst", (int) r.dest);
            row->setAttribute ("amt", (double) r.amount);
        }
        return el;
    }

    /** Replace routes from <MOD_MATRIX> (or no-op if null/wrong tag). */
    void fromXml (const juce::XmlElement* el)
    {
        clear();
        if (el == nullptr || ! el->hasTagName ("MOD_MATRIX")) return;

        for (auto* row = el->getFirstChildElement(); row != nullptr; row = row->getNextElement())
        {
            if (! row->hasTagName ("ROUTE")) continue;
            const int src = row->getIntAttribute ("src", -1);
            const int dst = row->getIntAttribute ("dst", -1);
            const float amt = (float) row->getDoubleAttribute ("amt", 0.5);
            if (src < 0 || src >= (int) Source::NumSources) continue;
            if (dst < 0 || dst >= (int) Dest::NumDests) continue;
            addRoute ((Source) src, (Dest) dst, amt);
        }
    }

private:
    std::array<Route, MaxRoutes> routes {};
    std::array<float, static_cast<size_t>(Source::NumSources)> sourceValues {};
};

} // namespace salek
