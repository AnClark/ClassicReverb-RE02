#include "UI.h"

#include "CenteredSeparatorText.hpp"
#include "imgui-knobs.h"
#include "AddTextScaled.hpp"

#include "../fonts/LiberationSans-Regular.hpp"
#include "../fonts/CormorantFont.hpp"
#include "src/Resources.hpp"    // Dejavu Sans font (bundled with DGL, resolved via dpf/dgl include path)
#include "../fonts/FontAwesome5.hpp"
#include "../fonts/IconFontAwesome5.h"

static constexpr float kScaleMarkInitFontSize = 12.5f;

// -----------------------------------------------------------------------
// Scale-mark style (shared across knobs)

ImGuiKnobs_Mod::KnobScaleMarkStyle kScaleMarkStyle = {
    .outer_radius = 1.20f,
    .tick_length  = 0.50f,
    .font_size    = kScaleMarkInitFontSize,
};

// -----------------------------------------------------------------------
// Font loading

void ClassicReverbUI::_loadFonts()
{
    // Font sizes:
    //  #0  – 12.5 px  Chassis regular text  (knob labels)
    //  #1  – 14.0 px  Section titles
    //  #2  – 20.0 px  Scale-mark / Kjaerhus logo (down-sampled by knob widget)
    //  #3  – 20.0 px  Cormorant SemiBoldItalic  (plugin-name mark)
    //  #4  – 14.5 px  Dejavu Sans  (ImGui menu / tooltip)  + Font Awesome icons (merged)

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig fc;
    fc.FontDataOwnedByAtlas = false;
    fc.OversampleH = 1;
    fc.OversampleV = 1;
    fc.PixelSnapH  = true;

    io.Fonts->Clear();

    const float scale = getScaleFactor();

    // Font #0 – chassis regular text
    static constexpr ImWchar kChassisRanges[] = { ' ', '~', 178, 179, 0 };  // Basic Latin + '²'
    io.Fonts->AddFontFromMemoryCompressedTTF(
        (void*)LiberationSansTTF_Compressed_compressed_data,
        LiberationSansTTF_Compressed_compressed_size,
        12.5f * scale, &fc, kChassisRanges);

    // Font #1 – section titles (uppercase only, reduces atlas size)
    static constexpr ImWchar kTitleRanges[] = { 'A', 'Z', 0 };
    io.Fonts->AddFontFromMemoryCompressedTTF(
        (void*)LiberationSansTTF_Compressed_compressed_data,
        LiberationSansTTF_Compressed_compressed_size,
        14.0f * scale, &fc, kTitleRanges);

    // Font #2 – scale marks + logo  (oversized, down-sampled at render time)
    static constexpr ImWchar kScaleMarkRanges[] = {
        'A', 'Z', 'a', 'z', '0', '9', ' ', '!',
        '+', ':', 8734, 8735,   // infinity ∞
        198, 199,               // Æ (Liberation Sans)
        0 };
    io.Fonts->AddFontFromMemoryCompressedTTF(
        (void*)LiberationSansTTF_Compressed_compressed_data,
        LiberationSansTTF_Compressed_compressed_size,
        20.0f * scale, &fc, kScaleMarkRanges);

    // Font #3 – Cormorant SemiBoldItalic for "Classic Reverb" logotype
    static constexpr ImWchar kPluginNameRanges[] = { 'A', 'Z', 'a', 'z', '0', '9', ' ', '!', 0 };
    io.Fonts->AddFontFromMemoryCompressedTTF(
        (void*)CormorantSemiBoldItalicTTF_compressed_data,
        CormorantSemiBoldItalicTTF_compressed_size,
        20.0f * scale, &fc, kPluginNameRanges);

    // Font #4 – Dejavu Sans for ImGui menus / tooltips (full charset)
    io.Fonts->AddFontFromMemoryTTF(
        (void*)dpf_resources::dejavusans_ttf,
        dpf_resources::dejavusans_ttf_size,
        14.5f * scale, &fc);

    // Font Awesome icons – merged into #4
    static constexpr ImWchar kFAranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    fc.MergeMode = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(
        (void*)FontAwesomeTTF_compressed_data,
        FontAwesomeTTF_compressed_size,
        14.5f * scale, &fc, kFAranges);
    fc.MergeMode = false;

    io.Fonts->Build();
    io.FontDefault = io.Fonts->Fonts[4];

    // Tell the knob widget to use the oversized font for scale marks
    kScaleMarkStyle.custom_font = io.Fonts->Fonts[2];

    // Remember to scale the scale mark font's size to screen DPI
    kScaleMarkStyle.font_size = kScaleMarkInitFontSize * getScaleFactor();
}

// -----------------------------------------------------------------------
// Chassis background

void ClassicReverbUI::_drawChassisBackground(float margin, float rounding)
{
    const ImVec2 winPos  = ImGui::GetWindowPos();
    const ImVec2 winSize = ImGui::GetWindowSize();

    const ImVec2 panelMin = ImVec2(winPos.x + margin,             winPos.y + margin);
    const ImVec2 panelMax = ImVec2(winPos.x + winSize.x - margin, winPos.y + winSize.y - margin);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Soft drop-shadow (upper-left light source → shadow falls to bottom-right)
    static constexpr int   kShadowLayers = 6;
    static const float     kShadowMax    = SCALE(9.0f);
    for (int i = kShadowLayers; i >= 1; --i)
    {
        const float frac   = static_cast<float>(i) / kShadowLayers;
        const float offset = kShadowMax * frac;
        const int   alpha  = static_cast<int>(70.0f * (kShadowLayers - i + 1) / kShadowLayers);
        dl->AddRectFilled(
            ImVec2(panelMin.x + offset, panelMin.y + offset),
            ImVec2(panelMax.x + offset, panelMax.y + offset),
            IM_COL32(0, 0, 0, alpha), rounding);
    }

    // Main panel – base colour #a75957 (slightly different from RE-04, for distinction)
    dl->AddRectFilled(panelMin, panelMax, IM_COL32(0xa7, 0x59, 0x57, 0xff), rounding);

    // Subtle top-left highlight edge
    dl->AddRect(panelMin, panelMax, IM_COL32(0xff, 0xe0, 0xb8, 60), rounding, 0, 1.5f);
}

// -----------------------------------------------------------------------
// Kjaerhus logo

void ClassicReverbUI::_drawKjaerhusLogo(const ImVec2& size)
{
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetCursorScreenPos();

    // Invisible button acts as both the hitbox and the layout reservation
    if (ImGui::InvisibleButton("##Logo_Clickable", size))
        fAboutWindowOpened = true;

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Triangle (concave Bezier) layered behind the text
    {
        const float llen            = SCALE(40.0f);
        const float height          = SCALE(35.0f);
        const ImVec2 p1 = ImVec2(pos.x + SCALE(72.0f), pos.y - SCALE(1.0f));
        const ImVec2 p2 = ImVec2(p1.x, p1.y + llen);
        const ImVec2 p3 = ImVec2(p1.x + height, p1.y + llen * 0.5f);
        const float  ci = SCALE(10.0f);

        const ImVec2 ctrl_top = ImVec2((p1.x + p3.x) * 0.5f, (p1.y + p3.y) * 0.5f + ci);
        const ImVec2 ctrl_bot = ImVec2((p3.x + p2.x) * 0.5f, (p3.y + p2.y) * 0.5f - ci);

        dl->PathClear();
        dl->PathLineTo(p1);
        dl->PathBezierQuadraticCurveTo(ctrl_top, p3);
        dl->PathBezierQuadraticCurveTo(ctrl_bot, p2);
        dl->PathFillConcave(IM_COL32(255, 255, 255, 60));
    }

    // "ANCLARK STUDIO" text
    ImGuiExt::AddTextScaled(dl, ImGui::GetIO().Fonts->Fonts[2], SCALE(20.0f),
        ImVec2(pos.x + SCALE(10.0f), pos.y + SCALE(8.0f)),
        IM_COL32(255, 255, 255, 255),
        "ANCLARK STUDIO", 0.65f, 1.0f);

    // "Classic Series Reborn" badge
    {
        const char*    info_text   = "Classic Series Reborn";
        const float kFontSz    = SCALE(16.0f);
        const float kScaleX    = 0.8f;
        const float kScaleY    = 0.8f;
        const float kPadX      = SCALE(8.0f);
        const float kPadY      = SCALE(1.0f);
        const float kRound     = SCALE(3.0f);
        constexpr ImU32 kBgColor   = IM_COL32(100, 100, 100, 60);

        ImFont*       font     = ImGui::GetIO().Fonts->Fonts[2];
        const ImVec2  text_pos = ImVec2(pos.x + SCALE(10.0f), pos.y + SCALE(8.0f + 22.0f));
        const ImVec2  raw_sz   = font->CalcTextSizeA(kFontSz, FLT_MAX, 0.0f, info_text);
        const ImVec2  text_sz  = ImVec2(raw_sz.x * kScaleX, raw_sz.y * kScaleY);

        dl->AddRectFilled(
            ImVec2(text_pos.x - kPadX, text_pos.y - kPadY),
            ImVec2(text_pos.x + text_sz.x + kPadX, text_pos.y + text_sz.y + kPadY),
            kBgColor, kRound);

        ImGuiExt::AddTextScaled(dl, font, kFontSz, text_pos,
            IM_COL32(255, 255, 255, 255), info_text, kScaleX, kScaleY);
    }
}

// -----------------------------------------------------------------------
// Plugin name mark

void ClassicReverbUI::_drawPluginName()
{
    ImGui::BeginGroup();

    // "Classic Reverb" logotype
    ImGui::AlignTextToFramePadding();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
    ImGui::Dummy(ImVec2(0, SCALE(2)));
    ImGui::SameLine();
    ImGui::Text("Classic Reverb");
    ImGui::PopFont();

    ImGui::SameLine();

    // "RE-02" model capsule
    {
        ImDrawList*     dl      = ImGui::GetWindowDrawList();
        ImFont*         font    = ImGui::GetIO().Fonts->Fonts[2];
        const float     kFontSz = SCALE(12.5f);
        const float     kPadX   = SCALE(3.0f);
        const float     kPadY   = SCALE(2.0f);
        const float     kRound  = SCALE(4.0f);

        const ImVec2 re_sz  = font->CalcTextSizeA(kFontSz, FLT_MAX, 0.0f, "RE");
        const ImVec2 o2_sz  = font->CalcTextSizeA(kFontSz, FLT_MAX, 0.0f, "02");
        const float  height = re_sz.y + kPadY * 2.0f;
        const float  lw     = re_sz.x + kPadX * 2.0f;
        const float  rw     = o2_sz.x + kPadX * 2.0f;

        const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        const ImVec2 p0  = ImVec2(cursor_pos.x, cursor_pos.y + SCALE(4.0f));
        const ImVec2 mid = ImVec2(p0.x + lw,      p0.y);
        const ImVec2 p1  = ImVec2(p0.x + lw + rw, p0.y + height);

        // Right half – solid white fill
        dl->AddRectFilled(mid, p1, IM_COL32(255, 255, 255, 200), kRound, ImDrawFlags_RoundCornersRight);
        // Outer border – white
        dl->AddRect(p0, p1, IM_COL32(255, 255, 255, 255), kRound);
        // "RE" – white on transparent
        dl->AddText(font, kFontSz, ImVec2(p0.x + kPadX, p0.y + kPadY), IM_COL32(255, 255, 255, 255), "RE");
        // "02" – black on white
        dl->AddText(font, kFontSz, ImVec2(mid.x + kPadX, p0.y + kPadY), IM_COL32(0, 0, 0, 255), "02");

        ImGui::InvisibleButton("##RE02_info", ImVec2(lw + rw, height));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
        {
            ImGui::SetTooltip(
                "\"RE\" stands for Reverse Engineering.\n"
                "Classic Reverb RE is an open source recreation of the original Kjaerhus Classic Reverb.");
        }
    }

    ImGui::EndGroup();
}

// -----------------------------------------------------------------------
// Knob helper

void ClassicReverbUI::_addKnob(
    ClassicReverbParams paramId, const char* label,
    float v_min, float v_max,
    const ImGuiKnobs_Mod::KnobScaleMark* marks, uint32_t mark_count,
    bool isLogarithmic, bool use_pivot, float pivot_value)
{
    const float   kKnobSize  = SCALE(50.0f);
    constexpr int kStepCount = 10;

    constexpr float kPi       = 3.14159265358979323846f;
    constexpr float kAngleMin = kPi * (130.0f / 180.0f);
    constexpr float kAngleMax = kPi * (410.0f / 180.0f);

    ImGuiKnobFlags flags = ImGuiKnobFlags_TitleBottom;
    if (isLogarithmic) flags |= ImGuiKnobFlags_Logarithmic;
    if (use_pivot)     flags |= ImGuiKnobFlags_Pivot;

    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        IM_COL32(0x2f + 70, 0x4d + 70, 0x44 + 70, 0xff));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        IM_COL32(0x2f + 90, 0x4d + 90, 0x44 + 90, 0xff));

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);  // Chassis regular font

    if (ImGuiKnobs_Mod::Knob(label, &fParams[paramId], v_min, v_max,
                             0.0f, "%.1f", ImGuiKnobVariant_Tick, kKnobSize, flags,
                             kStepCount, kAngleMin, kAngleMax,
                             marks, mark_count, &kScaleMarkStyle, pivot_value))
    {
        setParameterValue(paramId, fParams[paramId]);
    }

    if (ImGui::IsItemActivated())
        editParameter(paramId, true);

    if (ImGui::IsItemDeactivated())
        editParameter(paramId, false);

    ImGui::PopFont();
    ImGui::PopStyleColor(2);
}

// -----------------------------------------------------------------------
// Section helpers

bool ClassicReverbUI::_BeginSection(const char* title, float width)
{
    ImGui::BeginGroup();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(255, 255, 255, 255));
    ImGuiExt::CenteredSeparatorText(title, width);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    // Gap between title and knobs – prevents scale marks from overlapping
    ImGui::Dummy(ImVec2(0, SCALE(8)));

    return true;
}

void ClassicReverbUI::_EndSection()
{
    ImGui::EndGroup();
}

// -----------------------------------------------------------------------
// Mouse cursor update

void ClassicReverbUI::_UpdateMouseCursor()
{
    ImGuiMouseCursor cur =
        ImGui::GetIO().MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (fLastMouseCursor == cur) return;

    fLastMouseCursor = cur;
    switch (cur)
    {
        case ImGuiMouseCursor_None:       getWindow().setCursor(MouseCursor::kMouseCursorArrow);          break;
        case ImGuiMouseCursor_Arrow:      getWindow().setCursor(MouseCursor::kMouseCursorArrow);          break;
        case ImGuiMouseCursor_TextInput:  getWindow().setCursor(MouseCursor::kMouseCursorCaret);          break;
        case ImGuiMouseCursor_ResizeAll:  getWindow().setCursor(MouseCursor::kMouseCursorCrosshair);      break;
        case ImGuiMouseCursor_ResizeNS:   getWindow().setCursor(MouseCursor::kMouseCursorUpDown);         break;
        case ImGuiMouseCursor_ResizeEW:   getWindow().setCursor(MouseCursor::kMouseCursorLeftRight);      break;
        case ImGuiMouseCursor_ResizeNESW: getWindow().setCursor(MouseCursor::kMouseCursorUpRightDownLeft);break;
        case ImGuiMouseCursor_ResizeNWSE: getWindow().setCursor(MouseCursor::kMouseCursorUpLeftDownRight);break;
        case ImGuiMouseCursor_Hand:       getWindow().setCursor(MouseCursor::kMouseCursorHand);           break;
        case ImGuiMouseCursor_NotAllowed: getWindow().setCursor(MouseCursor::kMouseCursorNotAllowed);     break;
        default:                          getWindow().setCursor(MouseCursor::kMouseCursorArrow);          break;
    }
}
