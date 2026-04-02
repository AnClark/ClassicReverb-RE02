#ifndef CLASSIC_REVERB_UI_H
#define CLASSIC_REVERB_UI_H

#include <cstring>

#include "DistrhoUI.hpp"
#include "Definitions.h"

// Forward decls.
namespace ImGuiKnobs_Mod {
    struct KnobScaleMark;
}

// -----------------------------------------------------------------------

class ClassicReverbUI : public DISTRHO::UI
{
public:
    ClassicReverbUI();

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;

    // -------------------------------------------------------------------
    // ImGui Callbacks

    void onImGuiDisplay() override;


private:
    // -------------------------------------------------------------------
    // Local variables

    float fParams[kNumParams];

    bool fAboutWindowOpened;  // Flag to track if the "About" window is open
    int  fLastMouseCursor;    // To track the last mouse cursor state for optimization

    // -------------------------------------------------------------------
    // Internal procedures

    void _loadFonts();  // Load ImGui fonts (invoked in constructor)
    void _drawChassisBackground(float margin, float rounding);  // Draw the background chassis
    void _drawKjaerhusLogo(const ImVec2& size);
    void _drawPluginName();

    // Helper to add a knob (explicit range overload)
    void _addKnob(ClassicReverbParams paramId, const char* label,
                  float v_min, float v_max,
                  const ImGuiKnobs_Mod::KnobScaleMark* marks, uint32_t mark_count,
                  bool isLogarithmic = false,
                  bool use_pivot = false, float pivot_value = 0.0f);

    // Helper to add a knob using kParamRanges (range inferred from Definitions.h)
    inline void _addKnob(ClassicReverbParams paramId, const char* label,
                         const ImGuiKnobs_Mod::KnobScaleMark* marks, uint32_t mark_count,
                         bool isLogarithmic = false,
                         bool use_pivot = false, float pivot_value = 0.0f)
    {
        _addKnob(paramId, label,
                 kParamRanges[paramId].min, kParamRanges[paramId].max,
                 marks, mark_count, isLogarithmic, use_pivot, pivot_value);
    }

    bool _BeginSection(const char* title, float width);
    void _EndSection();
    void _UpdateMouseCursor();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicReverbUI)
};

// -----------------------------------------------------------------------

#endif // CLASSIC_REVERB_UI_H
