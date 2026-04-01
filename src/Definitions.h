#ifndef CLASSIC_REVERB_DEFINITIONS_H
#define CLASSIC_REVERB_DEFINITIONS_H

#include "DistrhoDetails.hpp"   // For DISTRHO::ParameterRanges

// Parameter enumeration — used by the DSP layer
enum ClassicReverbParams {
    kRoomSize = 0,      // Room size
    kDamping,           // Damping
    kPreDelay,          // Pre-delay
    kHiDamp,            // High frequency damping
    kLoCut,             // Low cut
    kEarlyReflection,   // Early reflection
    kMix,               // Dry/Wet mix
    kLevel,             // Output level
    kNumParams
};

// Parameter ranges — kept in sync with Plugin.cpp::initParameter()
constexpr DISTRHO::ParameterRanges kParamRanges[kNumParams] = {
    // { def,    min,     max    }
    { 150.0f,  0.625f, 640.0f   },   // Room Size, m²
    {  30.0f,  0.0f,   100.0f   },   // Damping, %
    {   0.0f, -150.0f, 150.0f   },   // Pre-delay, ms
    {   0.0f,  0.0f,   100.0f   },   // Hi Damping, %
    {  20.0f,  20.0f, 1000.0f   },   // Lo-Cut, Hz
    {   1.6f, -40.0f,   6.0f    },   // Early Reflections, dB
    {  50.0f,  0.0f,   100.0f   },   // Mix, %
    {   0.0f, -10.0f,  10.0f    },   // Level, dB
};

#endif // CLASSIC_REVERB_DEFINITIONS_H
