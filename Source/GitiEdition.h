#pragma once
#include <JuceHeader.h>
namespace giti {
struct Edition {
    int id; const char* code; const char* name; const char* element; const char* archetype; int bpm; const char* palette;
};
inline constexpr Edition genesisEditions[] = {
    {1,"GITI-001","The First Breath","Breath","BASS",174,"CYAN"},
    {2,"GITI-002","Santur Rain","Strings","FM",175,"VIOLET"},
    {3,"GITI-003","Cradle Song","Voice","LEAD",176,"MAGENTA"},
    {4,"GITI-004","Daf Heart","Pulse","SCREECH",177,"RED"},
    {5,"GITI-005","Spring Bell","Earth","ACID",178,"EMERALD"},
    {6,"GITI-006","Ney Dawn","Breath","ATMOS",179,"CYAN"},
    {7,"GITI-007","Tar Oath","Strings","DRONE",180,"VIOLET"},
    {8,"GITI-008","Throat Star","Voice","RITUAL",181,"MAGENTA"},
    {9,"GITI-009","Tombak Rain","Pulse","ALIEN",182,"RED"},
    {10,"GITI-010","Cave Choir","Earth","MACHINE",183,"EMERALD"},
    {11,"GITI-011","Glass Whisper","Breath","BASS",184,"CYAN"},
    {12,"GITI-012","Setar Thread","Strings","FM",185,"VIOLET"},
    {13,"GITI-013","Mother Tongue","Voice","LEAD",186,"MAGENTA"},
    {14,"GITI-014","Walk Not March","Pulse","SCREECH",187,"RED"},
    {15,"GITI-015","River Glass","Earth","ACID",188,"EMERALD"},
    {16,"GITI-016","Kite Wind","Breath","ATMOS",189,"CYAN"},
    {17,"GITI-017","Kamancheh Tide","Strings","DRONE",190,"VIOLET"},
    {18,"GITI-018","Choir of Salt","Voice","RITUAL",174,"MAGENTA"},
    {19,"GITI-019","Zarb Ember","Pulse","ALIEN",175,"RED"},
    {20,"GITI-020","Rain Loom","Earth","MACHINE",176,"EMERALD"},
    {21,"GITI-021","Frost Reed","Breath","BASS",177,"CYAN"},
    {22,"GITI-022","Oud Ember","Strings","FM",178,"VIOLET"},
    {23,"GITI-023","Ghazal Bloom","Voice","LEAD",179,"MAGENTA"},
    {24,"GITI-024","Anvil Psalm","Pulse","SCREECH",180,"RED"},
    {25,"GITI-025","Forest Throat","Earth","ACID",181,"EMERALD"},
    {26,"GITI-026","Monsoon Sigh","Breath","ATMOS",182,"CYAN"},
    {27,"GITI-027","Barbat Lantern","Strings","DRONE",183,"VIOLET"},
    {28,"GITI-028","Call at Dusk","Voice","RITUAL",184,"MAGENTA"},
    {29,"GITI-029","Stone Drum","Pulse","ALIEN",185,"RED"},
    {30,"GITI-030","Salt Desert Hum","Earth","MACHINE",186,"EMERALD"},
    {31,"GITI-031","Cloud Choir","Breath","BASS",187,"CYAN"},
    {32,"GITI-032","Qanun Loom","Strings","FM",188,"VIOLET"},
    {33,"GITI-033","Laughter Loom","Voice","LEAD",189,"MAGENTA"},
    {34,"GITI-034","Heartbeat Chorus","Pulse","SCREECH",190,"RED"},
    {35,"GITI-035","Tide Ledger","Earth","ACID",174,"EMERALD"},
    {36,"GITI-036","Silent Horn","Breath","ATMOS",175,"CYAN"},
    {37,"GITI-037","Chang Cathedral","Strings","DRONE",176,"VIOLET"},
    {38,"GITI-038","Hum of Forgiveness","Voice","RITUAL",177,"MAGENTA"},
    {39,"GITI-039","Sub Vow","Pulse","ALIEN",178,"RED"},
    {40,"GITI-040","Seed Chime","Earth","MACHINE",179,"EMERALD"},
    {41,"GITI-041","Sky Lung","Breath","BASS",180,"CYAN"},
    {42,"GITI-042","Rubab Storm","Strings","FM",181,"VIOLET"},
    {43,"GITI-043","Ten Thousand Voices","Voice","LEAD",182,"MAGENTA"},
    {44,"GITI-044","Thunder Lullaby","Pulse","SCREECH",183,"RED"},
    {45,"GITI-045","Root Drone","Earth","ACID",184,"EMERALD"},
    {46,"GITI-046","Last Horizon","Breath","ATMOS",185,"CYAN"},
    {47,"GITI-047","Nine Thousand Strings","Strings","DRONE",186,"VIOLET"},
    {48,"GITI-048","The Sung Name","Voice","RITUAL",187,"MAGENTA"},
    {49,"GITI-049","Pulse of Succession","Pulse","ALIEN",188,"RED"},
    {50,"GITI-050","The Last Voice","Earth","MACHINE",189,"EMERALD"}
};
inline constexpr int kGenesisEditionCount = 50;
inline const Edition& currentEdition() noexcept {
#if defined(GITI_EDITION_ID)
    constexpr int requested = GITI_EDITION_ID;
#else
    constexpr int requested = 1;
#endif
    constexpr int index = (requested < 1 || requested > kGenesisEditionCount) ? 0 : requested - 1;
    return genesisEditions[index];
}
}
