juce_add_gui_app(CanDamoniumRecorder
    VERSION 1.0.0
    DESCRIPTION "Impulse Response Recording Guidance Application"
    PRODUCT_NAME "Can Damonium IR Recorder"
)

juce_generate_juce_header(CanDamoniumRecorder)

target_sources(CanDamoniumRecorder PRIVATE
    Main.cpp
    RecorderWindow.cpp
    RecorderComponent.cpp
    InstructionPanel.cpp
    TestSignalGenerator.cpp
)

target_include_directories(CanDamoniumRecorder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(CanDamoniumRecorder PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_gui_basics
)
