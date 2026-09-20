#pragma once
#include "ImGui/imgui.h"
#include "Fonts.h"
#include <cmath>
#include <string>

namespace UiTheme {

    inline ImU32 Col(int r, int g, int b, int a = 255) {
        return IM_COL32(r, g, b, a);
    }

    inline ImU32 Alpha(ImU32 c, int a) {
        return (c & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a) << IM_COL32_A_SHIFT);
    }

    inline ImU32 LerpCol(ImU32 a, ImU32 b, float t) {
        t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
        int ar = (a >> IM_COL32_R_SHIFT) & 0xFF, ag = (a >> IM_COL32_G_SHIFT) & 0xFF;
        int ab = (a >> IM_COL32_B_SHIFT) & 0xFF, aa = (a >> IM_COL32_A_SHIFT) & 0xFF;
        int br = (b >> IM_COL32_R_SHIFT) & 0xFF, bg = (b >> IM_COL32_G_SHIFT) & 0xFF;
        int bb = (b >> IM_COL32_B_SHIFT) & 0xFF, ba = (b >> IM_COL32_A_SHIFT) & 0xFF;
        return IM_COL32(
            ar + static_cast<int>((br - ar) * t),
            ag + static_cast<int>((bg - ag) * t),
            ab + static_cast<int>((bb - ab) * t),
            aa + static_cast<int>((ba - aa) * t)
        );
    }

    inline float SmoothStep(float t) {
        t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
        return t * t * (3.f - 2.f * t);
    }

    inline float Animate(ImGuiID id, float target, float speed = 14.f) {
        float* v = ImGui::GetStateStorage()->GetFloatRef(id, target);
        float dt = ImGui::GetIO().DeltaTime;
        if (dt <= 0.f) dt = 0.016f;
        *v += (target - *v) * (1.f - expf(-speed * dt));
        if (fabsf(*v - target) < 0.001f) *v = target;
        return *v;
    }

    inline ImVec2 TextSize(ImFont* font, float size, const char* text) {
        if (font)
            return font->CalcTextSizeA(size, FLT_MAX, 0.f, text);
        return ImGui::CalcTextSize(text);
    }

    inline ImVec4 Accent()      { return ImVec4(0.58f, 0.44f, 1.00f, 1.f); }
    inline ImVec4 AccentHi()    { return ImVec4(0.74f, 0.62f, 1.00f, 1.f); }
    inline ImVec4 AccentBlue()  { return ImVec4(0.40f, 0.62f, 1.00f, 1.f); }
    inline ImVec4 PanelBg()     { return ImVec4(0.028f, 0.032f, 0.048f, 0.98f); }
    inline ImVec4 SidebarBg()   { return ImVec4(0.038f, 0.042f, 0.062f, 1.f); }
    inline ImVec4 CardBg()      { return ImVec4(0.055f, 0.060f, 0.085f, 0.98f); }
    inline ImVec4 CardHover()   { return ImVec4(0.070f, 0.076f, 0.105f, 1.f); }
    inline ImVec4 CardBorder()  { return ImVec4(0.20f, 0.22f, 0.32f, 0.65f); }
    inline ImVec4 TextMuted()   { return ImVec4(0.56f, 0.60f, 0.70f, 1.f); }
    inline ImVec4 TextBright()  { return ImVec4(0.97f, 0.98f, 1.00f, 1.f); }
    inline ImVec4 TextDim()     { return ImVec4(0.40f, 0.44f, 0.54f, 1.f); }
    inline ImVec4 Success()     { return ImVec4(0.35f, 0.92f, 0.58f, 1.f); }
    inline ImVec4 Warning()     { return ImVec4(0.98f, 0.76f, 0.28f, 1.f); }
    inline ImVec4 Danger()      { return ImVec4(0.98f, 0.38f, 0.45f, 1.f); }

    inline void DrawGlassPanel(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, ImU32 fill, ImU32 border) {
        dl->AddRectFilled(a, b, fill, rounding);
        dl->AddRect(a, b, border, rounding, 0, 1.f);
        dl->AddRectFilled(
            ImVec2(a.x + 1.f, a.y + 1.f), ImVec2(b.x - 1.f, a.y + 3.f),
            Col(255, 255, 255, 14), rounding, ImDrawFlags_RoundCornersTop);
    }

    inline void DrawAccentBar(ImDrawList* dl, ImVec2 a, float h, ImU32 c1, ImU32 c2) {
        dl->AddRectFilledMultiColor(a, ImVec2(a.x + 3.f, a.y + h), c1, c1, c2, c2);
    }

    inline void DrawSoftShadow(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, int layers = 4) {
        for (int i = layers; i >= 1; --i) {
            float expand = static_cast<float>(i) * 1.4f;
            int alpha = 14 + i * 5;
            dl->AddRectFilled(
                ImVec2(a.x - expand, a.y - expand + 3.f),
                ImVec2(b.x + expand, b.y + expand + 3.f),
                Col(0, 0, 0, alpha), rounding + expand);
        }
    }

    inline void DrawProgressBar(ImDrawList* dl, ImVec2 a, ImVec2 b, float t, ImU32 track, ImU32 fill) {
        t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
        dl->AddRectFilled(a, b, track, 3.f);
        if (t > 0.001f)
            dl->AddRectFilled(a, ImVec2(a.x + (b.x - a.x) * t, b.y), fill, 3.f);
    }

    inline void DrawDivider(ImDrawList* dl, ImVec2 a, float width) {
        dl->AddLine(a, ImVec2(a.x + width, a.y), Col(255, 255, 255, 16));
    }

    inline ImU32 TypeAccent(int type, int alpha = 255) {
        switch (type) {
            case 1: return Col(55, 210, 120, alpha);
            case 2: return Col(245, 190, 55, alpha);
            case 3: return Col(245, 75, 90, alpha);
            default: return Col(140, 110, 255, alpha);
        }
    }

    inline const char* TypeIcon(int type) {
        switch (type) {
            case 1: return "+";
            case 2: return "!";
            case 3: return "x";
            default: return "i";
        }
    }

    inline bool StyledButton(const char* label, ImVec2 size = ImVec2(-1, 34.f)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.11f, 0.16f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.14f, 0.28f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.18f, 0.38f, 1.f));
        bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        return pressed;
    }

    inline bool PrimaryButton(const char* label, ImVec2 size = ImVec2(-1, 34.f)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.17f, 0.42f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.24f, 0.52f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.28f, 0.60f, 1.f));
        bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        return pressed;
    }

    inline void SectionHeader(const char* title) {
        ImGui::Spacing();
        if (AppFonts::Bold) ImGui::PushFont(AppFonts::Bold);
        ImGui::TextColored(Accent(), "%s", title);
        if (AppFonts::Bold) ImGui::PopFont();
        ImVec2 p = ImGui::GetCursorScreenPos();
        DrawDivider(ImGui::GetWindowDrawList(), p, ImGui::GetContentRegionAvail().x);
        ImGui::Dummy(ImVec2(0, 4));
    }

    inline void HintText(const char* text) {
        ImGui::PushStyleColor(ImGuiCol_Text, TextDim());
        ImGui::TextWrapped("%s", text);
        ImGui::PopStyleColor();
    }
}
