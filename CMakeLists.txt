cmake_minimum_required (VERSION 3.20)

project (ClassicReverb-RE02
    VERSION 1.0.0
    DESCRIPTION "Reverse-engineered plugin of Kjaerhus Audio Classic Reverb"
    LANGUAGES C CXX
)

add_subdirectory (dpf)

include_directories (src)

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap
    FILES_DSP
        src/Plugin.cpp
        src/DSP.cpp
    FILES_UI
        src/UI.cpp
        src/UISubroutines.cpp
        widgets/dpf-imgui/DearImGui.cpp
        widgets/imgui-knobs-mod/imgui-knobs.cpp
)

# DistrhoPluginInfo.h must be findable by the DPF internal headers
target_include_directories(${PROJECT_NAME} PUBLIC
    src
    widgets/dpf-imgui
    widgets/dpf-imgui/DearImGui
    widgets/imgui-knobs-mod
    widgets/imgui-ext
    fonts
    ${PROJECT_BINARY_DIR}
)

# Enable cursor changes in the knob widget
target_compile_definitions(${PROJECT_NAME}-ui PRIVATE IMGUI_KNOBS_SET_CURSOR=1)

# Ensure C++17 (needed for std::clamp etc.)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Generate config.h from config.h.in
configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wno-unused-variable -ffast-math)
endif()
