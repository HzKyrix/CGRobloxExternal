#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Globals/Globals.h"
#include "../../../Core/Vars/Vars.h"
#include "../../../Core/Features/MM2/MM2.h"
#include "ScreenFilter.h"
#include "Chams.h"
#include "../../../Render/Fonts.h"
#include "../../../Render/ImGui/imgui.h"
#include <string>
#include <algorithm>
#include <cmath>
#include <vector>

namespace Visuals {

    struct PartDrawInfo {
        uintptr_t addr;
        float depth;
    };

    inline bool WorldToScreenValid(const RBX::Vec3& world, const RBX::Mat4& vm, RBX::Vec2& out) {
        RBX::Vec4 q;
        q.X = world.X * vm.data[0] + world.Y * vm.data[1] + world.Z * vm.data[2] + vm.data[3];
        q.Y = world.X * vm.data[4] + world.Y * vm.data[5] + world.Z * vm.data[6] + vm.data[7];
        q.Z = world.X * vm.data[8] + world.Y * vm.data[9] + world.Z * vm.data[10] + vm.data[11];
        q.W = world.X * vm.data[12] + world.Y * vm.data[13] + world.Z * vm.data[14] + vm.data[15];
        if (q.W < 0.08f) return false;
        float iw = 1.f / q.W;
        const float sw = ImGui::GetIO().DisplaySize.x;
        const float sh = ImGui::GetIO().DisplaySize.y;
        out.X = (sw * 0.5f * q.X * iw) + (sw * 0.5f);
        out.Y = -(sh * 0.5f * q.Y * iw) + (sh * 0.5f);
        return true;
    }

    inline void GetPartHalfExtents(RBX::RbxInstance& part, float& hx, float& hy, float& hz) {
        auto size = part.GetSize();
        hx = (size.X > 0.05f) ? size.X * 0.5f : 0.5f;
        hy = (size.Y > 0.05f) ? size.Y * 0.5f : 0.5f;
        hz = (size.Z > 0.05f) ? size.Z * 0.5f : 0.5f;

        std::string n = part.GetName();
        if (n == "Head") { hx = hy = hz = (hx > 0.3f) ? hx : 0.55f; }
        else if (n.find("Torso") != std::string::npos || n == "Torso") {
            if (hx < 0.5f) hx = 0.9f;
            if (hy < 0.5f) hy = 0.85f;
            if (hz < 0.3f) hz = 0.45f;
        }
    }

    inline void DrawOutlinedText(ImDrawList* drawList, const ImVec2& pos, const std::string& text, ImU32 textColor, ImFont* font = nullptr) {
        if (font) ImGui::PushFont(font);
        drawList->AddText(ImVec2(pos.x - 1, pos.y), IM_COL32(0, 0, 0, 200), text.c_str());
        drawList->AddText(ImVec2(pos.x + 1, pos.y), IM_COL32(0, 0, 0, 200), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y - 1), IM_COL32(0, 0, 0, 200), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y + 1), IM_COL32(0, 0, 0, 200), text.c_str());
        drawList->AddText(pos, textColor, text.c_str());
        if (font) ImGui::PopFont();
    }

    inline void AddPartCorners(RBX::RbxInstance& part, RBX::Vec3* out, int& count) {
        if (part.Addr == 0) return;
        auto cf = part.GetCFrame();
        auto pos = cf.GetPosition();
        auto right = cf.GetRightVector();
        auto up = cf.GetUpVector();
        auto look = cf.GetLookVector();
        float hx, hy, hz;
        GetPartHalfExtents(part, hx, hy, hz);

        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                    out[count++] = {
                        pos.X + right.X * (x * hx) + up.X * (y * hy) + look.X * (z * hz),
                        pos.Y + right.Y * (x * hx) + up.Y * (y * hy) + look.Y * (z * hz),
                        pos.Z + right.Z * (x * hx) + up.Z * (y * hy) + look.Z * (z * hz)
                    };
    }

    inline void AddCachedPartCorners(const PlayerCache::PartCache& part, RBX::Vec3* out, int& count) {
        if (part.addr == 0) return;

        auto pos = part.smoothCf.GetPosition();
        auto right = part.smoothCf.GetRightVector();
        auto up = part.smoothCf.GetUpVector();
        auto look = part.smoothCf.GetLookVector();

        float hx = (part.size.X > 0.05f) ? part.size.X * 0.5f : 0.5f;
        float hy = (part.size.Y > 0.05f) ? part.size.Y * 0.5f : 0.5f;
        float hz = (part.size.Z > 0.05f) ? part.size.Z * 0.5f : 0.5f;

        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                    out[count++] = {
                        pos.X + right.X * (x * hx) + up.X * (y * hy) + look.X * (z * hz),
                        pos.Y + right.Y * (x * hx) + up.Y * (y * hy) + look.Y * (z * hz),
                        pos.Z + right.Z * (x * hx) + up.Z * (y * hy) + look.Z * (z * hz)
                    };
    }

    inline bool ProjectBounds(const RBX::Vec3* points, int count, const RBX::Mat4& viewMatrix,
        float& minX, float& minY, float& maxX, float& maxY)
    {
        minX = 999999.f; minY = 999999.f;
        maxX = -999999.f; maxY = -999999.f;
        int validPoints = 0;
        RBX::Vec2 sp;

        for (int i = 0; i < count; i++) {
            if (!WorldToScreenValid(points[i], viewMatrix, sp)) continue;
            minX = (std::min)(minX, sp.X);
            minY = (std::min)(minY, sp.Y);
            maxX = (std::max)(maxX, sp.X);
            maxY = (std::max)(maxY, sp.Y);
            validPoints++;
        }
        return validPoints >= 3;
    }

    inline bool ComputePlayerScreenBox(PlayerCache::CachedPlayer& plr, const RBX::Mat4& viewMatrix,
        float& minX, float& minY, float& maxX, float& maxY, float dt)
    {
        if (!plr.anchors.ready) return false;

        const auto& a = plr.anchors;
        const float halfW = a.bodyHeight * Vars::ESP::boxWidthRatio;
        const float topY = a.footY + a.bodyHeight;
        const float footY = a.footY;
        const float cx = a.centerX;
        const float cz = a.centerZ;

        const RBX::Vec3 corners[8] = {
            { cx - halfW, footY, cz - halfW }, { cx + halfW, footY, cz - halfW },
            { cx - halfW, topY,  cz - halfW }, { cx + halfW, topY,  cz - halfW },
            { cx - halfW, footY, cz + halfW }, { cx + halfW, footY, cz + halfW },
            { cx - halfW, topY,  cz + halfW }, { cx + halfW, topY,  cz + halfW },
        };

        if (!ProjectBounds(corners, 8, viewMatrix, minX, minY, maxX, maxY))
            return false;

        const float pad = Vars::ESP::boxPadding;
        minX -= pad; minY -= pad; maxX += pad; maxY += pad;

        if (Vars::ESP::stableBox)
            plr.screenBox.Apply(minX, minY, maxX, maxY, dt,
                Vars::ESP::screenStabilize, Vars::ESP::screenBeta);

        return true;
    }

    inline bool BuildBounds(RBX::RbxInstance& character, bool isR6, RBX::Vec3 boundPoints[], int& count) {
        count = 0;
        if (character.Addr == 0) return false;

        static const char* r6[] = { "Head","Torso","Left Arm","Right Arm","Left Leg","Right Leg","HumanoidRootPart" };
        static const char* r15[] = {
            "Head","UpperTorso","LowerTorso",
            "LeftUpperArm","LeftLowerArm","LeftHand",
            "RightUpperArm","RightLowerArm","RightHand",
            "LeftUpperLeg","LeftLowerLeg","LeftFoot",
            "RightUpperLeg","RightLowerLeg","RightFoot",
            "HumanoidRootPart"
        };
        const char** list = isR6 ? r6 : r15;
        int n = isR6 ? 7 : 16;

        for (int i = 0; i < n; i++) {
            auto p = character.FindChild(list[i]);
            AddPartCorners(p, boundPoints, count);
        }
        return count > 0;
    }

    inline RBX::RbxInstance CachedPartInst(PlayerCache::CachedPlayer& plr, const char* name) {
        if (auto* part = PlayerCache::FindPartByName(plr, name))
            return RBX::RbxInstance(part->addr);
        return RBX::RbxInstance(0);
    }

    inline void DrawBoneLine(ImDrawList* drawList, PlayerCache::CachedPlayer& plr,
        RBX::RbxInstance& from, RBX::RbxInstance& to, const RBX::Mat4& viewMatrix, ImU32 color)
    {
        if (from.Addr == 0 || to.Addr == 0) return;
        RBX::Vec2 a, b;
        if (!WorldToScreenValid(PlayerCache::GetPartPos(plr, from), viewMatrix, a)) return;
        if (!WorldToScreenValid(PlayerCache::GetPartPos(plr, to), viewMatrix, b)) return;
        drawList->AddLine(ImVec2(a.X, a.Y), ImVec2(b.X, b.Y), IM_COL32(0, 0, 0, 200), 2.5f);
        drawList->AddLine(ImVec2(a.X, a.Y), ImVec2(b.X, b.Y), color, 1.3f);
    }

    inline void RenderSkeleton(ImDrawList* drawList, PlayerCache::CachedPlayer& plr,
        const RBX::Mat4& viewMatrix)
    {
        if (!Vars::ESP::skeleton) return;

        const ImU32 boneColor = IM_COL32(255, 255, 255, 220);
        const bool isR6 = PlayerCache::FindPartByName(plr, "Torso") != nullptr;

        if (isR6) {
            auto head = CachedPartInst(plr, "Head");
            auto torso = CachedPartInst(plr, "Torso");
            auto la = CachedPartInst(plr, "Left Arm");
            auto ra = CachedPartInst(plr, "Right Arm");
            auto ll = CachedPartInst(plr, "Left Leg");
            auto rl = CachedPartInst(plr, "Right Leg");
            auto hrp = CachedPartInst(plr, "HumanoidRootPart");

            DrawBoneLine(drawList, plr, head, torso, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, torso, la, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, torso, ra, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, torso, ll, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, torso, rl, viewMatrix, boneColor);
            if (hrp.Addr != 0)
                DrawBoneLine(drawList, plr, hrp, torso, viewMatrix, boneColor);
        } else {
            auto head = CachedPartInst(plr, "Head");
            auto upper = CachedPartInst(plr, "UpperTorso");
            auto lower = CachedPartInst(plr, "LowerTorso");
            auto hrp = CachedPartInst(plr, "HumanoidRootPart");

            auto lua = CachedPartInst(plr, "LeftUpperArm");
            auto lla = CachedPartInst(plr, "LeftLowerArm");
            auto lh = CachedPartInst(plr, "LeftHand");
            auto rua = CachedPartInst(plr, "RightUpperArm");
            auto rla = CachedPartInst(plr, "RightLowerArm");
            auto rh = CachedPartInst(plr, "RightHand");

            auto lul = CachedPartInst(plr, "LeftUpperLeg");
            auto lll = CachedPartInst(plr, "LeftLowerLeg");
            auto lf = CachedPartInst(plr, "LeftFoot");
            auto rul = CachedPartInst(plr, "RightUpperLeg");
            auto rll = CachedPartInst(plr, "RightLowerLeg");
            auto rf = CachedPartInst(plr, "RightFoot");

            DrawBoneLine(drawList, plr, head, upper, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, upper, lower, viewMatrix, boneColor);
            if (hrp.Addr != 0)
                DrawBoneLine(drawList, plr, hrp, lower, viewMatrix, boneColor);

            DrawBoneLine(drawList, plr, upper, lua, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, lua, lla, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, lla, lh, viewMatrix, boneColor);

            DrawBoneLine(drawList, plr, upper, rua, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, rua, rla, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, rla, rh, viewMatrix, boneColor);

            DrawBoneLine(drawList, plr, lower, lul, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, lul, lll, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, lll, lf, viewMatrix, boneColor);

            DrawBoneLine(drawList, plr, lower, rul, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, rul, rll, viewMatrix, boneColor);
            DrawBoneLine(drawList, plr, rll, rf, viewMatrix, boneColor);
        }
    }

    inline ImVec2 GetTracerOrigin() {
        ImVec2 screen = ImGui::GetIO().DisplaySize;
        switch (Vars::ESP::tracerOrigin) {
            case 1: return ImVec2(screen.x * 0.5f, screen.y * 0.5f);
            case 2: return ImVec2(screen.x * 0.5f, 0.f);
            default: return ImVec2(screen.x * 0.5f, screen.y);
        }
    }

    inline void RenderTracer(ImDrawList* drawList, PlayerCache::CachedPlayer& plr,
        const RBX::Mat4& viewMatrix, float minX, float minY, float maxX, float maxY)
    {
        if (!Vars::ESP::tracers) return;

        RBX::Vec2 target;
        auto torso = CachedPartInst(plr, "Torso");
        if (torso.Addr == 0) torso = CachedPartInst(plr, "UpperTorso");
        if (torso.Addr == 0) torso = CachedPartInst(plr, "HumanoidRootPart");

        if (torso.Addr != 0) {
            if (!WorldToScreenValid(PlayerCache::GetPartPos(plr, torso), viewMatrix, target)) {
                target = RBX::Vec2{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            }
        } else {
            target = RBX::Vec2{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
        }

        ImVec2 origin = GetTracerOrigin();
        drawList->AddLine(origin, ImVec2(target.X, target.Y), IM_COL32(0, 0, 0, 180), 2.5f);
        drawList->AddLine(origin, ImVec2(target.X, target.Y), IM_COL32(210, 50, 65, 220), 1.2f);
    }

    inline void DrawRoleTag(ImDrawList* drawList, float minX, float minY, float maxX, const char* tag, ImU32 color, ImFont* font) {
        if (font) ImGui::PushFont(font);
        ImVec2 ts = ImGui::CalcTextSize(tag);
        if (font) ImGui::PopFont();
        float cx = (minX + maxX) * 0.5f - ts.x * 0.5f;
        float y = minY - ts.y - 18.f;
        DrawOutlinedText(drawList, ImVec2(cx, y), tag, color, font);
    }

    inline void DrawMurdererBox(ImDrawList* drawList, float minX, float minY, float maxX, float maxY) {
        drawList->AddRect(ImVec2(minX - 2, minY - 2), ImVec2(maxX + 2, maxY + 2),
            IM_COL32(0, 0, 0, 220), 3.f, 0, 3.f);
        drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY),
            IM_COL32(255, 45, 55, 255), 3.f, 0, 2.f);
        drawList->AddRect(ImVec2(minX + 1, minY + 1), ImVec2(maxX - 1, maxY - 1),
            IM_COL32(255, 120, 130, 120), 2.f, 0, 1.f);
    }

    inline void DrawSheriffBox(ImDrawList* drawList, float minX, float minY, float maxX, float maxY) {
        drawList->AddRect(ImVec2(minX - 2, minY - 2), ImVec2(maxX + 2, maxY + 2),
            IM_COL32(0, 0, 0, 220), 3.f, 0, 3.f);
        drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY),
            IM_COL32(55, 130, 255, 255), 3.f, 0, 2.f);
    }

    inline void RenderMM2HUD(ImDrawList* drawList) {
        if (!Vars::MM2::enabled || !Vars::MM2::murdererHUD) return;

        ImFont* font = AppFonts::Bold ? AppFonts::Bold : AppFonts::Regular;
        float y = 52.f;

        if (!MM2::murdererName.empty()) {
            std::string txt = "MURDERER: " + MM2::murdererName;
            if (font) ImGui::PushFont(font);
            ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
            if (font) ImGui::PopFont();

            float sw = ImGui::GetIO().DisplaySize.x;
            ImVec2 pos((sw - ts.x) * 0.5f - 12.f, y);
            drawList->AddRectFilled(
                ImVec2(pos.x - 8, pos.y - 4),
                ImVec2(pos.x + ts.x + 28, pos.y + ts.y + 6),
                IM_COL32(18, 8, 10, 230), 6.f);
            drawList->AddRect(
                ImVec2(pos.x - 8, pos.y - 4),
                ImVec2(pos.x + ts.x + 28, pos.y + ts.y + 6),
                IM_COL32(255, 50, 65, 200), 6.f, 0, 1.5f);
            DrawOutlinedText(drawList, ImVec2(pos.x + 10, pos.y), txt, IM_COL32(255, 70, 85, 255), font);
            y += ts.y + 14.f;
        }

        if (Vars::MM2::showSheriff && !MM2::sheriffName.empty()) {
            std::string txt = "SHERIFF: " + MM2::sheriffName;
            if (font) ImGui::PushFont(font);
            ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
            if (font) ImGui::PopFont();

            float sw = ImGui::GetIO().DisplaySize.x;
            ImVec2 pos((sw - ts.x) * 0.5f - 12.f, y);
            drawList->AddRectFilled(
                ImVec2(pos.x - 8, pos.y - 4),
                ImVec2(pos.x + ts.x + 28, pos.y + ts.y + 6),
                IM_COL32(8, 12, 22, 230), 6.f);
            drawList->AddRect(
                ImVec2(pos.x - 8, pos.y - 4),
                ImVec2(pos.x + ts.x + 28, pos.y + ts.y + 6),
                IM_COL32(55, 130, 255, 200), 6.f, 0, 1.5f);
            DrawOutlinedText(drawList, ImVec2(pos.x + 10, pos.y), txt, IM_COL32(90, 160, 255, 255), font);
        }
    }

    inline void RenderMM2Player(ImDrawList* drawList, PlayerCache::CachedPlayer& plr,
        RBX::RbxInstance& character, const RBX::Mat4& viewMatrix, float minX, float minY, float maxX, float maxY)
    {
        if (!Vars::MM2::enabled || !Vars::MM2::roleESP) return;

        ImFont* espFont = AppFonts::Bold ? AppFonts::Bold : AppFonts::Regular;

        if (plr.mm2Role == static_cast<int>(MM2::Role::Murderer)) {
            DrawMurdererBox(drawList, minX, minY, maxX, maxY);
            DrawRoleTag(drawList, minX, minY, maxX, "MURDERER", IM_COL32(255, 60, 75, 255), espFont);
        } else if (Vars::MM2::showSheriff && plr.mm2Role == static_cast<int>(MM2::Role::Sheriff)) {
            DrawSheriffBox(drawList, minX, minY, maxX, maxY);
            DrawRoleTag(drawList, minX, minY, maxX, "SHERIFF", IM_COL32(80, 150, 255, 255), espFont);
        }
    }

    inline void RenderMM2Roles(ImDrawList* drawList, const RBX::Mat4& viewMatrix, float dt) {
        if (!Vars::MM2::enabled || !Vars::MM2::roleESP) return;

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid) continue;
            if (plr.mm2Role == static_cast<int>(MM2::Role::Innocent)) continue;
            if (plr.mm2Role == static_cast<int>(MM2::Role::Sheriff) && !Vars::MM2::showSheriff) continue;

            auto character = RBX::RbxInstance(plr.characterAddr);

            float minX, minY, maxX, maxY;
            if (!ComputePlayerScreenBox(plr, viewMatrix, minX, minY, maxX, maxY, dt)) continue;

            RenderMM2Player(drawList, plr, character, viewMatrix, minX, minY, maxX, maxY);
        }
    }

    inline void RenderESP(ImDrawList* drawList, const RBX::Mat4& viewMatrix)
    {
        const bool mm2Active = Vars::MM2::enabled && Vars::MM2::roleESP;
        if (!Vars::ESP::enabled && !Vars::Chams::enabled && !mm2Active) return;

        ImFont* espFont = AppFonts::Regular;
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        float dt = ImGui::GetIO().DeltaTime;
        if (dt <= 0.f) dt = 0.016f;

        if (Vars::Chams::enabled)
            ChamsRender::RenderAll(drawList, viewMatrix);

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid) continue;

            float minX, minY, maxX, maxY;
            if (!ComputePlayerScreenBox(plr, viewMatrix, minX, minY, maxX, maxY, dt)) continue;

            if ((maxX - minX) > screenSize.x * 1.2f || (maxY - minY) > screenSize.y * 1.2f) continue;
            if ((maxX - minX) < 2.f || (maxY - minY) < 2.f) continue;

            const bool isMurderer = plr.mm2Role == static_cast<int>(MM2::Role::Murderer);
            const bool isSheriff = plr.mm2Role == static_cast<int>(MM2::Role::Sheriff);

            if (!Vars::ESP::enabled) continue;

            RenderSkeleton(drawList, plr, viewMatrix);
            RenderTracer(drawList, plr, viewMatrix, minX, minY, maxX, maxY);

            if (Vars::ESP::boxes) {
                ImU32 boxCol = IM_COL32(210, 50, 65, 255);
                if (mm2Active && isMurderer) boxCol = IM_COL32(255, 45, 55, 255);
                else if (mm2Active && isSheriff && Vars::MM2::showSheriff) boxCol = IM_COL32(55, 130, 255, 255);

                drawList->AddRect(ImVec2(minX - 1, minY - 1), ImVec2(maxX + 1, maxY + 1),
                    IM_COL32(0, 0, 0, 180), 2.f, 0, 2.f);
                drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), boxCol, 2.f, 0, 1.2f);
            }

            if (Vars::ESP::healthBar && plr.maxHealth > 0) {
                float hp = (std::max)(0.f, (std::min)(1.f, static_cast<float>(plr.health) / plr.maxHealth));
                float barH = (maxY - minY) * hp;
                drawList->AddRectFilled(ImVec2(minX - 6, minY), ImVec2(minX - 2, maxY), IM_COL32(0, 0, 0, 150), 2.f);
                drawList->AddRectFilled(ImVec2(minX - 5, maxY - barH), ImVec2(minX - 3, maxY),
                    IM_COL32(static_cast<int>(255 * (1 - hp)), static_cast<int>(255 * hp), 60, 255), 2.f);
            }

            if (Vars::ESP::names) {
                if (espFont) ImGui::PushFont(espFont);
                ImVec2 ts = ImGui::CalcTextSize(plr.name.c_str());
                if (espFont) ImGui::PopFont();
                DrawOutlinedText(drawList, ImVec2((minX + maxX) * 0.5f - ts.x * 0.5f, minY - ts.y - 3),
                    plr.name, IM_COL32(245, 245, 250, 255), espFont);
            } else if (mm2Active && isMurderer) {
                DrawRoleTag(drawList, minX, minY, maxX, "MURDERER",
                    IM_COL32(255, 60, 75, 255), AppFonts::Bold ? AppFonts::Bold : espFont);
            }

            if (Vars::ESP::distance) {
                std::string d = std::to_string(static_cast<int>(plr.distance)) + "m";
                if (espFont) ImGui::PushFont(espFont);
                ImVec2 ts = ImGui::CalcTextSize(d.c_str());
                if (espFont) ImGui::PopFont();
                DrawOutlinedText(drawList, ImVec2((minX + maxX) * 0.5f - ts.x * 0.5f, maxY + 3),
                    d, IM_COL32(210, 55, 70, 255), espFont);
            }
        }

        if (mm2Active)
            RenderMM2Roles(drawList, viewMatrix, dt);

        RenderMM2HUD(drawList);
    }
}
