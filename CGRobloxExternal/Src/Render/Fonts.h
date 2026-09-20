#pragma once
#include "ImGui/imgui.h"

namespace AppFonts {
    inline ImFont* Regular = nullptr;
    inline ImFont* Bold = nullptr;
    inline ImFont* Small = nullptr;
    inline ImFont* Title = nullptr;

    inline float FontSize(ImFont* font, float fallback = 16.f) {
        return font ? font->LegacySize : fallback;
    }

    inline void Init(ImGuiIO& io) {
        ImFontConfig cfg;
        cfg.OversampleH = 3;
        cfg.OversampleV = 3;
        cfg.PixelSnapH = true;

        Regular = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f, &cfg);
        Bold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 17.5f, &cfg);
        Small = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 13.0f, &cfg);
        Title = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 26.0f, &cfg);

        if (!Regular) Regular = io.Fonts->AddFontDefault();
        if (!Bold) Bold = Regular;
        if (!Small) Small = Regular;
        if (!Title) Title = Bold;

        io.FontDefault = Regular;
    }
}
