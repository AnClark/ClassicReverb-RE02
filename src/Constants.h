#ifndef CLASSIC_REVERB_CONSTANTS_H
#define CLASSIC_REVERB_CONSTANTS_H

// Constants extracted from the original Classic Reverb.dll (written in Delphi 7)
// Extraction date: 2026-03-04

namespace ClassicReverbConsts {

    // Early reflection tap gains (7 taps) - address 0x00488D0C
    static const float earlyTapGains[7] = {
        0.6499999762f, 0.4499999881f, 0.4099999964f, 0.3400000036f, 
        0.3100000024f, 0.2800000012f, 0.2399999946f
    };

    // Early reflection tap times (samples @ 44.1kHz)
    // Calculated from FUN_004845b8
    static const int earlyTapTimes[7] = {
        1238, 1471, 1783, 2095, 2407, 2719, 3031
    };

    // Comb filter mix coefficients Left channel (16 paths) - address 0x00488D74
    static const float combMixCoeffsL[16] = {
        +0.1985000074f, +0.1791999936f, -0.1234999970f, +0.3492999971f,
        +0.3014999926f, +0.2960000038f, -0.1323000044f, -0.0251000002f,
        +0.1665000021f, -0.0852800012f, +0.3416000009f, -0.2382999957f,
        +0.4674000144f, +0.3549999893f, -0.0678500012f, +0.3731000125f,
    };

    // Comb filter mix coefficients Right channel (16 paths) - address 0x00488DB4
    static const float combMixCoeffsR[16] = {
        -0.1879000068f, +0.0484500006f, +0.3702999949f, -0.0232800003f,
        +0.0234099999f, -0.3535999954f, -0.3925999999f, +0.2949000001f,
        +0.1508000046f, -0.0342799984f, +0.2205999941f, +0.3231999874f,
        -0.0836599991f, -0.2635000050f, -0.2529000044f, -0.1958999932f,
    };

    // Default parameter values (8 parameters) - address 0x0048884C
    // Parameter mapping (based on the original plugin provided by the user):
    //   [0] Room Size          (range: 0-1, default 1.0)
    //   [1] Damping            (range: 0-1, default 0.6)
    //   [2] Pre-delay          (range: -150ms~150ms, 0.5=0ms, default 0.5)
    //   [3] Hi Damp            (range: 0-1, default 0.5)
    //   [4] Lo Cut             (range: 0-1, default 0.5)
    //   [5] Early Reflection   (range: 0-1, default 0.3)
    //   [6] Mix                (range: 0-1, default 0.4)
    //   [7] Level              (range: 0-1, default 0.5)
    static const float defaultParams[8] = {
        1.000000f,  // Param 0: Room Size
        0.600000f,  // Param 1: Damping
        0.500000f,  // Param 2: Pre-delay (0.5 = 0ms, no delay)
        0.500000f,  // Param 3: Hi Damp
        0.500000f,  // Param 4: Lo Cut
        0.300000f,  // Param 5: Early Reflection
        0.400000f,  // Param 6: Mix
        0.500000f,  // Param 7: Level
    };
    
    // Coefficients calculated at runtime (computed in FUN_004845b8)
    // These values are computed during initialization/parameter changes, not hardcoded constants
    struct CalculatedCoeffs {
        float feedbackCoeff;      // 0x127bac: Comb filter feedback coefficient
        float dampingCoeff;       // 0x127ba8: sqrt(1 - feedback^2)
        float outputGain;         // 0x127be8: Output gain
        float allpassCoeff1;      // 0x127bbc
        float allpassCoeff2;      // 0x127bc0
        float allpassCoeff3;      // 0x127bc4
    };
    
    // DSP processing constants
    // NOTE: In the original plugin, DAT_00485f5c and DAT_00485f68 were initialized at runtime
    // Based on the usage context analysis, the following typical values are used:
    static const float ditherAmount = 0.00001f;   // Dither amount (~-100dB)
    static const float modDepth = 0.5f;           // Modulation depth (50%)
    
    // Delay buffer sizes
    static const int EARLY_REFLECTION_BUFFER_SIZE = 4096 * 4;   // early reflection buffer
    static const int MOD_DELAY_BUFFER_SIZE = 1024 * 4;          // modulation delay buffer
    static const int COMB_BUFFER_SIZE = 16384;       // comb filter buffer
    static const int PREDELAY_BUFFER_SIZE = 16384 * 4;          // predelay buffer
}

#endif // CLASSIC_REVERB_CONSTANTS_H
