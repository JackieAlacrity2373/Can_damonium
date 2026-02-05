juce_add_plugin(CanDamoniumPlugin
    VERSION 1.0.0
    PLUGIN_MANUFACTURER_CODE Cand
    PLUGIN_CODE Cdam
    DESCRIPTION "Impulse Response Convolver VST3 Plugin"
    FORMATS VST3 Standalone
    PRODUCT_NAME "Can Damonium"
)

juce_generate_juce_header(CanDamoniumPlugin)

target_sources(CanDamoniumPlugin PRIVATE
    Main.cpp
    PluginProcessor.cpp
    PluginEditor.cpp
    ConvolutionEngine.cpp
    IRLibrary.cpp
    IRLibraryManager.cpp
)

target_include_directories(CanDamoniumPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(CanDamoniumPlugin PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_gui_basics
)

target_compile_definitions(CanDamoniumPlugin PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
)

# Copy embedded IR resources
add_custom_command(TARGET CanDamoniumPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "IR resources will be embedded in binary"
)
