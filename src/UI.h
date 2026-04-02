#ifndef CLASSIC_REVERB_UI_H
#define CLASSIC_REVERB_UI_H

#include <string>
#include <queue>
#include <mutex>

#include "DistrhoUI.hpp"
#include "FileBrowserDialog.hpp"  // DPF cross-platform file browser API
#include "Definitions.h"
#include "PresetManager.h"

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
    // State Callbacks (DPF WANT_STATE)

    void stateChanged(const char* key, const char* value) override;

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
    // Preset Manager UI state

    // Dialog mode enum (for modal popups inside the preset manager overlay)
    enum class PmDialogMode { None, SaveNew, Rename, ConfirmDelete, ConfirmUpdate };

    bool         fPresetManagerOpened = false;
    PmDialogMode fPmDialogMode        = PmDialogMode::None;
    char         fPmNameBuffer[128]   = {};   // text input for Save As / Rename dialogs

    // Buffered values for atomic state restoration from stateChanged() callbacks
    std::string fRestoredPresetType = "Factory";
    std::string fRestoredPresetName;
    bool        fRestoredModified   = false;

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

    // Preset Manager related procedures
    void _drawPresetManager();          // Draw the preset manager overlay window
    void _applyRestoredPresetState();   // Restore preset context from buffered stateChanged() values

    // -------------------------------------------------------------------
    // Instances

    ScopedPointer<PresetManager> fPresetManager;
    friend class PresetManager;

    // -------------------------------------------------------------------
    // File browser stuff (DPF cross-platform native file dialog)

    // Definitions & states
    enum class FileBrowserAction { None, Import, Export };
    DGL_NAMESPACE::FileBrowserHandle fFileBrowserHandle = nullptr;  // nullptr = no dialog open
    FileBrowserAction                fFileBrowserAction = FileBrowserAction::None;

    // Poll native file dialog each frame; process result when dialog closes
    void _handleFileBrowserIdle();

    // -------------------------------------------------------------------
    // Message box stuff

    // Definitions & states
    std::queue<std::string> fMessageBoxQueue;    // Queue of messages to be shown in message boxes
    bool                    fRequestMessagePopup = false;  // Trigger flag for message box display
    std::mutex              fMessageQueueMutex;  // Mutex to protect access to the message box queue

    // Poll message queue and show message box when needed.
    void _handleMessageBoxIdle();
    inline void _showMessageBox(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(fMessageQueueMutex);
        fMessageBoxQueue.push(message);
        fRequestMessagePopup = true;
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicReverbUI)
};

// -----------------------------------------------------------------------

#endif // CLASSIC_REVERB_UI_H
