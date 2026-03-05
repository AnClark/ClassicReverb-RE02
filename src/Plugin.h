#ifndef CLASSIC_REVERB_PLUGIN_H
#define CLASSIC_REVERB_PLUGIN_H

#include "DSP.h"
#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"

#include <memory>

// Example plugin demonstrating parameter handling
class ClassicReverbPlugin : public DISTRHO::Plugin
{
public:
    ClassicReverbPlugin();

protected:
    /* --------------------------------------------------------------------------------------------------------
     * Information */
    const char* getLabel() const override
    {
        return DISTRHO_PLUGIN_NAME;
    }

    const char* getDescription() const override
    {
        return DISTRHO_PLUGIN_DESCRIPTION;
    }

    const char* getMaker() const override
    {
        return DISTRHO_PLUGIN_BRAND;
    }

    const char* getHomePage() const override
    {
        return DISTRHO_PLUGIN_URI;
    }

    const char* getLicense() const override
    {
        return DISTRHO_PLUGIN_LICENSE;
    }

    uint32_t getVersion() const override
    {
        return DISTRHO_PLUGIN_VERSION;
    }

    int64_t getUniqueId() const override
    {
        return d_cconst('K', 'R', 'e', 'v');
    }

    /* --------------------------------------------------------------------------------------------------------
     * Init */

    void initParameter(uint32_t index, Parameter& parameter) override;

    /* --------------------------------------------------------------------------------------------------------
     * Internal data accessors */

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    /* --------------------------------------------------------------------------------------------------------
    * Audio/MIDI Processing */

    void activate() override;
    void deactivate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

    /* --------------------------------------------------------------------------------------------------------
     * Host events */
    
    void sampleRateChanged(double newSampleRate) override;

private:
    std::unique_ptr<ClassicReverb> fReverb; // Reverb processing instance

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicReverbPlugin)
};

#endif // CLASSIC_REVERB_PLUGIN_H
