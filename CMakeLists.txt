cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static CRT: no VC++ redistributable needed
project(Yolo VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(Yolo
    COMPANY_NAME "Yolo"
    PLUGIN_MANUFACTURER_CODE Yolo
    PLUGIN_CODE Ylo1
    FORMATS VST3
    PRODUCT_NAME "Yolo"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Pitch\ Shift
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(Yolo PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(Yolo PRIVATE Source)
target_compile_definitions(Yolo PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0 JUCE_STRICT_REFCOUNTEDPOINTER=1)
target_link_libraries(Yolo PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

if(UNIX AND NOT APPLE)
    # allow building on minimal Linux boxes / CI without every dev header
    target_compile_definitions(Yolo PUBLIC JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XSHM=0
        JUCE_USE_XRENDER=0 JUCE_USE_XCURSOR=0 JUCE_JACK=0 JUCE_ALSA=0)
endif()

option(YOLO_SNAPSHOT "Build editor snapshot tool" OFF)
if(YOLO_SNAPSHOT)
    juce_add_console_app(YoloSnap PRODUCT_NAME YoloSnap)
    target_sources(YoloSnap PRIVATE Tests/snap.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(YoloSnap PRIVATE Source)
    target_compile_definitions(YoloSnap PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
    target_link_libraries(YoloSnap PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics)
endif()

if(YOLO_SNAPSHOT)
    juce_add_console_app(YoloAnim PRODUCT_NAME YoloAnim)
    target_sources(YoloAnim PRIVATE Tests/snap_anim.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(YoloAnim PRIVATE Source)
    target_compile_definitions(YoloAnim PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
    target_link_libraries(YoloAnim PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics)
endif()

if(YOLO_SNAPSHOT)
    juce_add_console_app(YoloFunc PRODUCT_NAME YoloFunc)
    target_sources(YoloFunc PRIVATE Tests/func_test.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(YoloFunc PRIVATE Source)
    target_compile_definitions(YoloFunc PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
    target_link_libraries(YoloFunc PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics)
endif()
