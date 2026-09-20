#pragma once
#include "ImGui/imgui.h"
#include "Fonts.h"
#include "UiTheme.h"
#include "Hud.h"
#include "../Core/Vars/Vars.h"
#include "../Core/Globals/Globals.h"
#include <string>
#include "../Core/Cache/Cache.h"
#include "../Core/Features/PlayerList/PlayerList.h"
#include "../Core/Features/MM2/MM2.h"
#include "../Core/Features/BladeBall/BladeBall.h"
#include "../Core/Features/Teleport/Teleport.h"
#include "../Core/Features/Misc/AutoClicker.h"
#include "../Core/Config/Config.h"
#include "../Game/Offsets/Offsets.h"
#include "../Game/Offsets/OffsetUpdater.h"
#include "../Core/Features/Misc/Notifications.h"
#include <cmath>
#include <algorithm>
#include <cctype>

namespace MenuUI {

    inline ImVec4 Accent()      { return UiTheme::Accent(); }
    inline ImVec4 AccentSoft()  { return ImVec4(0.22f, 0.18f, 0.42f, 1.f); }
    inline ImVec4 PanelBg()     { return UiTheme::PanelBg(); }
    inline ImVec4 CardBg()      { return UiTheme::CardBg(); }
    inline ImVec4 CardBorder()  { return UiTheme::CardBorder(); }
    inline ImVec4 TextMuted()   { return UiTheme::TextMuted(); }
    inline ImVec4 TextBright()  { return UiTheme::TextBright(); }

    inline float gCardW = 290.f;
    inline float gCardH = 400.f;
    inline float gCardGap = 14.f;
    inline float gToggleX = 0.f;
    inline int gGridColumns = 2;
    inline int gGridColumn = 0;
    inline ImVec2 gGridOrigin{};

    inline void ApplyStyle() {
        ImGuiStyle& s = ImGui::GetStyle();
        ImVec4* c = s.Colors;

        s.WindowRounding = 18.0f;
        s.ChildRounding = 14.0f;
        s.FrameRounding = 10.0f;
        s.PopupRounding = 12.0f;
        s.ScrollbarRounding = 10.0f;
        s.GrabRounding = 10.0f;
        s.WindowPadding = ImVec2(0, 0);
        s.FramePadding = ImVec2(12, 6);
        s.ItemSpacing = ImVec2(10, 6);
        s.ItemInnerSpacing = ImVec2(8, 6);
        s.ScrollbarSize = 8.0f;
        s.WindowBorderSize = 0.0f;
        s.ChildBorderSize = 1.0f;
        s.Alpha = 1.0f;

        c[ImGuiCol_Text] = TextBright();
        c[ImGuiCol_TextDisabled] = TextMuted();
        c[ImGuiCol_WindowBg] = PanelBg();
        c[ImGuiCol_ChildBg] = CardBg();
        c[ImGuiCol_Border] = CardBorder();
        c[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.08f, 0.12f, 1.f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.11f, 0.16f, 1.f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.14f, 0.12f, 0.24f, 1.f);
        c[ImGuiCol_SliderGrab] = Accent();
        c[ImGuiCol_SliderGrabActive] = UiTheme::AccentHi();
        c[ImGuiCol_CheckMark] = Accent();
        c[ImGuiCol_Button] = ImVec4(0.08f, 0.09f, 0.13f, 1.f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.11f, 0.22f, 1.f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.15f, 0.32f, 1.f);
        c[ImGuiCol_Header] = ImVec4(0.11f, 0.10f, 0.20f, 1.f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.13f, 0.28f, 1.f);
        c[ImGuiCol_HeaderActive] = Accent();
        c[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.05f, 0.08f, 0.98f);
        c[ImGuiCol_Separator] = ImVec4(0.20f, 0.22f, 0.30f, 0.55f);
        c[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.04f, 0.06f, 0.6f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.18f, 0.34f, 0.9f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.24f, 0.46f, 1.f);
        c[ImGuiCol_ScrollbarGrabActive] = Accent();
        c[ImGuiCol_Tab] = ImVec4(0.07f, 0.08f, 0.12f, 1.f);
        c[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.11f, 0.22f, 1.f);
        c[ImGuiCol_TabActive] = ImVec4(0.17f, 0.14f, 0.30f, 1.f);
    }

    inline bool ToggleSwitch(const char* id, bool* v) {
        ImGui::PushID(id);
        ImVec2 p = ImGui::GetCursorScreenPos();
        const float h = 22.f, w = 42.f;
        ImGui::InvisibleButton("##t", ImVec2(w, h));
        if (ImGui::IsItemClicked()) *v = !*v;

        ImGuiID aid = ImGui::GetID("##anim");
        float t = UiTheme::Animate(aid, *v ? 1.f : 0.f, 20.f);
        bool hovered = ImGui::IsItemHovered();
        bool held = ImGui::IsItemActive();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImU32 trackOff = UiTheme::Col(36, 40, 52, 255);
        ImU32 trackOn = UiTheme::Col(140, 110, 255, 255);
        ImU32 track = UiTheme::LerpCol(trackOff, trackOn, t);
        if (hovered) track = UiTheme::LerpCol(track, UiTheme::Col(170, 140, 255, 255), 0.18f);

        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), track, h * 0.5f);
        dl->AddRect(p, ImVec2(p.x + w, p.y + h), UiTheme::Col(255, 255, 255, hovered ? 28 : 14), h * 0.5f);

        float knobX = p.x + 11.f + t * (w - 22.f);
        float knobR = held ? 8.f : 8.5f;
        dl->AddCircleFilled(ImVec2(knobX, p.y + h * 0.5f), knobR, UiTheme::Col(252, 252, 255, 255));
        dl->AddCircleFilled(ImVec2(knobX, p.y + h * 0.5f), 4.5f, UiTheme::Col(220, 215, 255, 160));

        ImGui::PopID();
        return false;
    }

    inline void BeginCardGrid(float rowHeight = -1.f, int columns = 2) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        gGridColumns = (std::max)(1, columns);
        gGridColumn = 0;
        gCardW = (avail.x - gCardGap * static_cast<float>(gGridColumns - 1)) / static_cast<float>(gGridColumns);
        gCardH = (rowHeight > 0.f) ? rowHeight : avail.y;
        gGridOrigin = ImGui::GetCursorPos();
        gToggleX = gCardW - 62.f;
    }

    inline void NextCardColumn() {
        gGridColumn++;
        if (gGridColumn >= gGridColumns) return;
        ImGui::SetCursorPos(ImVec2(
            gGridOrigin.x + static_cast<float>(gGridColumn) * (gCardW + gCardGap),
            gGridOrigin.y));
    }

    inline void EndCardGrid() {
        gGridColumn = 0;
    }

    inline bool BeginModuleCard(const char* id, const char* title, bool* masterToggle) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, CardBg());
        ImGui::PushStyleColor(ImGuiCol_Border, CardBorder());
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 14));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 7.f);

        bool open = ImGui::BeginChild(
            id,
            ImVec2(gCardW, gCardH),
            true,
            ImGuiWindowFlags_None
        );

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cardMin = ImGui::GetWindowPos();
        UiTheme::DrawAccentBar(dl, cardMin, 52.f,
            UiTheme::Col(140, 110, 255, 255), UiTheme::Col(90, 160, 255, 255));

        if (AppFonts::Bold) ImGui::PushFont(AppFonts::Bold);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.f);
        ImGui::TextColored(TextBright(), "%s", title);
        if (AppFonts::Bold) ImGui::PopFont();

        if (masterToggle) {
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 46.f);
            ToggleSwitch((std::string(id) + "_sw").c_str(), masterToggle);
        }

        UiTheme::DrawDivider(dl, ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail().x);
        ImGui::Dummy(ImVec2(0, 8));
        return open;
    }

    inline void EndModuleCard() {
        ImGui::EndChild();
        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor(2);
    }

    inline void LabelRow(const char* label) {
        ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
    }

    inline void RowToggle(const char* id, const char* label, bool* v) {
        LabelRow(label);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 42.f);
        ToggleSwitch(id, v);
    }

    inline void RowSlider(const char* label, float* v, float lo, float hi, const char* fmt) {
        LabelRow(label);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.07f, 0.10f, 1.f));
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat((std::string("##") + label).c_str(), v, lo, hi, fmt);
        ImGui::PopStyleColor();
    }

    inline void RowCombo(const char* id, const char* label, int* idx, const char* const* items, int count) {
        LabelRow(label);
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo(id, idx, items, count);
    }

    inline void RowColor3(const char* id, const char* label, float* rgb) {
        LabelRow(label);
        ImGui::SetNextItemWidth(-1);
        ImGui::ColorEdit3(id, rgb, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    }

    inline void RowTogglePair(const char* id1, const char* label1, bool* v1,
                              const char* id2, const char* label2, bool* v2) {
        const float contentMax = ImGui::GetWindowContentRegionMax().x;
        const float contentMin = ImGui::GetWindowContentRegionMin().x;
        const float fullW = contentMax - contentMin;
        const float halfW = fullW * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
        ImGui::TextUnformatted(label1);
        ImGui::PopStyleColor();
        ImGui::SameLine(contentMin + halfW - 46.f);
        ToggleSwitch(id1, v1);

        ImGui::SameLine(contentMin + halfW + 8.f);
        ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
        ImGui::TextUnformatted(label2);
        ImGui::PopStyleColor();
        ImGui::SameLine(contentMax - 42.f);
        ToggleSwitch(id2, v2);
    }

    inline void RowKeyCombo(const char* id, const char* label, int* key,
        const char* const* names, const int* values, int count)
    {
        LabelRow(label);
        ImGui::SetNextItemWidth(-1);
        int idx = 0;
        for (int i = 0; i < count; i++)
            if (values[i] == *key) { idx = i; break; }
        if (ImGui::Combo(id, &idx, names, count))
            *key = values[idx];
    }

    inline bool SidebarItem(const char* label, const char* subtitle, int idx) {
        bool active = Vars::selectedTab == idx;
        ImVec2 sz(ImGui::GetContentRegionAvail().x - 4.f, 48.f);
        ImGui::PushID(idx);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.09f, 0.16f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.12f, 0.22f, 1.f));
        bool click = ImGui::Button("##navbtn", sz);
        bool hovered = ImGui::IsItemHovered();
        ImVec2 rmin = ImGui::GetItemRectMin();
        ImVec2 rmax = ImGui::GetItemRectMax();
        ImGui::PopStyleColor(3);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiID aid = ImGui::GetID("##navanim");
        float t = UiTheme::Animate(aid, active ? 1.f : (hovered ? 0.4f : 0.f), 16.f);

        if (t > 0.01f) {
            dl->AddRectFilled(rmin, rmax, UiTheme::Col(140, 110, 255, static_cast<int>(42.f * t)), 12.f);
            dl->AddRectFilled(ImVec2(rmin.x, rmin.y + 10.f), ImVec2(rmin.x + 3.f, rmax.y - 10.f),
                UiTheme::Col(140, 110, 255, static_cast<int>(255.f * t)), 2.f);
        }

        ImU32 textCol = active ? UiTheme::Col(248, 249, 255, 255)
            : UiTheme::LerpCol(UiTheme::Col(145, 150, 168, 255), UiTheme::Col(225, 228, 240, 255), t);
        ImU32 subCol = UiTheme::LerpCol(UiTheme::Col(90, 96, 112, 255), UiTheme::Col(165, 172, 188, 255), t);

        ImFont* navBold = AppFonts::Bold ? AppFonts::Bold : ImGui::GetFont();
        ImFont* navSmall = AppFonts::Small ? AppFonts::Small : ImGui::GetFont();
        dl->AddText(navBold, AppFonts::FontSize(navBold, 16.f), ImVec2(rmin.x + 14.f, rmin.y + 9.f), textCol, label);
        dl->AddText(navSmall, AppFonts::FontSize(navSmall, 13.f), ImVec2(rmin.x + 14.f, rmin.y + 28.f), subCol, subtitle);

        ImGui::PopID();
        if (click) Vars::selectedTab = idx;
        return click;
    }

    inline void RenderCombatTab() {
        BeginCardGrid(-1.f, 3);

        if (BeginModuleCard("##aim", "Aimbot", &Vars::Aimbot::enabled)) {
            const char* keys[] = { "None","LMB","RMB","MMB","X1","X2","Shift","Ctrl","Alt",
                "C","V","X","Z","Q","E","R","T","F","G" };
            const int kv[] = { 0,1,2,4,5,6,16,17,18,0x43,0x56,0x58,0x5A,0x51,0x45,0x52,0x54,0x46,0x47 };
            const char* bones[] = { "Head", "HumanoidRootPart" };
            RowKeyCombo("##kb", "Aim Key", &Vars::Aimbot::aimbotKey, keys, kv, 19);
            RowCombo("##bone", "Aim Bone", &Vars::Aimbot::aimTarget, bones, 2);

            RowSlider("FOV", &Vars::Aimbot::fovRadius, 10.f, 500.f, "%.0f");
            RowSlider("Smooth", &Vars::Aimbot::smoothing, 1.f, 20.f, "%.1f");
            RowToggle("fov_d", "Draw FOV", &Vars::Aimbot::showFOV);

            ImGui::Spacing();
            UiTheme::HintText("Hold the selected key to lock the closest target in your FOV.");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##trigger", "Trigger Bot", &Vars::TriggerBot::enabled)) {
            const char* parts[] = { "Head", "Torso", "All Parts" };
            RowCombo("##tpart", "Hit Part", &Vars::TriggerBot::hitPart, parts, 3);
            const char* tkeys[] = { "Always","LMB","RMB","MMB","X1","X2","Shift","Ctrl","Alt" };
            const int tkv[] = { 0,1,2,4,5,6,16,17,18 };
            RowKeyCombo("##tkb", "Trigger Key", &Vars::TriggerBot::triggerKey, tkeys, tkv, 9);
            RowSlider("Radius", &Vars::TriggerBot::triggerRadius, 4.f, 40.f, "%.0f px");
            if (Vars::TriggerBot::hitPart == 2)
                RowSlider("Hitbox Pad", &Vars::TriggerBot::hitboxPad, 0.f, 8.f, "%.1f");
            RowSlider("Delay", &Vars::TriggerBot::delayMs, 0.f, 200.f, "%.0f ms");
            RowSlider("Cooldown", &Vars::TriggerBot::cooldownMs, 30.f, 300.f, "%.0f ms");
            RowSlider("Max Dist", &Vars::TriggerBot::maxDistance, 50.f, 800.f, "%.0f");
            RowTogglePair("tmtm", "Team Check", &Vars::TriggerBot::teamCheck,
                          "tdead", "Skip Dead", &Vars::TriggerBot::ignoreDead);
            RowToggle("tfire", "Skip While Firing", &Vars::TriggerBot::skipWhileFiring);

            UiTheme::HintText("Fires when your cursor enters a valid player hitbox.");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##bb", "Blade Ball Auto Parry", &Vars::BladeBall::enabled)) {
            RowTogglePair("bbpro", "Pro Mode", &Vars::BladeBall::proMode,
                          "bbburst", "Burst", &Vars::BladeBall::burstParry);
            RowTogglePair("bbhl", "Strict Aim", &Vars::BladeBall::requireHighlight,
                          "bbclk", "Dual In", &Vars::BladeBall::dualInput);

            RowSlider("Parry Dist", &Vars::BladeBall::parryDistance, 3.f, 60.f, "%.0f");
            RowSlider("Parry Time", &Vars::BladeBall::parryTiming, 0.05f, 0.4f, "%.2fs");
            RowSlider("Ping Adj", &Vars::BladeBall::pingCompensation, 0.f, 0.15f, "%.3f");
            RowSlider("Timing Scale", &Vars::BladeBall::timingScale, 0.6f, 1.5f, "%.2f");
            RowSlider("Track Dist", &Vars::BladeBall::maxTrackDistance, 40.f, 120.f, "%.0f");

            ImGui::Spacing();
            ImVec2 statusMin = ImGui::GetCursorScreenPos();
            float statusW = ImGui::GetContentRegionAvail().x;
            ImGui::GetWindowDrawList()->AddRectFilled(statusMin,
                ImVec2(statusMin.x + statusW, statusMin.y + 72.f),
                IM_COL32(20, 22, 31, 255), 8.f);
            ImGui::SetCursorScreenPos(ImVec2(statusMin.x + 10.f, statusMin.y + 8.f));
            ImGui::TextColored(Accent(), "LIVE STATUS");

            if (Vars::BladeBall::enabled) {
                if (!BladeBall::inArena) {
                    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.f), "Lobby / Spawn");
                    ImGui::TextColored(TextMuted(), "Waiting for round...");
                } else {
                    ImGui::TextColored(TextMuted(), "%s", BladeBall::statusText.c_str());
                    if (BladeBall::ballDetected) {
                        ImGui::TextColored(ImVec4(0.55f, 0.90f, 1.f, 1.f),
                            "%.0f speed  |  %.0fm  |  %.3fs",
                            BladeBall::trackedSpeed, BladeBall::trackedDistance, BladeBall::trackedTimeToHit);
                    }
                }
            } else {
                ImGui::TextColored(TextMuted(), "Module disabled");
            }
        }
        EndModuleCard();

        EndCardGrid();
    }

    inline void RenderVisualsTab() {
        BeginCardGrid(-1.f, 3);

        if (BeginModuleCard("##esp", "Player ESP", &Vars::ESP::enabled)) {
            RowTogglePair("box", "Boxes", &Vars::ESP::boxes,
                          "skel", "Skeleton", &Vars::ESP::skeleton);
            RowTogglePair("trace", "Tracers", &Vars::ESP::tracers,
                          "name", "Names", &Vars::ESP::names);
            RowTogglePair("dist", "Distance", &Vars::ESP::distance,
                          "hp", "Health", &Vars::ESP::healthBar);
            if (Vars::ESP::tracers) {
                const char* origins[] = { "Bottom", "Center", "Top" };
                RowCombo("##trorg", "Tracer From", &Vars::ESP::tracerOrigin, origins, 3);
            }
            RowSlider("Pos Smooth", &Vars::ESP::smoothSpeed, 0.f, 40.f, "%.0f");
            RowSlider("Box Width", &Vars::ESP::boxWidthRatio, 0.22f, 0.38f, "%.2f");
            RowSlider("Box Pad", &Vars::ESP::boxPadding, 0.f, 3.f, "%.1f");
            RowToggle("stab", "Anti-Shake", &Vars::ESP::stableBox);
            if (Vars::ESP::stableBox) {
                RowSlider("Stabilize", &Vars::ESP::screenStabilize, 0.2f, 1.5f, "%.2f");
                RowSlider("Response", &Vars::ESP::screenBeta, 0.05f, 0.25f, "%.2f");
            }
            UiTheme::HintText("R6 / R15 body tracking with stabilized screen boxes.");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##chams", "Chams", &Vars::Chams::enabled)) {
            RowTogglePair("chfill", "Fill", &Vars::Chams::fill,
                          "chout", "Outline", &Vars::Chams::outline);
            RowTogglePair("chglow", "Glow", &Vars::Chams::glow,
                          "chmm2", "MM2 Colors", &Vars::Chams::teamColor);
            RowToggle("chroot", "Show Root", &Vars::Chams::showRoot);
            RowSlider("Opacity", &Vars::Chams::opacity, 0.10f, 0.90f, "%.2f");
            float col[3] = { Vars::Chams::colorR, Vars::Chams::colorG, Vars::Chams::colorB };
            RowColor3("##chc", "Color", col);
            Vars::Chams::colorR = col[0];
            Vars::Chams::colorG = col[1];
            Vars::Chams::colorB = col[2];

            UiTheme::HintText("Full 3D body-part chams with back-face culling and depth sorting.");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##mm2", "Murder Mystery 2", &Vars::MM2::enabled)) {
            RowToggle("mm2esp", "Role ESP", &Vars::MM2::roleESP);
            RowToggle("mm2sh", "Show Sheriff", &Vars::MM2::showSheriff);
            RowToggle("mm2hud", "Murderer HUD", &Vars::MM2::murdererHUD);

            ImGui::Spacing();
            ImGui::TextColored(Accent(), "ROLE DETECTION");
            if (Vars::MM2::enabled) {
                if (!MM2::murdererName.empty())
                    ImGui::TextColored(ImVec4(1.f, 0.38f, 0.45f, 1.f), "Murderer  %s", MM2::murdererName.c_str());
                else
                    ImGui::TextColored(TextMuted(), "Murderer  Unknown");

                if (Vars::MM2::showSheriff) {
                    if (!MM2::sheriffName.empty())
                        ImGui::TextColored(ImVec4(0.45f, 0.72f, 1.f, 1.f), "Sheriff   %s", MM2::sheriffName.c_str());
                    else
                        ImGui::TextColored(TextMuted(), "Sheriff   Unknown");
                }
            } else {
                ImGui::TextColored(TextMuted(), "Enable module to scan roles.");
            }

            ImGui::Spacing();
            ImGui::TextColored(TextMuted(), "Scans backpack and character");
            ImGui::TextColored(TextMuted(), "for Knife / Gun tools.");
        }
        EndModuleCard();

        EndCardGrid();
    }

    inline void RenderLocalTab() {
        const float rowH = (ImGui::GetContentRegionAvail().y - gCardGap) * 0.5f;

        BeginCardGrid(rowH, 2);

        if (BeginModuleCard("##movement", "Movement", nullptr)) {
            RowToggle("walksw", "WalkSpeed", &Vars::Local::speedEnabled);
            RowSlider("Speed", &Vars::Local::walkSpeed, 16.f, 200.f, "%.0f");
            ImGui::Spacing();

            RowToggle("jumpsw", "JumpPower", &Vars::Local::jumpEnabled);
            RowSlider("Power", &Vars::Local::jumpPower, 50.f, 200.f, "%.0f");
            ImGui::Spacing();

            RowToggle("flysw", "Fly", &Vars::Local::flyEnabled);
            RowSlider("Fly Speed", &Vars::Local::flySpeed, 10.f, 300.f, "%.0f");
            RowToggle("noclipsw", "Noclip", &Vars::Local::noclipEnabled);

            ImGui::Spacing();
            ImGui::TextColored(Accent(), "FLY CONTROLS");
            ImGui::TextColored(TextMuted(), "WASD  move");
            ImGui::TextColored(TextMuted(), "Space  up");
            ImGui::TextColored(TextMuted(), "Ctrl / Shift  down");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##autoclick", "Auto Clicker", &Vars::AutoClicker::enabled)) {
            const char* holdKeys[] = {
                "Always", "LMB", "RMB", "MMB", "X1", "X2",
                "Shift", "Ctrl", "Alt", "E", "Q", "R", "F", "G", "C", "V"
            };
            const int holdVals[] = {
                0, 1, 2, 4, 5, 6,
                16, 17, 18, 0x45, 0x51, 0x52, 0x46, 0x47, 0x43, 0x56
            };
            RowKeyCombo("##achold", "Activation", &Vars::AutoClicker::holdKey, holdKeys, holdVals, 16);

            LabelRow("Mouse Button");
            const char* buttons[] = { "Left Click", "Right Click" };
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##acbtn", &Vars::AutoClicker::clickButton, buttons, 2);

            RowToggle("acfocus", "Roblox Focus Only", &Vars::AutoClicker::onlyFocused);
            RowSlider("Max CPS", reinterpret_cast<float*>(&Vars::AutoClicker::maxCps), 20.f, 300.f, "%.0f");

            ImGui::Spacing();
            UiTheme::SectionHeader("LIVE STATUS");
            if (Vars::AutoClicker::enabled) {
                if (AutoClicker::isActive.load()) {
                    ImGui::TextColored(ImVec4(0.35f, 0.92f, 0.58f, 1.f),
                        "Clicking  |  %d CPS", AutoClicker::clicksPerSecond.load());
                } else if (Vars::AutoClicker::onlyFocused && !AutoClicker::IsRobloxFocused()) {
                    ImGui::TextColored(ImVec4(0.98f, 0.76f, 0.28f, 1.f), "Waiting for Roblox focus");
                } else if (Vars::AutoClicker::holdKey != 0) {
                    ImGui::TextColored(TextMuted(), "Hold activation key to click");
                } else {
                    ImGui::TextColored(TextMuted(), "Ready - zero delay turbo");
                }
            } else {
                ImGui::TextColored(TextMuted(), "Module disabled");
            }

            UiTheme::HintText("Paced turbo clicks — high CPS without freezing Roblox or your PC.");
        }
        EndModuleCard();

        EndCardGrid();

        ImGui::Dummy(ImVec2(0, gCardGap));
        BeginCardGrid(rowH, 2);

        if (BeginModuleCard("##utility", "Utility & System", nullptr)) {
            RowToggle("afksw", "Anti-AFK", &Vars::Misc::antiAfkEnabled);
            RowSlider("Interval", &Vars::Misc::antiAfkInterval, 20.f, 180.f, "%.0fs");
            RowToggle("afkntf", "Pulse Notify", &Vars::Misc::antiAfkNotify);

            ImGui::Spacing();
            RowToggle("notifysw", "Notifications", &Vars::Misc::notificationsEnabled);

            ImGui::Spacing();
            ImGui::TextColored(Accent(), "NOTIFICATION SOURCES");
            ImGui::TextColored(TextMuted(), "MM2 role detection");
            ImGui::TextColored(TextMuted(), "Blade Ball auto parry");
            ImGui::TextColored(TextMuted(), "Game reattach");
            ImGui::TextColored(TextMuted(), "Anti-AFK pulse");

            ImGui::Spacing();
            ImGui::TextColored(Accent(), "STATUS");
            ImGui::TextColored(TextMuted(), Vars::Misc::antiAfkEnabled
                ? "Idle protection active" : "Idle protection disabled");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##tpmain", "Teleport Tools", nullptr)) {
            if (ImGui::Button("Stage End TP", ImVec2(-1, 34)))
                TeleportFeatures::StageEndTP();

            RowSlider("Stage Height", &Vars::Teleport::stageEndOffsetY, 0.f, 15.f, "%.1f");
            ImGui::TextColored(TextMuted(), "%s", TeleportFeatures::stageStatus.c_str());

            ImGui::Spacing();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 sep = ImGui::GetCursorScreenPos();
            dl->AddLine(sep, ImVec2(sep.x + gCardW - 28.f, sep.y), IM_COL32(40, 42, 50, 200));
            ImGui::Dummy(ImVec2(0, 6));

            RowToggle("ctp", "Click TP", &Vars::Teleport::clickTpEnabled);
            const char* modes[] = { "Ray Distance", "Part Snap" };
            RowCombo("##ctpmode", "Click Mode", &Vars::Teleport::clickTpMode, modes, 2);
            const char* tkeys[] = { "Always","LMB","RMB","MMB","X1","X2","Shift","Ctrl","Alt" };
            const int tkv[] = { 0,1,2,4,5,6,16,17,18 };
            RowKeyCombo("##ctpkey", "TP Key + LMB", &Vars::Teleport::clickTpKey, tkeys, tkv, 9);
            if (Vars::Teleport::clickTpMode == 0)
                RowSlider("Ray Dist", &Vars::Teleport::clickTpDistance, 50.f, 800.f, "%.0f");
            else {
                RowSlider("Part Radius", &Vars::Teleport::clickPartRadius, 8.f, 60.f, "%.0f px");
                RowSlider("Max Scan", &Vars::Teleport::clickTpMaxScan, 100.f, 1500.f, "%.0f");
            }
            RowSlider("TP Height", &Vars::Teleport::clickTpOffsetY, 0.f, 15.f, "%.1f");
            RowToggle("ctpntf", "Notify", &Vars::Teleport::clickTpNotify);

            ImGui::Spacing();
            ImGui::TextColored(Accent(), "CLICK TP STATUS");
            ImGui::TextColored(TextMuted(), "%s", TeleportFeatures::clickStatus.c_str());
        }
        EndModuleCard();

        EndCardGrid();
    }

    inline bool PlayerMatchesSearch(const PlayerCache::CachedPlayer& plr) {
        if (Vars::Players::searchBuf[0] == '\0') return true;
        std::string name = plr.name;
        std::string query = Vars::Players::searchBuf;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
        return name.find(query) != std::string::npos;
    }

    inline void RenderPlayersTab() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        const float gap = 12.f;
        const float leftW = (avail.x - gap) * 0.58f;
        const float rightW = avail.x - gap - leftW;
        const float h = avail.y;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, CardBg());
        ImGui::PushStyleColor(ImGuiCol_Border, CardBorder());
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));

        ImGui::BeginChild("##plist", ImVec2(leftW, h), true);

        if (AppFonts::Bold) ImGui::PushFont(AppFonts::Bold);
        ImGui::TextColored(Accent(), "Player List");
        if (AppFonts::Bold) ImGui::PopFont();
        ImGui::TextColored(TextMuted(), "%d online", static_cast<int>(PlayerCache::players.size()));

        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##search", "Search player...", Vars::Players::searchBuf, sizeof(Vars::Players::searchBuf));

        ImGui::Spacing();
        float listH = ImGui::GetContentRegionAvail().y;
        float listW = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("##plist_scroll", ImVec2(listW, listH), false);

        ImFont* listFont = AppFonts::Regular ? AppFonts::Regular : ImGui::GetFont();

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid || !PlayerMatchesSearch(plr)) continue;

            bool selected = (Vars::Players::selectedAddr == plr.playerAddr);
            ImGui::PushID(static_cast<int>(plr.playerAddr & 0x7FFFFFFF));

            std::string label = plr.name;
            if (Vars::MM2::enabled) {
                if (plr.mm2Role == static_cast<int>(MM2::Role::Murderer)) label += "  [MURDERER]";
                else if (plr.mm2Role == static_cast<int>(MM2::Role::Sheriff)) label += "  [SHERIFF]";
            }
            label += "  |  " + std::to_string(static_cast<int>(plr.distance)) + "m";
            if (plr.maxHealth > 0 && plr.maxHealth <= 1000)
                label += "  |  " + std::to_string(plr.health) + "/" + std::to_string(plr.maxHealth);

            const float rowH = 32.f;
            ImVec2 rowSize(listW, rowH);
            bool hovered = false;
            if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_None, rowSize)) {
                Vars::Players::selectedAddr = plr.playerAddr;
            }
            hovered = ImGui::IsItemHovered();

            ImVec2 rmin = ImGui::GetItemRectMin();
            ImVec2 rmax = ImGui::GetItemRectMax();
            ImDrawList* pdl = ImGui::GetWindowDrawList();
            if (selected) {
                pdl->AddRectFilled(rmin, rmax, UiTheme::Col(130, 100, 255, 35), 8.f);
                pdl->AddRectFilled(ImVec2(rmin.x, rmin.y + 6.f), ImVec2(rmin.x + 3.f, rmax.y - 6.f),
                    UiTheme::Col(130, 100, 255, 255), 2.f);
            } else if (hovered) {
                pdl->AddRectFilled(rmin, rmax, UiTheme::Col(255, 255, 255, 8), 8.f);
            }

            ImU32 textCol = selected ? UiTheme::Col(255, 120, 130, 255) : UiTheme::Col(225, 228, 238, 255);
            float textY = rmin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f;
            if (listFont) ImGui::PushFont(listFont);
            ImGui::GetWindowDrawList()->AddText(ImVec2(rmin.x + 8.f, textY), textCol, label.c_str());
            if (listFont) ImGui::PopFont();

            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0, gap);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, CardBg());
        ImGui::PushStyleColor(ImGuiCol_Border, CardBorder());
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));
        ImGui::BeginChild("##pactions", ImVec2(rightW, h), true);

        gCardW = rightW;
        gToggleX = rightW - 58.f;

        if (AppFonts::Bold) ImGui::PushFont(AppFonts::Bold);
        ImGui::TextColored(TextBright(), "Player Actions");
        if (AppFonts::Bold) ImGui::PopFont();
        ImGui::Spacing();

        const PlayerCache::CachedPlayer* sel = nullptr;
        for (auto& plr : PlayerCache::players) {
            if (plr.isValid && plr.playerAddr == Vars::Players::selectedAddr) {
                sel = &plr;
                break;
            }
        }

        if (sel) {
            ImGui::TextColored(TextBright(), "%s", sel->name.c_str());
            if (Vars::MM2::enabled) {
                if (sel->mm2Role == static_cast<int>(MM2::Role::Murderer))
                    ImGui::TextColored(ImVec4(1.f, 0.35f, 0.4f, 1.f), "Role: MURDERER");
                else if (sel->mm2Role == static_cast<int>(MM2::Role::Sheriff))
                    ImGui::TextColored(ImVec4(0.4f, 0.65f, 1.f, 1.f), "Role: SHERIFF");
                else
                    ImGui::TextColored(TextMuted(), "Role: Innocent");
            }
            ImGui::TextColored(TextMuted(), "Distance: %dm", static_cast<int>(sel->distance));
            ImGui::TextColored(TextMuted(), "Health: %d / %d", sel->health, sel->maxHealth);
            ImGui::TextColored(TextMuted(), "Team: %s",
                PlayerListFeature::GetPlayerTeamName(sel->playerAddr).c_str());
            ImGui::Spacing();

            if (UiTheme::PrimaryButton("Teleport", ImVec2(-1, 36)))
                PlayerListFeature::TeleportToCached(*sel);

            if (UiTheme::StyledButton("Bring to Me", ImVec2(-1, 36)))
                PlayerListFeature::BeginBringCached(*sel);

            if (UiTheme::StyledButton("Copy Name", ImVec2(-1, 36)))
                PlayerListFeature::CopyToClipboard(sel->name);

            ImGui::Spacing();
            RowToggle("holdbring", "Hold Bring", &Vars::Players::bringHoldActive);
            RowSlider("TP Height", &Vars::Players::teleportOffsetY, 0.f, 15.f, "%.1f");
            RowSlider("Bring Side", &Vars::Players::bringSideOffset, 1.f, 12.f, "%.1f");
            RowSlider("Bring Height", &Vars::Players::bringOffsetY, -5.f, 10.f, "%.1f");
        } else {
            ImGui::TextColored(TextMuted(), "Select a player from the list.");
        }

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    inline void RenderConfigTab() {
        BeginCardGrid();

        if (BeginModuleCard("##cfgmgr", "Config Manager", nullptr)) {
            ImGui::TextColored(TextMuted(), "Config name");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##cfgname", Vars::Config::nameBuf, sizeof(Vars::Config::nameBuf));

            ImGui::Spacing();
            if (UiTheme::PrimaryButton("Save", ImVec2(-1, 34)))
                ConfigSystem::Save(Vars::Config::nameBuf);

            if (UiTheme::StyledButton("Load", ImVec2(-1, 34)))
                ConfigSystem::Load(Vars::Config::nameBuf);

            if (UiTheme::StyledButton("Delete", ImVec2(-1, 34)))
                ConfigSystem::Delete(Vars::Config::nameBuf);

            ImGui::Spacing();
            if (!ConfigSystem::statusMessage.empty()) {
                ImVec4 col = ConfigSystem::statusOk
                    ? ImVec4(0.35f, 0.9f, 0.45f, 1.f)
                    : ImVec4(1.f, 0.35f, 0.4f, 1.f);
                ImGui::TextColored(col, "%s", ConfigSystem::statusMessage.c_str());
            }
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##cfglist", "Saved Configs", nullptr)) {
            auto configs = ConfigSystem::List();
            if (configs.empty()) {
                ImGui::TextColored(TextMuted(), "No configs yet.");
                ImGui::TextColored(TextMuted(), "Save one to get started.");
            } else {
                float listH = ImGui::GetContentRegionAvail().y - 8.f;
                ImGui::BeginChild("##cfgscroll", ImVec2(-1, listH), false);
                for (int i = 0; i < static_cast<int>(configs.size()); i++) {
                    bool selected = (Vars::Config::selectedIndex == i);
                    if (ImGui::Selectable(configs[i].c_str(), selected, 0, ImVec2(-1, 26.f))) {
                        Vars::Config::selectedIndex = i;
                        strncpy_s(Vars::Config::nameBuf, configs[i].c_str(), sizeof(Vars::Config::nameBuf) - 1);
                    }
                }
                ImGui::EndChild();
            }

            ImGui::Spacing();
            ImGui::TextColored(TextMuted(), "Folder:");
            std::string folder = ConfigSystem::GetConfigDir().string();
            ImGui::TextWrapped("%s", folder.c_str());
        }
        EndModuleCard();

        EndCardGrid();
    }

    inline void RenderAboutTab() {
        BeginCardGrid();

        if (BeginModuleCard("##info", "CHEAT GLOBAL", nullptr)) {
            if (AppFonts::Title) ImGui::PushFont(AppFonts::Title);
            ImGui::TextColored(Accent(), "Hz1337");
            if (AppFonts::Title) ImGui::PopFont();
            ImGui::Spacing();
            ImGui::TextColored(TextMuted(), "Premium Roblox External");
            ImGui::Spacing();
            ImGui::TextColored(Accent(), "KEYBINDS");
            ImGui::BulletText("INSERT  Toggle menu");
            ImGui::BulletText("Aim key  Hold to lock target");
            ImGui::BulletText("WASD + Space/Ctrl  Fly controls");
        }
        EndModuleCard();
        NextCardColumn();

        if (BeginModuleCard("##ver", "Build", nullptr)) {
            ImGui::PushStyleColor(ImGuiCol_Text, TextMuted());
            ImGui::Text("%s", offsets::RobloxVersion.c_str());
            ImGui::Text("Source: %s", offsets::OffsetSource.c_str());
            ImGui::Text("Auto sync: %s", offsets::AutoSyncEnabled ? "ON" : "OFF");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::Checkbox("Auto offset update", &offsets::AutoSyncEnabled);
            if (UiTheme::PrimaryButton("Sync offsets now", ImVec2(-1, 34))) {
                if (OffsetUpdater::EnsureSynced(false, true)) {
                    Notify::Push("Offsets", "Updated to " + offsets::RobloxVersion, Notify::Type::Info, 4.f);
                } else {
                    Notify::Push("Offsets", "Sync failed - using embedded", Notify::Type::Warning, 4.f);
                }
            }
        }
        EndModuleCard();

        EndCardGrid();
    }

    inline void Render() {
        static bool wasMenuOpen = false;
        if (!Vars::menuOpen) {
            wasMenuOpen = false;
            return;
        }

        if (!wasMenuOpen)
            ImGui::GetStateStorage()->SetFloat(ImGui::GetID("##menu_open_anim"), 0.f);
        wasMenuOpen = true;

        const ImVec2 winSize(1180, 680);
        float openAnim = UiTheme::Animate(ImGui::GetID("##menu_open_anim"), 1.f, 18.f);

        ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(
            (ImGui::GetIO().DisplaySize.x - winSize.x) * 0.5f,
            (ImGui::GetIO().DisplaySize.y - winSize.y) * 0.5f
        ), ImGuiCond_FirstUseEver);

        if (AppFonts::Regular) ImGui::PushFont(AppFonts::Regular);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.15f + openAnim * 0.85f);
        ImGui::Begin("##cg", &Vars::menuOpen,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
        const float sideW = 232.f;
        const float headerH = 88.f;

        const char* tabTitles[] = {
            "Combat", "Visuals", "Local Player", "Players", "Configuration", "About"
        };
        const char* tabDescriptions[] = {
            "Aim assistance, trigger control and Blade Ball automation",
            "Player ESP, chams and Murder Mystery 2 role detection",
            "Movement modifiers, utility tools and teleport systems",
            "Browse online players and run player actions",
            "Save, load and manage your configuration profiles",
            "Build information, offsets and quick reference"
        };
        int tab = (Vars::selectedTab >= 0 && Vars::selectedTab < 6) ? Vars::selectedTab : 0;

        UiTheme::DrawSoftShadow(dl, wp, ImVec2(wp.x + ws.x, wp.y + ws.y), 18.f, 6);
        dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), UiTheme::Col(6, 8, 13, 252), 18.f);
        dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), UiTheme::Col(140, 110, 255, 35), 18.f, 0, 1.f);

        dl->AddRectFilled(wp, ImVec2(wp.x + sideW, wp.y + ws.y), UiTheme::Col(9, 11, 17, 255),
            18.f, ImDrawFlags_RoundCornersLeft);
        dl->AddRectFilled(ImVec2(wp.x + sideW, wp.y),
            ImVec2(wp.x + ws.x, wp.y + headerH), UiTheme::Col(10, 12, 18, 255),
            18.f, ImDrawFlags_RoundCornersTopRight);
        dl->AddRectFilledMultiColor(
            ImVec2(wp.x + sideW, wp.y), ImVec2(wp.x + ws.x, wp.y + 2.f),
            UiTheme::Col(140, 110, 255, 180), UiTheme::Col(90, 160, 255, 180),
            UiTheme::Col(90, 160, 255, 180), UiTheme::Col(140, 110, 255, 180));
        dl->AddLine(ImVec2(wp.x + sideW, wp.y), ImVec2(wp.x + sideW, wp.y + ws.y), UiTheme::Col(255, 255, 255, 14));
        dl->AddLine(ImVec2(wp.x + sideW, wp.y + headerH),
            ImVec2(wp.x + ws.x, wp.y + headerH), UiTheme::Col(255, 255, 255, 10));

        dl->AddCircleFilled(ImVec2(wp.x + 32.f, wp.y + 32.f), 14.f, UiTheme::Col(140, 110, 255, 255));
        dl->AddCircleFilled(ImVec2(wp.x + 32.f, wp.y + 32.f), 5.f, UiTheme::Col(250, 248, 255, 255));

        ImGui::SetCursorPos(ImVec2(56, 18));
        if (AppFonts::Title) ImGui::PushFont(AppFonts::Title);
        ImGui::TextColored(TextBright(), "CHEAT GLOBAL");
        if (AppFonts::Title) ImGui::PopFont();
        ImGui::SetCursorPos(ImVec2(56, 48));
        ImGui::TextColored(TextMuted(), "EXTERNAL SUITE  /  2026");

        ImGui::SetCursorPos(ImVec2(18, 96));
        ImGui::TextColored(UiTheme::TextDim(), "NAVIGATION");
        ImGui::SetCursorPos(ImVec2(12, 122));
        ImGui::BeginChild("##nav", ImVec2(sideW - 16, ws.y - 204), false);
        SidebarItem("Combat", "Aimbot & Blade Ball", 0);
        SidebarItem("Visuals", "ESP & Chams", 1);
        SidebarItem("Local", "Movement & TP", 2);
        SidebarItem("Players", "List & Actions", 3);
        SidebarItem("Config", "Profiles", 4);
        SidebarItem("About", "Info & Offsets", 5);
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(14, ws.y - 72));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.06f, 0.09f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.22f, 0.32f, 0.55f));
        ImGui::BeginChild("##prof", ImVec2(sideW - 28, 58), true);
        ImGui::SetCursorPos(ImVec2(14, 12));
        ImGui::TextColored(TextBright(), "Hz1337");
        ImU32 statusCol = Globals::dataModel.Addr ? UiTheme::Col(70, 220, 130, 255) : UiTheme::Col(240, 90, 100, 255);
        dl->AddCircleFilled(ImVec2(ImGui::GetWindowPos().x + 16.f, ImGui::GetWindowPos().y + 42.f), 4.f, statusCol);
        ImGui::SetCursorPos(ImVec2(26, 34));
        ImGui::TextColored(UiTheme::TextMuted(), Globals::dataModel.Addr ? "Attached to game" : "Waiting for attach...");
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::SetCursorPos(ImVec2(sideW + 26, 20));
        if (AppFonts::Title) ImGui::PushFont(AppFonts::Title);
        ImGui::Text("%s", tabTitles[tab]);
        if (AppFonts::Title) ImGui::PopFont();
        ImGui::SetCursorPos(ImVec2(sideW + 26, 52));
        ImGui::PushTextWrapPos(wp.x + ws.x - 200.f);
        ImGui::TextColored(TextMuted(), "%s", tabDescriptions[tab]);
        ImGui::PopTextWrapPos();

        ImVec2 pillMin(wp.x + ws.x - 176.f, wp.y + 22.f);
        UiTheme::DrawGlassPanel(dl, pillMin, ImVec2(pillMin.x + 154.f, pillMin.y + 36.f),
            18.f, UiTheme::Col(16, 18, 26, 235), UiTheme::Col(140, 110, 255, 45));
        ImFont* smallFont = AppFonts::Small ? AppFonts::Small : ImGui::GetFont();
        dl->AddCircleFilled(ImVec2(pillMin.x + 16.f, pillMin.y + 18.f), 4.f, UiTheme::Col(140, 110, 255, 255));
        dl->AddText(smallFont, AppFonts::FontSize(smallFont, 13.f),
            ImVec2(pillMin.x + 28.f, pillMin.y + 10.f), UiTheme::Col(210, 214, 228, 255), "INSERT");

        ImGui::SetCursorPos(ImVec2(ws.x - 46.f, 18.f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.12f, 0.14f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.14f, 0.16f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
        if (ImGui::Button("X##close", ImVec2(34, 34)))
            Vars::menuOpen = false;
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SetCursorPos(ImVec2(sideW + 18, headerH + 18));
        ImGui::BeginChild("##content",
            ImVec2(ws.x - sideW - 36, ws.y - headerH - 36), false);

        switch (Vars::selectedTab) {
            case 0: RenderCombatTab(); break;
            case 1: RenderVisualsTab(); break;
            case 2: RenderLocalTab(); break;
            case 3: RenderPlayersTab(); break;
            case 4: RenderConfigTab(); break;
            case 5: RenderAboutTab(); break;
        }
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar(2);
        if (AppFonts::Regular) ImGui::PopFont();
    }

    inline void RenderWatermark(int fps) {
        Hud::Render(fps);
    }
}
