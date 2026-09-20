#pragma once
#include "UiTheme.h"
#include "Fonts.h"
#include "../Core/Vars/Vars.h"
#include "../Core/Globals/Globals.h"
#include "../Game/Offsets/Offsets.h"
#include <string>
#include <vector>

namespace Hud {

    inline void DrawChip(ImDrawList* dl, ImVec2& cursor, const char* label, ImU32 bg, ImU32 text) {
        ImFont* f = AppFonts::Small ? AppFonts::Small : ImGui::GetFont();
        float fs = AppFonts::FontSize(f, 13.f);
        ImVec2 ts = UiTheme::TextSize(f, fs, label);
        float padX = 9.f, padY = 4.f;
        ImVec2 a(cursor.x, cursor.y);
        ImVec2 b(a.x + ts.x + padX * 2.f, a.y + ts.y + padY * 2.f);
        dl->AddRectFilled(a, b, bg, 7.f);
        dl->AddRect(a, b, UiTheme::Col(255, 255, 255, 18), 7.f);
        dl->AddText(f, fs, ImVec2(a.x + padX, a.y + padY), text, label);
        cursor.x = b.x + 7.f;
    }

    inline void Render(int fps) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 display = ImGui::GetIO().DisplaySize;
        ImFont* titleFont = AppFonts::Bold ? AppFonts::Bold : ImGui::GetFont();
        ImFont* smallFont = AppFonts::Small ? AppFonts::Small : ImGui::GetFont();
        float titleSize = AppFonts::FontSize(titleFont, 15.f);
        float smallSize = AppFonts::FontSize(smallFont, 13.f);

        const float pad = 16.f;
        const float panelH = 44.f;

        const char* brand = "CHEAT GLOBAL";
        std::string fpsTxt = std::to_string(fps) + " OVR";
        std::string ver = offsets::RobloxVersion;
        if (ver.size() > 24) ver = ver.substr(0, 24) + "..";

        ImVec2 brandSz = UiTheme::TextSize(titleFont, titleSize, brand);
        ImVec2 fpsSz = UiTheme::TextSize(smallFont, smallSize, fpsTxt.c_str());
        ImVec2 verSz = UiTheme::TextSize(smallFont, smallSize, ver.c_str());

        float chipsW = 0.f;
        struct Chip { const char* label; ImU32 bg; };
        std::vector<Chip> chips;
        if (Vars::Aimbot::enabled) chips.push_back({ "AIM", UiTheme::Col(140, 110, 255, 70) });
        if (Vars::ESP::enabled) chips.push_back({ "ESP", UiTheme::Col(80, 170, 255, 70) });
        if (Vars::Chams::enabled) chips.push_back({ "CHM", UiTheme::Col(180, 110, 255, 70) });
        if (Vars::TriggerBot::enabled) chips.push_back({ "TRG", UiTheme::Col(255, 120, 80, 70) });
        if (Vars::BladeBall::enabled) chips.push_back({ "BB", UiTheme::Col(255, 200, 80, 70) });
        if (Vars::Local::flyEnabled) chips.push_back({ "FLY", UiTheme::Col(80, 220, 160, 70) });
        if (Vars::AutoClicker::enabled) chips.push_back({ "CLK", UiTheme::Col(255, 170, 90, 70) });

        for (const auto& c : chips) {
            ImVec2 ts = UiTheme::TextSize(smallFont, smallSize, c.label);
            chipsW += ts.x + 26.f;
        }
        if (!chips.empty()) chipsW += static_cast<float>(chips.size() - 1) * 7.f;

        float panelW = brandSz.x + fpsSz.x + verSz.x + chipsW + 96.f;
        panelW = (std::min)(panelW, display.x - pad * 2.f);
        float x = display.x - panelW - pad;
        float y = pad;
        ImVec2 a(x, y);
        ImVec2 b(x + panelW, y + panelH);

        UiTheme::DrawSoftShadow(dl, a, b, 12.f, 3);
        UiTheme::DrawGlassPanel(dl, a, b, 12.f, UiTheme::Col(8, 10, 16, 220), UiTheme::Col(140, 110, 255, 50));
        dl->AddRectFilledMultiColor(
            ImVec2(a.x, a.y), ImVec2(b.x, a.y + 2.f),
            UiTheme::Col(140, 110, 255, 190), UiTheme::Col(90, 160, 255, 190),
            UiTheme::Col(90, 160, 255, 190), UiTheme::Col(140, 110, 255, 190));

        float cx = a.x + 14.f;
        float cy = a.y + panelH * 0.5f;
        dl->AddCircleFilled(ImVec2(cx, cy), 5.f, UiTheme::Col(140, 110, 255, 255));
        dl->AddCircleFilled(ImVec2(cx, cy), 2.f, UiTheme::Col(250, 248, 255, 255));

        cx += 12.f;
        dl->AddText(titleFont, titleSize, ImVec2(cx, a.y + 12.f),
            UiTheme::Col(242, 244, 252, 255), brand);
        cx += brandSz.x + 16.f;

        ImU32 fpsCol = fps >= 55 ? UiTheme::Col(70, 220, 130, 255)
            : (fps >= 35 ? UiTheme::Col(240, 190, 60, 255) : UiTheme::Col(240, 80, 90, 255));
        dl->AddText(smallFont, smallSize, ImVec2(cx, a.y + 14.f), fpsCol, fpsTxt.c_str());
        cx += fpsSz.x + 14.f;

        dl->AddCircleFilled(ImVec2(cx, cy), 3.f,
            Globals::dataModel.Addr ? UiTheme::Col(70, 220, 130, 255) : UiTheme::Col(240, 80, 90, 255));
        cx += 10.f;
        dl->AddText(smallFont, smallSize, ImVec2(cx, a.y + 14.f),
            UiTheme::Col(150, 156, 172, 255), ver.c_str());
        cx += verSz.x + 12.f;

        ImVec2 chipCursor(cx, a.y + 12.f);
        for (const auto& c : chips)
            DrawChip(dl, chipCursor, c.label, c.bg, UiTheme::Col(235, 238, 248, 255));
    }
}
