cmake_minimum_required(VERSION 3.22)
project(AncientSynth VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "" FORCE)

# JUCE_PATH can be set by the setup script to a pre-cloned local copy.
# This avoids git safe.directory issues when building on external drives as sudo.
if(NOT DEFINED JUCE_PATH)
  set(JUCE_PATH "${CMAKE_SOURCE_DIR}/juce" CACHE PATH "Path to JUCE")
endif()

if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
  message(FATAL_ERROR
    "JUCE not found at ${JUCE_PATH}\n"
    "Run the setup script, or clone JUCE manually:\n"
    "  git clone https://github.com/juce-framework/JUCE.git --branch 8.0.7 ${JUCE_PATH}")
endif()

add_subdirectory("${JUCE_PATH}" juce_build)

juce_add_plugin(AncientSynth
    COMPANY_NAME            "Tully EDM Vibe"
    COMPANY_COPYRIGHT       "(c)(p) 2026 Tully EDM Vibe"
    COMPANY_EMAIL           "info@tullyedmvibe.com"
    BUNDLE_ID               com.TullyEDMVibe.AncientSynth
    PLUGIN_MANUFACTURER_CODE Tull
    PLUGIN_CODE             AS30
    FORMATS                 AU VST3 Standalone
    PRODUCT_NAME            "Ancient Synth"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES         "Instrument|Synth"
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice
)

target_sources(AncientSynth PRIVATE
    Source/PluginProcessor.cpp
)

# Expose Source/ so #include "../DSP.h" resolves from Source/UI/
target_include_directories(AncientSynth PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
)

target_compile_definitions(AncientSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Suppress warnings that are noisy but not errors
target_compile_options(AncientSynth PRIVATE
    -Wno-sign-conversion
    -Wno-conversion
    -Wno-shorten-64-to-32
    -Wno-switch-enum
    -Wno-float-equal
    -Wno-shadow
    -Wno-unused-parameter
    -Wno-pedantic
    -Wno-deprecated
)

target_link_libraries(AncientSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Demo-video renderer (cmake -DAS_VIDEO=ON): ASVideo outDir [scale]
option(AS_VIDEO "Build the ASVideo demo renderer" OFF)
if(AS_VIDEO)
  juce_add_console_app(ASVideo PRODUCT_NAME "ASVideo")
  target_sources(ASVideo PRIVATE Tests/VideoRender.cpp)
  target_include_directories(ASVideo PRIVATE ${CMAKE_SOURCE_DIR}/Source)
  target_compile_definitions(ASVideo PRIVATE JucePlugin_Name="Ancient Synth" JucePlugin_VersionString="3.1.0" JucePlugin_IsSynth=1 JucePlugin_WantsMidiInput=1 JucePlugin_ProducesMidiOutput=0 JucePlugin_IsMidiEffect=0 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
  target_link_libraries(ASVideo PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags)
endif()
