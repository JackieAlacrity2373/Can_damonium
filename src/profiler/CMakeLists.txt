cmake_minimum_required(VERSION 3.24)

juce_add_gui_app(CanDamoniumProfiler
    VERSION 1.0.0
    DESCRIPTION "Impulse Response Profiler - Records amp profiles for standardization"
    PRODUCT_NAME "Can Damonium Profiler"
)

juce_generate_juce_header(CanDamoniumProfiler)

target_sources(CanDamoniumProfiler PRIVATE
    Main.cpp
    ProfilerWindow.cpp
    ProfilerComponent.cpp
    TestSignalGenerator.cpp
    IRRecorder.cpp
    IRProcessor.cpp
)

target_include_directories(CanDamoniumProfiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(CanDamoniumProfiler PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_gui_basics
)
