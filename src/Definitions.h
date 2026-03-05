#ifndef CLASSIC_REVERB_DEFINITIONS_H
#define CLASSIC_REVERB_DEFINITIONS_H

// Parameter enumeration (matches original plugin)
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

#endif // CLASSIC_REVERB_DEFINITIONS_H
