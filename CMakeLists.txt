cmake_minimum_required(VERSION 3.22)
project(Quasar_EQ_6 VERSION 6.0.4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(C:/zlth_base/4_dev/1_juce/quasar-eq/JUCE)

juce_add_plugin(Quasar_EQ_6
    PLUGIN_MANUFACTURER_CODE Zlth
    PLUGIN_CODE K1wg
    FORMATS VST3
    PRODUCT_NAME "Quasar EQ 6"
    VST3_CAN_REPLACE_VST2 FALSE)

add_subdirectory(Source)
add_subdirectory(Resources)

target_link_libraries(Quasar_EQ_6
    PRIVATE
        BinaryData
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags)

target_compile_definitions(Quasar_EQ_6
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0)

juce_generate_juce_header(Quasar_EQ_6)
