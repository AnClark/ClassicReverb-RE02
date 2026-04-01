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
)
