cmake_minimum_required(VERSION 3.22)
project(GITI_BY_SALEK_HIGHTECH VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(GITI_BY_SALEK_HIGHTECH
    COMPANY_NAME "Salek"
    PLUGIN_MANUFACTURER_CODE Salk
    PLUGIN_CODE Sht8
    FORMATS VST3 Standalone
    PRODUCT_NAME "GITI BY SALEK HIGHTECH"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument Synth"
)

juce_generate_juce_header(GITI_BY_SALEK_HIGHTECH)

file(GLOB SALEK_ICON_ASSETS CONFIGURE_DEPENDS "Source/Assets/icons/*.png")

juce_add_binary_data(SalekBinaryAssets SOURCES
    "Source/Assets/bg_isatis.jpg"
    "Source/Assets/bg_salek.jpg"
    "Source/Assets/logo_giti.png"
    "Source/Assets/logo_giti_alt.png"
    "Source/Assets/icons_atlas.png"
    "Source/Assets/logo_salek_sheet.png"
    "Source/Assets/char_toronowla.png"
    "Source/Assets/char_neon_street.png"
    "Source/Assets/char_pink_dead.png"
    "Source/Assets/char_apron.png"
    "Source/Assets/char_catgirl.png"
    "Source/Assets/char_cyber_white.png"
    "Source/Assets/char_foxgirl.png"
    "Source/Assets/char_gun.png"
    "Source/Assets/char_purple_latex.png"
    "Source/Assets/char_white_suit.png"
    ${SALEK_ICON_ASSETS}
)

target_sources(GITI_BY_SALEK_HIGHTECH PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/AssetsData.h
    Source/Synth/SynthEngine.cpp
    Source/Synth/SynthEngine.h
    Source/Synth/SynthVoice.cpp
    Source/Synth/SynthVoice.h
    Source/Synth/SynthSound.cpp
    Source/Synth/SynthSound.h
    Source/Oscillators/WavetableOscillator.cpp
    Source/Oscillators/WavetableOscillator.h
    Source/Oscillators/GranularOscillator.h
    Source/Wavetables/WavetableData.cpp
    Source/Wavetables/WavetableData.h
    Source/Filters/StateVariableFilter.h
    Source/Modulation/LFO.h
    Source/Modulation/ModMatrix.h
    Source/FX/SimpleDelay.h
    Source/FX/SimpleChorus.h
    Source/FX/SimpleReverb.h
    Source/Sequencer/Arpeggiator.h
    Source/Sequencer/StepSequencer.h
    Source/DSP/SmoothedValue.h
    Source/SHAE/SHAECore.h
    Source/UI/ModMatrixPanel.h
)

target_include_directories(GITI_BY_SALEK_HIGHTECH PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_compile_definitions(GITI_BY_SALEK_HIGHTECH PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GITI_BY_SALEK_HIGHTECH
    PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_opengl SalekBinaryAssets
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags
)

if (MSVC)
    target_compile_options(GITI_BY_SALEK_HIGHTECH PRIVATE /W3 /MP /bigobj /utf-8)
endif()

# Copy Assets next to plugin so character PNGs load at runtime
add_custom_command(TARGET GITI_BY_SALEK_HIGHTECH POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Assets"
        "$<TARGET_FILE_DIR:GITI_BY_SALEK_HIGHTECH>/Assets"
    COMMENT "Copy Assets beside plugin binary"
)


# Lightweight SHAE verification target. It exercises only the realtime-safe core;
# it does not alter the plugin/standalone product target.
juce_add_console_app(GITI_SHAE_SELFTEST PRODUCT_NAME "GITI SHAE Self Test")
target_sources(GITI_SHAE_SELFTEST PRIVATE Source/Tests/SHAESelfTest.cpp)
target_link_libraries(GITI_SHAE_SELFTEST PRIVATE
    juce::juce_audio_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
