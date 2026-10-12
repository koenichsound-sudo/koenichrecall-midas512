cmake_minimum_required(VERSION 3.22)
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Apple Silicon")
set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "Minimum macOS")
project(KoenichRecallMIDAS512 VERSION 1.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_SOURCE_DIR "" CACHE PATH "Local JUCE 8.0.15 source directory; empty downloads it")
if(JUCE_SOURCE_DIR)
  add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.15 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(KoenichRecallMIDAS512
  COMPANY_NAME "Koenich Sound" BUNDLE_ID "sound.koenich.recall.midas512"
  PLUGIN_MANUFACTURER_CODE Koen PLUGIN_CODE Km51
  FORMATS VST3 PRODUCT_NAME "Koenich Recall - MIDAS 512 EQ"
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Tools)
target_sources(KoenichRecallMIDAS512 PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(KoenichRecallMIDAS512 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(KoenichRecallMIDAS512 PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
if(APPLE)
  # JUCE creates moduleinfo.json after its initial signing step. Seal the final bundle.
  add_custom_command(TARGET KoenichRecallMIDAS512_VST3 POST_BUILD
    COMMAND /usr/bin/codesign --force --sign - "$<TARGET_BUNDLE_DIR:KoenichRecallMIDAS512_VST3>"
    VERBATIM)
endif()
option(RECALL_BUILD_TESTS "Build processor, GUI and VST3 integration tests" ON)
if(RECALL_BUILD_TESTS)
  juce_add_console_app(RecallTests PRODUCT_NAME RecallTests)
  target_sources(RecallTests PRIVATE Tests/RecallTests.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_compile_definitions(RecallTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_PLUGINHOST_VST3=1 JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(RecallTests PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
  enable_testing()
  add_test(NAME RecallTests COMMAND RecallTests)
endif()
