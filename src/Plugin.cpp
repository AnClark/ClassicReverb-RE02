#include "Plugin.h"
#include "Definitions.h"
#include <cstring>

// ------------------------------------------------------------------------------------------------------------
// ClassicReverbPlugin implementation
// ------------------------------------------------------------------------------------------------------------

ClassicReverbPlugin::ClassicReverbPlugin()
    : Plugin(kNumParams, 0, 3)  // 3 states: preset_name, preset_modified, preset_type
{
    // Initialize DSP
    fReverb = std::make_unique<ClassicReverb>(getSampleRate());

    // Clean up buffer on start
    fReverb->resetBuffer();
}


/* ------------------------------------------------------------------
 * Init
 * ------------------------------------------------------------------ */

void ClassicReverbPlugin::initState(uint32_t index, State& state)
{
    state.hints = kStateIsHostWritable;

    switch (index)
    {
    case 0:
        state.key          = "preset_name";
        state.defaultValue = "";
        state.label        = "Current Preset Name";
        break;
    case 1:
        state.key          = "preset_modified";
        state.defaultValue = "false";
        state.label        = "Preset Modified";
        break;
    case 2:
        state.key          = "preset_type";
        state.defaultValue = "Factory";
        state.label        = "Preset Type";
        break;
    default:
        break;
    }
}

void ClassicReverbPlugin::setState(const char* /*key*/, const char* /*value*/)
{
    // Preset state is managed by the UI; the DSP side does not need to act on it.
    // DPF will forward state changes to the UI via stateChanged() automatically.
}

void ClassicReverbPlugin::initParameter(uint32_t index, Parameter& parameter)
{
    parameter.hints = kParameterIsAutomatable;
    parameter.ranges = kParamRanges[index];

    switch (index) {
    case kRoomSize:
        parameter.name = "Room Size";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.unit = "m2";  // Square metre
        break;
    case kDamping:
        parameter.name = "Damping";
        parameter.unit = "%";
        break;
    case kPreDelay:
        parameter.name = "Pre-delay";
        parameter.unit = "ms";
        break;
    case kHiDamp:
        parameter.name = "Hi Damp";
        parameter.unit = "%";
        break;
    case kLoCut:
        parameter.name = "Lo Cut";
        parameter.unit = "Hz";
        break;
    case kEarlyReflection:
        // FIXME: Early Reflection in original plugin is in Decibel (-∞ ~ 6.0 dB)
        parameter.name = "Early Reflection";
        parameter.unit = "dB";
        break;
    case kMix:
        parameter.name = "Mix";
        parameter.unit = "%";
        break;
    case kLevel:
        parameter.name = "Level";
        parameter.unit = "dB";
        break;
    }

    parameter.symbol = parameter.name;
    parameter.symbol.toBasic();

    // Apply default param value
    setParameterValue(index, parameter.ranges.def);
}

/* ------------------------------------------------------------------
 * Internal data
 * ------------------------------------------------------------------ */

float ClassicReverbPlugin::getParameterValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr && index < kNumParams, 0.0f);

    // All parameters are mapped to 0.0-1.0 internally for compatibility with VST 2.4 specs,
    // so we need to convert from the actual range.

    switch (index) {
    case kRoomSize:
        return (fReverb->getParameter(kRoomSize) * (640.0f - 0.625f)) + 0.625f;
    case kDamping:
        return fReverb->getParameter(kDamping) * 100.0f;
    case kPreDelay:
        return (fReverb->getParameter(kPreDelay) * 300.0f) - 150.0f;
    case kHiDamp:
        return fReverb->getParameter(kHiDamp) * 100.0f;
    case kLoCut:
        return (fReverb->getParameter(kLoCut) * (1000.0f - 20.0f)) + 20.0f;
    case kEarlyReflection:
        return (fReverb->getParameter(kEarlyReflection) * (6.0f + 40.0f)) - 40.0f;
    case kMix:
        return fReverb->getParameter(kMix) * 100.0f;
    case kLevel:
        return (fReverb->getParameter(kLevel) * 20.0f) - 10.0f;
    }

    return 0.0f;
}

void ClassicReverbPlugin::setParameterValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr && index < kNumParams, )

    // All parameters are mapped to 0.0-1.0 internally for compatibility with VST 2.4 specs,
    // so we need to convert from the actual range.

    switch (index) {
    case kRoomSize:
        fReverb->setParameter(kRoomSize, (value - 0.625f) / (640.0f - 0.625f));
        break;
    case kDamping:
        fReverb->setParameter(kDamping, value / 100.0f);
        break;
    case kPreDelay:
        fReverb->setParameter(kPreDelay, (value + 150.0f) / 300.0f);
        break;
    case kHiDamp:
        fReverb->setParameter(kHiDamp, value / 100.0f);
        break;
    case kLoCut:
        fReverb->setParameter(kLoCut, (value - 20.0f) / (1000.0f - 20.0f));
        break;
    case kEarlyReflection:
        fReverb->setParameter(kEarlyReflection, (value + 40.0f) / (6.0f + 40.0f));
        break;
    case kMix:
        fReverb->setParameter(kMix, value / 100.0f);
        break;
    case kLevel:
        fReverb->setParameter(kLevel, (value + 10.0f) / 20.0f);
        break;
    }
}


/* ------------------------------------------------------------------
 * Process
 * ------------------------------------------------------------------ */

void ClassicReverbPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr, )

    fReverb->processReplacing(inputs, outputs, frames);
}

void ClassicReverbPlugin::activate()
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr, )

    fReverb->resetBuffer();
}

void ClassicReverbPlugin::deactivate()
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr, )

    fReverb->resetBuffer();
}

/* ------------------------------------------------------------------
 * Host events
 * ------------------------------------------------------------------ */

void ClassicReverbPlugin::sampleRateChanged(double newSampleRate)
{
    DISTRHO_SAFE_ASSERT_RETURN(fReverb.get() != nullptr, )

    fReverb->setSampleRate(newSampleRate);
}

// -----------------------------------------------------------------------------

START_NAMESPACE_DISTRHO

Plugin* createPlugin()
{
    return new ClassicReverbPlugin();
}

END_NAMESPACE_DISTRHO
