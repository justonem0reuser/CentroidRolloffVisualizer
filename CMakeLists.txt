cmake_minimum_required(VERSION 3.22)
project("CentroidRolloffVisualizer")

include(FetchContent)

set(JUCE_VERSION "8.0.12")

# config.cmake can be added to specify another directory for external dependencies
if(EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    include("${CMAKE_SOURCE_DIR}/config.cmake") # for THIRD_PARTY_DIR
endif()
if (NOT DEFINED THIRD_PARTY_DIR)
    set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/build/_deps")
endif()

set(FETCHCONTENT_BASE_DIR "${THIRD_PARTY_DIR}/juce-${JUCE_VERSION}")
set(JUCE_URL "https://github.com/juce-framework/JUCE.git")

# Fetch JUCE
FetchContent_Declare(
    JUCE 
    GIT_REPOSITORY ${JUCE_URL} 
    GIT_TAG ${JUCE_VERSION}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

set(BUILD_STANDALONE_DEFAULT ON)
if(DEFINED ENV{CI})
    set(BUILD_STANDALONE_DEFAULT OFF)
endif()
option(BUILD_STANDALONE "Build Standalone" ${BUILD_STANDALONE_DEFAULT})

set(JUCE_PLUGIN_FORMATS VST3 AU)
if(BUILD_STANDALONE)
    list(APPEND JUCE_PLUGIN_FORMATS Standalone)
endif()

set(PLUGIN_VERSION_DEFAULT "0.1.0")
if(NOT DEFINED VERSION)
    set(VERSION ${PLUGIN_VERSION_DEFAULT})
endif()

juce_add_plugin(CentroidRolloffVisualizer
  VERSION ${VERSION}
  PRODUCT_NAME "CentroidRolloffVisualizer"
  COMPANY_NAME "Just1m0reApp"
  COMPANY_WEBSITE "https://github.com/justonem0reuser"

  PLUGIN_MANUFACTURER_CODE Jmor
  PLUGIN_CODE Crov

  FORMATS ${JUCE_PLUGIN_FORMATS}

  AU_MAIN_TYPE kAudioUnitType_Effect

  VST3_CATEGORIES
    "Analyzer"
  VST2_CATEGORY "kPlugCategAnalysis"
)

# This is required to add a target to MSVS in a local machine
if (MSVC AND BUILD_STANDALONE)
    add_custom_target(Install
        COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --config $<CONFIG>
    )
    add_dependencies(Install CentroidRolloffVisualizer_VST3)
    if(TARGET CentroidRolloffVisualizer_AU)
        add_dependencies(Install CentroidRolloffVisualizer_AU)
    endif()
endif()

juce_generate_juce_header(CentroidRolloffVisualizer)

target_compile_definitions(CentroidRolloffVisualizer
  PUBLIC
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
)

if (MSVC)
    target_compile_options(CentroidRolloffVisualizer PRIVATE
        $<$<CONFIG:Release>:/O2 /DNDEBUG /GL>
    )

    target_link_options(CentroidRolloffVisualizer PRIVATE
        $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /INCREMENTAL:NO /LTCG>
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_options(CentroidRolloffVisualizer PRIVATE
        -static-libstdc++
        -static-libgcc
)
endif()

target_sources(CentroidRolloffVisualizer
  PRIVATE
    "Source/CentroidRolloffPanVisualizer.cpp"
    "Source/FeaturesCalculator.cpp"
    "Source/Plot.cpp"
    "Source/PluginEditor.cpp"
    "Source/PluginProcessor.cpp"
)

target_link_libraries(CentroidRolloffVisualizer
  PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Installation target

if(APPLE)
    install(
        DIRECTORY
            "${CMAKE_BINARY_DIR}/CentroidRolloffVisualizer_artefacts/$<CONFIG>/AU/CentroidRolloffVisualizer.component/"
        DESTINATION
            "$ENV{HOME}/Library/Audio/Plug-Ins/Components/CentroidRolloffVisualizer.component"
    )
    install(
        DIRECTORY
            "${CMAKE_BINARY_DIR}/CentroidRolloffVisualizer_artefacts/$<CONFIG>/VST3/CentroidRolloffVisualizer.vst3/"
        DESTINATION
            "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/CentroidRolloffVisualizer.vst3"
    )
elseif(UNIX)
    install(
        DIRECTORY
            "${CMAKE_BINARY_DIR}/CentroidRolloffVisualizer_artefacts/$<CONFIG>/VST3/CentroidRolloffVisualizer.vst3/"
        DESTINATION
            "$ENV{HOME}/.vst3/CentroidRolloffVisualizer.vst3"
    )
elseif(WIN32)
    install(
        DIRECTORY
            "${CMAKE_BINARY_DIR}/CentroidRolloffVisualizer_artefacts/$<CONFIG>/VST3/CentroidRolloffVisualizer.vst3/"
        DESTINATION
            "$ENV{PROGRAMFILES}/Common Files/VST3/CentroidRolloffVisualizer.vst3"
    )
endif()

# Run additional script if exist (for example, run DAW)
set(POST_BUILD_SCRIPT "${CMAKE_SOURCE_DIR}/post_build_plugin.cmake")
if(EXISTS "${POST_BUILD_SCRIPT}")
    install(SCRIPT "${POST_BUILD_SCRIPT}")
endif()
