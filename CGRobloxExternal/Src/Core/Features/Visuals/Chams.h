#pragma once
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Globals/Globals.h"
#include "../../../Core/Vars/Vars.h"
#include "../../../Core/Features/MM2/MM2.h"
#include "../../../Render/ImGui/imgui.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

namespace ChamsRender {

    struct ProjectedFace {
        ImVec2 points[4];
        int count = 0;
        float depth = 0.f;
        RBX::Vec3 normal{};
        RBX::Vec3 center{};
    };

    inline bool WorldToScreen(const RBX::Vec3& world, const RBX::Mat4& vm, RBX::Vec2& out, float& outW) {
        RBX::Vec4 q;
        q.X = world.X * vm.data[0] + world.Y * vm.data[1] + world.Z * vm.data[2] + vm.data[3];
        q.Y = world.X * vm.data[4] + world.Y * vm.data[5] + world.Z * vm.data[6] + vm.data[7];
        q.Z = world.X * vm.data[8] + world.Y * vm.data[9] + world.Z * vm.data[10] + vm.data[11];
        q.W = world.X * vm.data[12] + world.Y * vm.data[13] + world.Z * vm.data[14] + vm.data[15];
        outW = q.W;
        if (q.W < 0.06f) return false;

        float iw = 1.f / q.W;
        const float sw = ImGui::GetIO().DisplaySize.x;
        const float sh = ImGui::GetIO().DisplaySize.y;
        out.X = (sw * 0.5f * q.X * iw) + (sw * 0.5f);
        out.Y = -(sh * 0.5f * q.Y * iw) + (sh * 0.5f);
        return std::isfinite(out.X) && std::isfinite(out.Y);
    }

    inline RBX::Vec3 Cross(const RBX::Vec3& a, const RBX::Vec3& b) {
        return {
            a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X
        };
    }

    inline float Dot(const RBX::Vec3& a, const RBX::Vec3& b) {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    inline RBX::Vec3 Normalize(const RBX::Vec3& v) {
        float len = sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
        if (len < 0.0001f) return {};
        return { v.X / len, v.Y / len, v.Z / len };
    }

    inline void GetPartHalfExtents(const PlayerCache::PartCache& part, float& hx, float& hy, float& hz) {
        const bool hasSize = part.size.X > 0.05f && part.size.Y > 0.05f && part.size.Z > 0.05f;
        if (hasSize) {
            hx = part.size.X * 0.5f;
            hy = part.size.Y * 0.5f;
            hz = part.size.Z * 0.5f;
            return;
        }

        hx = hy = hz = 0.45f;
        if (part.name == "Head") {
            hx = hy = hz = 0.55f;
        } else if (part.name == "HumanoidRootPart") {
            hx = 0.95f; hy = 0.95f; hz = 0.45f;
        } else if (part.name.find("Torso") != std::string::npos || part.name == "Torso") {
            hx = 0.95f; hy = 0.90f; hz = 0.48f;
        } else if (part.name.find("Hand") != std::string::npos || part.name.find("Foot") != std::string::npos) {
            hx = 0.30f; hy = 0.28f; hz = 0.30f;
        } else if (part.name.find("Arm") != std::string::npos || part.name.find("Leg") != std::string::npos) {
            hx = 0.42f; hy = 0.55f; hz = 0.42f;
        }
    }

    inline void BuildPartCorners(const PlayerCache::PartCache& part, RBX::Vec3 out[8]) {
        const RBX::CFrame& cf = part.smoothCf;
        RBX::Vec3 pos = cf.GetPosition();
        RBX::Vec3 right = cf.GetRightVector();
        RBX::Vec3 up = cf.GetUpVector();
        RBX::Vec3 look = cf.GetLookVector();

        float hx, hy, hz;
        GetPartHalfExtents(part, hx, hy, hz);

        int i = 0;
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2, ++i)
                    out[i] = {
                        pos.X + right.X * (x * hx) + up.X * (y * hy) + look.X * (z * hz),
                        pos.Y + right.Y * (x * hx) + up.Y * (y * hy) + look.Y * (z * hz),
                        pos.Z + right.Z * (x * hx) + up.Z * (y * hy) + look.Z * (z * hz)
                    };
    }

    inline ImU32 RoleFillColor(int mm2Role, int alpha) {
        if (Vars::MM2::enabled && Vars::Chams::teamColor) {
            if (mm2Role == static_cast<int>(MM2::Role::Murderer))
                return IM_COL32(255, 55, 75, alpha);
            if (mm2Role == static_cast<int>(MM2::Role::Sheriff))
                return IM_COL32(70, 140, 255, alpha);
        }

        int r = static_cast<int>(Vars::Chams::colorR * 255.f);
        int g = static_cast<int>(Vars::Chams::colorG * 255.f);
        int b = static_cast<int>(Vars::Chams::colorB * 255.f);
        return IM_COL32(r, g, b, alpha);
    }

    inline ImU32 RoleOutlineColor(int mm2Role, int alpha) {
        if (Vars::MM2::enabled && Vars::Chams::teamColor) {
            if (mm2Role == static_cast<int>(MM2::Role::Murderer))
                return IM_COL32(255, 120, 130, alpha);
            if (mm2Role == static_cast<int>(MM2::Role::Sheriff))
                return IM_COL32(140, 190, 255, alpha);
        }

        int r = (std::min)(255, static_cast<int>(Vars::Chams::colorR * 255.f) + 45);
        int g = (std::min)(255, static_cast<int>(Vars::Chams::colorG * 255.f) + 45);
        int b = (std::min)(255, static_cast<int>(Vars::Chams::colorB * 255.f) + 45);
        return IM_COL32(r, g, b, alpha);
    }

    inline void CollectPartFaces(
        const PlayerCache::PartCache& part,
        const RBX::Mat4& viewMatrix,
        const RBX::Vec3& cameraPos,
        std::vector<ProjectedFace>& outFaces,
        int mm2Role)
    {
        if (part.addr == 0) return;

        RBX::Vec3 corners[8];
        BuildPartCorners(part, corners);

        static const int kFaces[6][4] = {
            { 0, 1, 3, 2 },
            { 4, 6, 7, 5 },
            { 0, 2, 6, 4 },
            { 1, 5, 7, 3 },
            { 0, 4, 5, 1 },
            { 2, 3, 7, 6 }
        };

        const int baseAlpha = (std::min)(255, static_cast<int>(Vars::Chams::opacity * 255.f));

        for (int f = 0; f < 6; ++f) {
            RBX::Vec3 w[4];
            for (int i = 0; i < 4; ++i)
                w[i] = corners[kFaces[f][i]];

            RBX::Vec3 edge1 = { w[1].X - w[0].X, w[1].Y - w[0].Y, w[1].Z - w[0].Z };
            RBX::Vec3 edge2 = { w[3].X - w[0].X, w[3].Y - w[0].Y, w[3].Z - w[0].Z };
            RBX::Vec3 normal = Normalize(Cross(edge1, edge2));
            RBX::Vec3 center = {
                (w[0].X + w[1].X + w[2].X + w[3].X) * 0.25f,
                (w[0].Y + w[1].Y + w[2].Y + w[3].Y) * 0.25f,
                (w[0].Z + w[1].Z + w[2].Z + w[3].Z) * 0.25f
            };

            RBX::Vec3 toCam = { cameraPos.X - center.X, cameraPos.Y - center.Y, cameraPos.Z - center.Z };
            if (Dot(normal, toCam) <= 0.f) continue;

            ProjectedFace face;
            face.normal = normal;
            face.center = center;
            float depthSum = 0.f;
            float wSum = 0.f;

            for (int i = 0; i < 4; ++i) {
                RBX::Vec2 sp;
                float pw = 0.f;
                if (!WorldToScreen(w[i], viewMatrix, sp, pw)) continue;
                face.points[face.count++] = ImVec2(sp.X, sp.Y);
                depthSum += pw;
                wSum += 1.f;
            }

            if (face.count < 3) continue;
            face.depth = wSum > 0.f ? depthSum / wSum : 9999.f;
            outFaces.push_back(face);
        }
    }

    inline void DrawFaceFill(ImDrawList* dl, const ProjectedFace& face, ImU32 fillCol) {
        if (face.count == 3) {
            dl->AddTriangleFilled(face.points[0], face.points[1], face.points[2], fillCol);
            return;
        }
        if (face.count == 4) {
            float light = (std::max)(0.f, Dot(face.normal, Normalize({ 0.25f, 0.85f, 0.35f })));
            const int alpha = static_cast<int>((fillCol >> IM_COL32_A_SHIFT) & 0xFF);
            const int baseR = static_cast<int>((fillCol >> IM_COL32_R_SHIFT) & 0xFF);
            const int baseG = static_cast<int>((fillCol >> IM_COL32_G_SHIFT) & 0xFF);
            const int baseB = static_cast<int>((fillCol >> IM_COL32_B_SHIFT) & 0xFF);
            const int boost = static_cast<int>(light * 35.f);
            ImU32 top = IM_COL32(
                (std::min)(255, baseR + boost),
                (std::min)(255, baseG + boost),
                (std::min)(255, baseB + boost),
                alpha);
            ImU32 bottom = IM_COL32(
                baseR, baseG, baseB,
                (std::max)(0, alpha - 25));

            dl->AddQuadFilled(face.points[0], face.points[1], face.points[2], face.points[3], bottom);
            dl->AddTriangleFilled(face.points[0], face.points[1], face.points[2], top);
            dl->AddTriangleFilled(face.points[0], face.points[2], face.points[3], top);
            return;
        }
        dl->AddConvexPolyFilled(face.points, face.count, fillCol);
    }

    inline void DrawFaceOutline(ImDrawList* dl, const ProjectedFace& face, ImU32 outlineCol) {
        for (int i = 0; i < face.count; ++i) {
            const ImVec2& a = face.points[i];
            const ImVec2& b = face.points[(i + 1) % face.count];
            dl->AddLine(a, b, IM_COL32(0, 0, 0, 170), 2.6f);
            dl->AddLine(a, b, outlineCol, 1.25f);
        }
    }

    inline void DrawSilhouetteGlow(ImDrawList* dl, const ProjectedFace& face, ImU32 glowCol) {
        if (face.count < 3) return;
        ImVec2 c{ 0.f, 0.f };
        for (int i = 0; i < face.count; ++i) {
            c.x += face.points[i].x;
            c.y += face.points[i].y;
        }
        c.x /= face.count;
        c.y /= face.count;

        ImVec2 expanded[4];
        const float expand = 1.08f;
        for (int i = 0; i < face.count; ++i) {
            expanded[i].x = c.x + (face.points[i].x - c.x) * expand;
            expanded[i].y = c.y + (face.points[i].y - c.y) * expand;
        }

        if (face.count == 4)
            dl->AddQuadFilled(expanded[0], expanded[1], expanded[2], expanded[3], glowCol);
        else if (face.count == 3)
            dl->AddTriangleFilled(expanded[0], expanded[1], expanded[2], glowCol);
    }

    inline bool ShouldRenderPart(const std::string& name) {
        if (name.empty()) return false;
        if (name == "HumanoidRootPart" && !Vars::Chams::showRoot) return false;
        return true;
    }

    inline void RenderPlayer(
        ImDrawList* drawList,
        PlayerCache::CachedPlayer& plr,
        const RBX::Mat4& viewMatrix,
        const RBX::Vec3& cameraPos)
    {
        if (!Vars::Chams::enabled || plr.parts.empty()) return;
        if (plr.distance > 450.f) return;

        static thread_local std::vector<ProjectedFace> faces;
        faces.clear();
        faces.reserve(plr.parts.size() * 4);

        for (auto& part : plr.parts) {
            if (!ShouldRenderPart(part.name)) continue;
            CollectPartFaces(part, viewMatrix, cameraPos, faces, plr.mm2Role);
        }

        if (faces.empty()) return;

        std::sort(faces.begin(), faces.end(),
            [](const ProjectedFace& a, const ProjectedFace& b) { return a.depth > b.depth; });

        const int baseAlpha = (std::min)(255, static_cast<int>(Vars::Chams::opacity * 255.f));
        const ImU32 fillCol = RoleFillColor(plr.mm2Role, baseAlpha);
        const ImU32 outlineCol = RoleOutlineColor(plr.mm2Role, 230);
        const ImU32 glowCol = RoleFillColor(plr.mm2Role, (std::max)(20, baseAlpha / 3));

        if (Vars::Chams::glow) {
            for (const auto& face : faces)
                DrawSilhouetteGlow(drawList, face, glowCol);
        }

        if (Vars::Chams::fill) {
            for (const auto& face : faces)
                DrawFaceFill(drawList, face, fillCol);
        }

        if (Vars::Chams::outline) {
            for (const auto& face : faces)
                DrawFaceOutline(drawList, face, outlineCol);
        }
    }

    inline void RenderAll(ImDrawList* drawList, const RBX::Mat4& viewMatrix) {
        if (!Vars::Chams::enabled) return;

        RBX::Vec3 cameraPos = PlayerCache::localPlayerPos;
        if (Globals::camera.Addr != 0)
            cameraPos = Globals::camera.GetCameraPos();

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid) continue;
            RenderPlayer(drawList, plr, viewMatrix, cameraPos);
        }
    }
}
