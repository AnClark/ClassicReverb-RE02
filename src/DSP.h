#ifndef DSP_H
#define DSP_H

#include <cstdint>
#include "Definitions.h"
#include "Constants.h"

using namespace ClassicReverbConsts;

struct ClassicReverbState {
    float params[kNumParams];
    float sampleRate;
    
    // Early reflections (expanded for high sample rates)
    int earlyWritePos;
    int earlyTapOffsets[7];
    float earlyBuffer[2][EARLY_REFLECTION_BUFFER_SIZE];  // Expanded 4x, supports 192kHz
    float earlyState[4];
    
    // Modulation delay
    int modWritePos[3];
    float modBuffers[3][MOD_DELAY_BUFFER_SIZE];  // Expanded 4x, supports 192kHz
    
    // 16-path comb filters
    float combBuffer[16][COMB_BUFFER_SIZE];  // Expanded 4x, supports 192kHz
    int combWritePos[16];
    int combSize[16];
    float combState[16][2];
    
    // Pre-delay
    int preDelayPos;
    int preDelaySize;
    float preDelayBuffer[2][PREDELAY_BUFFER_SIZE];  // Expanded 4x, supports 192kHz
    
    // All-pass filter state
    float allpassState[4];
    
    // Coefficients
    float feedbackCoeff;
    float dampingCoeff;
    float outputGain;
};

class ClassicReverb
{
    ClassicReverbState g_state; // Global reverb state for each DSP instance

public:
    ClassicReverb(float sampleRate);
    ~ClassicReverb() {}
    
    void processReplacing(const float** inputs, float** outputs, uint32_t sampleFrames);
    void setParameter(ClassicReverbParams index, float value);
    float getParameter(ClassicReverbParams index) const;
    void setSampleRate(float sampleRate);
    void resetBuffer(); // In VST 2.4, this is typically done in effMainsChanged when value == 0 (suspend)
                        // In DPF, this can be called on Plugin::activate() or Plugin::resume().

protected:
    void updateCoeffs();
    void processSample(float inL, float inR, float* outL, float* outR);
};

#endif // DSP_H
