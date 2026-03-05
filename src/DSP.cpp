#include "DSP.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

ClassicReverb::ClassicReverb(float sampleRate)
{
    // initialization
    memset(&g_state, 0, sizeof(g_state));

    // set default sample rate (prevent division by zero in updateCoeffs)
    g_state.sampleRate = sampleRate;
    for (int i = 0; i < kNumParams; i++) {
        g_state.params[i] = defaultParams[i];
    }

    updateCoeffs();
}

// ============================================================
// Core DSP function (corresponds to Delphi FUN_004845b8)
// ============================================================
void ClassicReverb::updateCoeffs() {
    // Room Size controls feedback amount
    float roomSize = g_state.params[kRoomSize];
    g_state.feedbackCoeff = 0.7f + roomSize * 0.25f;
    g_state.dampingCoeff = sqrtf(1.0f - g_state.feedbackCoeff * g_state.feedbackCoeff);
    
    // Damping controls overall high-frequency attenuation
    float damping = g_state.params[kDamping];
    
    // calculate early reflection tap times (based on pre-delay)
    // note: earlyTapTimes are sample counts at 44.1kHz and need scaling by sample rate
    float preDelay = g_state.params[kPreDelay];
    float sampleRateRatio = g_state.sampleRate / 44100.0f;
    int baseDelay = (int)(preDelay * 1000);
    for (int i = 0; i < 7; i++) {
        int scaledTapTime = (int)(earlyTapTimes[i] * sampleRateRatio);
        g_state.earlyTapOffsets[i] = scaledTapTime + baseDelay;
        if (g_state.earlyTapOffsets[i] >= 16384) 
            g_state.earlyTapOffsets[i] = 16384 - 1;  // prevent overflow
    }
    
    // compute comb filter sizes (based on room size)
    // note: base sizes must be scaled by sample rate to keep delay times consistent
    for (int i = 0; i < 16; i++) {
        // base delay time: (1000 + i*200) samples @44.1kHz
        // converts to ~22.68ms + i*4.54ms
        int baseSize = (int)((1000 + i * 200) * sampleRateRatio);
        g_state.combSize[i] = baseSize + (int)(roomSize * 1000 * sampleRateRatio);
        if (g_state.combSize[i] > 16384) g_state.combSize[i] = 16384;
    }
    
    // pre-delay size calculation
    // PreDelay range: -150ms ~ +150ms (parameter 0.0~1.0 maps to -150~+150ms, 0.5=0ms)
    // actual delay = abs(preDelay - 0.5) * 300ms
    float delayMs = fabsf(preDelay - 0.5f) * 300.0f;  // 0-150ms
    g_state.preDelaySize = (int)(delayMs * g_state.sampleRate / 1000.0f);
    if (g_state.preDelaySize < 1) g_state.preDelaySize = 1;  // minimum 1 to avoid division by zero
    if (g_state.preDelaySize > 65536) g_state.preDelaySize = 65536;  // support high sample rates
}

void ClassicReverb::setParameter(ClassicReverbParams index, float value) {
    if (index >= 0 && index < kNumParams) {
        g_state.params[index] = value;
        updateCoeffs();
    }
}

float ClassicReverb::getParameter(ClassicReverbParams index) const {
    if (index >= 0 && index < kNumParams) {
        return g_state.params[index];
    }
    return 0.0f;
}

void ClassicReverb::setSampleRate(float sampleRate) {
    g_state.sampleRate = sampleRate;
    updateCoeffs();
}

void ClassicReverb::resetBuffer() {
    memset(&g_state.earlyBuffer, 0, sizeof(g_state.earlyBuffer));
    memset(&g_state.modBuffers, 0, sizeof(g_state.modBuffers));
    memset(&g_state.combBuffer, 0, sizeof(g_state.combBuffer));
    memset(&g_state.preDelayBuffer, 0, sizeof(g_state.preDelayBuffer));
}

// ============================================================
// Single-sample processing (corresponds to Delphi FUN_0048490c)
// ============================================================
void ClassicReverb::processSample(float inL, float inR, float* outL, float* outR) {
    // 1. dither injection
    float dither = ((float)rand() / RAND_MAX - 0.5f) * 0.00001f;
    float mono = (inL + inR) * 0.5f + dither;
    
    // 2. write to early reflection buffer (using raw input)
    g_state.earlyBuffer[0][g_state.earlyWritePos] = inL;
    g_state.earlyBuffer[1][g_state.earlyWritePos] = inR;
    
    // 3. 7-tap early reflections
    float earlyL = 0.0f, earlyR = 0.0f;
    for (int i = 0; i < 7; i++) {
        int tapPos = g_state.earlyWritePos - g_state.earlyTapOffsets[i];
        if (tapPos < 0) tapPos += 16384;
        earlyL += g_state.earlyBuffer[0][tapPos] * earlyTapGains[i];
        earlyR += g_state.earlyBuffer[1][tapPos] * earlyTapGains[i];
    }
    g_state.earlyWritePos = (g_state.earlyWritePos + 1) & 16383;
    
    // 4. early reflection EQ and mix
    float earlyMix = g_state.params[kEarlyReflection];
    earlyL *= earlyMix;
    earlyR *= earlyMix;
    
    // low-cut filter (1st-order high-pass)
    float loCut = g_state.params[kLoCut];
    float loCutCoeff = 20.0f + loCut * 980.0f;  // 20-1000Hz
    float rc = 1.0f / (2.0f * 3.14159f * loCutCoeff);
    float alpha = 1.0f / (1.0f + rc * g_state.sampleRate);
    static float loCutState[2] = {0, 0};
    earlyL = alpha * (loCutState[0] + earlyL - earlyL);  // simplified HP
    earlyR = alpha * (loCutState[1] + earlyR - earlyR);
    loCutState[0] = earlyL;
    loCutState[1] = earlyR;
    
    // high-frequency damping filter (1st-order low-pass)
    float hiDamp = g_state.params[kHiDamp];
    float hiDampCoeff = hiDamp * 0.5f;
    earlyL = earlyL * (1.0f - hiDampCoeff) + g_state.earlyState[2] * hiDampCoeff;
    earlyR = earlyR * (1.0f - hiDampCoeff) + g_state.earlyState[3] * hiDampCoeff;
    g_state.earlyState[2] = earlyL;
    g_state.earlyState[3] = earlyR;
    
    // 5. mix early reflections into mono signal
    float modOut = mono + (earlyL + earlyR) * 0.5f;
    for (int i = 0; i < 3; i++) {
        int readPos = g_state.modWritePos[i] - (600 + i * 170);
        if (readPos < 0) readPos += 4096;
        float delayed = g_state.modBuffers[i][readPos];
        float out = modOut - delayed * 0.5f;
        g_state.modBuffers[i][g_state.modWritePos[i]] = modOut + delayed * 0.5f;
        g_state.modWritePos[i] = (g_state.modWritePos[i] + 1) & 4095;
        modOut = out;
    }
    
    // 6. 16-way comb filters
    float combSumL = 0.0f, combSumR = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        float* buffer = g_state.combBuffer[i];
        int pos = g_state.combWritePos[i];
        float sample = buffer[pos];
        
        combSumL += sample * combMixCoeffsL[i];
        combSumR += sample * combMixCoeffsR[i];
        
        // feedback + damping
        float feedback = sample * g_state.feedbackCoeff + modOut;
        float damped = feedback * (1.0f - g_state.params[kDamping] * 0.5f) 
                      + g_state.combState[i][0] * (g_state.params[kDamping] * 0.5f);
        g_state.combState[i][0] = damped;
        buffer[pos] = damped;
        
        g_state.combWritePos[i]++;
        if (g_state.combWritePos[i] >= g_state.combSize[i]) 
            g_state.combWritePos[i] = 0;
    }
    
    // 7. allpass diffusion (uses Hi Damp parameter)
    float apL = combSumL;
    float apR = combSumR;
    float apCoeff = g_state.params[kHiDamp];
    for (int stage = 0; stage < 2; stage++) {
        float inApL = apL;
        float inApR = apR;
        apL = g_state.allpassState[stage*2] * (1.0f - apCoeff) + inApL * apCoeff;
        apR = g_state.allpassState[stage*2+1] * (1.0f - apCoeff) + inApR * apCoeff;
        g_state.allpassState[stage*2] = inApL;
        g_state.allpassState[stage*2+1] = inApR;
    }
    
    // 8. pre-delay - key fix: decide which signal to delay based on PreDelay parameter
    // PreDelay range: -150ms ~ +150ms (param 0.0~1.0, center 0.5=0ms)
    // <= 0.5: delay dry signal (input), letting reverb lead ("negative delay")
    // >  0.5: delay wet signal (reverb), dry stays immediate ("positive delay")
    float preDelay = g_state.params[kPreDelay];
    float dryL = inL;  // 用于干声的输入
    float dryR = inR;
    float wetL = apL;  // 用于湿声的混响输出
    float wetR = apR;
    
    // apply delay only when away from center 0.5 (tolerance 0.001)
    if (fabsf(preDelay - 0.5f) > 0.001f) {
        if (preDelay <= 0.5f) {
            // PreDelay < 0.5: "negative delay" - delay dry, let reverb lead
            // actual delay amount: (0.5 - preDelay) * 300ms
            dryL = g_state.preDelayBuffer[0][g_state.preDelayPos];
            dryR = g_state.preDelayBuffer[1][g_state.preDelayPos];
            g_state.preDelayBuffer[0][g_state.preDelayPos] = inL;
            g_state.preDelayBuffer[1][g_state.preDelayPos] = inR;
            g_state.preDelayPos++;
            if (g_state.preDelayPos >= g_state.preDelaySize) g_state.preDelayPos = 0;
        } else {
            // PreDelay > 0.5: "positive delay" - delay reverb, dry is immediate
            // actual delay amount: (preDelay - 0.5) * 300ms
            wetL = g_state.preDelayBuffer[0][g_state.preDelayPos];
            wetR = g_state.preDelayBuffer[1][g_state.preDelayPos];
            g_state.preDelayBuffer[0][g_state.preDelayPos] = apL;
            g_state.preDelayBuffer[1][g_state.preDelayPos] = apR;
            g_state.preDelayPos++;
            if (g_state.preDelayPos >= g_state.preDelaySize) g_state.preDelayPos = 0;
        }
    }
    
    // 9. output mix
    float mix = g_state.params[kMix];
    float dryGain = 1.0f - mix;
    float wetGain = mix * g_state.params[kLevel] * 2.0f;  // Level parameter controls wet signal level
    
    *outL = dryL * dryGain + wetL * wetGain;
    *outR = dryR * dryGain + wetR * wetGain;
}

void ClassicReverb::processReplacing(const float** inputs, float** outputs, uint32_t sampleFrames)
{
    const float* inL = inputs[0];
    const float* inR = inputs[1];
    float* outL = outputs[0];
    float* outR = outputs[1];
    
    for (int32_t i = 0; i < sampleFrames; i++) {
        processSample(inL[i], inR[i], &outL[i], &outR[i]);
    }
}
