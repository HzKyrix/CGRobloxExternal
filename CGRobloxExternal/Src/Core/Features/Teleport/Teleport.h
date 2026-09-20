#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Game/SDK/SDK.h"
#include "../../Globals/Globals.h"
#include "../../Vars/Vars.h"
#include "../PlayerList/PlayerList.h"
#include "../Misc/Notifications.h"
#include <Windows.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <vector>
#include <functional>

namespace TeleportFeatures {

    inline std::string stageStatus = "Ready";
    inline std::string clickStatus = "Hold key + LMB";

    inline std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    inline bool IsPartClass(const std::string& cls) {
        return cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" ||
               cls == "TrussPart" || cls == "SpawnLocation";
    }

    inline RBX::Vec3 Add(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X + b.X, a.Y + b.Y, a.Z + b.Z };
    }

    inline RBX::Vec3 Sub(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X - b.X, a.Y - b.Y, a.Z - b.Z };
    }

    inline RBX::Vec3 Scale(const RBX::Vec3& v, float s) {
        return { v.X * s, v.Y * s, v.Z * s };
    }

    inline float Len(const RBX::Vec3& v) {
        return sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    }

    inline RBX::Vec3 Normalize(const RBX::Vec3& v) {
        float l = Len(v);
        if (l < 0.0001f) return {};
        return { v.X / l, v.Y / l, v.Z / l };
    }

    inline float Dist2D(float ax, float ay, float bx, float by) {
        float dx = ax - bx;
        float dy = ay - by;
        return sqrtf(dx * dx + dy * dy);
    }

    inline bool TeleportLocalRig(const RBX::Vec3& destHrpPos) {
        auto hrp = PlayerListFeature::GetLocalHRP();
        if (hrp.Addr == 0) return false;

        RBX::Vec3 delta = Sub(destHrpPos, hrp.GetPos());
        bool moved = false;

        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr != 0) {
            std::vector<RBX::RbxInstance> parts;
            PlayerListFeature::CollectCharacterParts(character, parts);
            for (auto& part : parts) {
                RBX::Vec3 np = Add(part.GetPos(), delta);
                if (PlayerListFeature::SetPartWorldPosition(part, np))
                    moved = true;
            }
        }

        if (!moved)
            moved = PlayerListFeature::SetPartWorldPosition(hrp, destHrpPos);

        return moved;
    }

    inline int ExtractStageNumber(const std::string& name) {
        int best = -1;
        int current = 0;
        bool inNumber = false;

        for (char c : name) {
            if (c >= '0' && c <= '9') {
                current = current * 10 + (c - '0');
                inNumber = true;
            } else if (inNumber) {
                if (current > best) best = current;
                current = 0;
                inNumber = false;
            }
        }
        if (inNumber && current > best) best = current;
        return best;
    }

    inline int ScoreStageName(const std::string& rawName) {
        std::string name = ToLower(rawName);

        if (name == "end" || name == "finish" || name == "goal" || name == "win" ||
            name == "complete" || name == "stageend" || name == "victory")
            return 100000;

        if (name.find("finish") != std::string::npos || name.find("stageend") != std::string::npos ||
            name.find("goal") != std::string::npos || name.find("complete") != std::string::npos)
            return 90000;

        if (name.find("checkpoint") != std::string::npos || name.find("stage") != std::string::npos) {
            int num = ExtractStageNumber(rawName);
            if (num >= 0) return 50000 + num;
        }

        if (name.find("end") != std::string::npos)
            return 40000;

        return -1;
    }

    struct StageCandidate {
        RBX::Vec3 pos{};
        int score = -1;
        std::string name;
    };

    inline void ScanStageParts(RBX::RbxInstance& node, int depth, std::vector<StageCandidate>& out) {
        if (node.Addr == 0 || depth > 10) return;

        std::string cls = node.GetClass();
        if (IsPartClass(cls)) {
            int score = ScoreStageName(node.GetName());
            if (score >= 0) {
                StageCandidate c;
                c.pos = node.GetPos();
                c.score = score;
                c.name = node.GetName();
                out.push_back(c);
            }
        }

        if (depth >= 10) return;
        for (auto& child : node.GetChildList())
            ScanStageParts(child, depth + 1, out);
    }

    inline bool FindStageEndPosition(RBX::Vec3& outPos, std::string& outName) {
        if (Globals::workspace.Addr == 0) return false;

        std::vector<StageCandidate> candidates;
        ScanStageParts(Globals::workspace, 0, candidates);

        static const char* folders[] = { "Stages", "Checkpoints", "Obby", "Tower", "Map", "Course" };
        for (const char* folderName : folders) {
            auto folder = Globals::workspace.FindChild(folderName);
            if (folder.Addr != 0)
                ScanStageParts(folder, 0, candidates);
        }

        if (candidates.empty()) {
            StageCandidate fallback{};
            float bestY = -999999.f;

            std::function<void(RBX::RbxInstance&, int)> scanHigh =
                [&](RBX::RbxInstance& node, int depth) {
                if (node.Addr == 0 || depth > 8) return;
                if (IsPartClass(node.GetClass())) {
                    RBX::Vec3 p = node.GetPos();
                    if (p.Y > bestY) {
                        bestY = p.Y;
                        fallback.pos = p;
                        fallback.name = node.GetName();
                        fallback.score = 1;
                    }
                }
                for (auto& child : node.GetChildList())
                    scanHigh(child, depth + 1);
            };

            scanHigh(Globals::workspace, 0);
            if (fallback.score > 0)
                candidates.push_back(fallback);
        }

        if (candidates.empty()) return false;

        auto best = std::max_element(candidates.begin(), candidates.end(),
            [](const StageCandidate& a, const StageCandidate& b) {
                if (a.score != b.score) return a.score < b.score;
                return a.pos.Y < b.pos.Y;
            });

        outPos = best->pos;
        outPos.Y += Vars::Teleport::stageEndOffsetY;
        outName = best->name;
        return true;
    }

    inline bool StageEndTP() {
        RBX::Vec3 pos{};
        std::string name;
        if (!FindStageEndPosition(pos, name)) {
            stageStatus = "Stage end not found";
            Notify::Push("Teleport", "Stage end not found", Notify::Type::Warning, 3.f);
            return false;
        }

        if (!TeleportLocalRig(pos)) {
            stageStatus = "TP failed";
            Notify::Push("Teleport", "Stage TP failed", Notify::Type::Error, 3.f);
            return false;
        }

        stageStatus = "TP -> " + name;
        Notify::Push("Stage End TP", name, Notify::Type::Success, 3.f);
        return true;
    }

    inline RBX::Vec3 GetCameraPosition() {
        if (Globals::camera.Addr == 0) return {};
        return Coms->ReadMemory<RBX::Vec3>(Globals::camera.Addr + offsets::CameraPos);
    }

    inline RBX::CFrame GetCameraRotation() {
        if (Globals::camera.Addr == 0) return {};
        return Coms->ReadMemory<RBX::CFrame>(Globals::camera.Addr + offsets::CameraRotation);
    }

    inline RBX::Vec3 ScreenToWorldDirection(const RBX::Mat4& viewMatrix, float mouseX, float mouseY) {
        float sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

        RBX::CFrame camRot = GetCameraRotation();
        RBX::Vec3 forward = Normalize(camRot.GetLookVector());
        RBX::Vec3 right = Normalize(camRot.GetRightVector());
        RBX::Vec3 up = Normalize(camRot.GetUpVector());

        float fov = 70.f;
        if (Globals::camera.Addr != 0)
            fov = Coms->ReadMemory<float>(Globals::camera.Addr + offsets::FOV);
        if (fov < 1.f) fov = 70.f;

        float fovRad = fov * 3.14159265f / 180.f;
        float tanHalf = tanf(fovRad * 0.5f);
        float aspect = sw / sh;

        float mx = (mouseX / sw - 0.5f) * 2.f;
        float my = (0.5f - mouseY / sh) * 2.f;

        RBX::Vec3 dir = Add(forward, Add(Scale(right, mx * tanHalf * aspect), Scale(up, my * tanHalf)));
        (void)viewMatrix;
        return Normalize(dir);
    }

    inline void CollectClickableParts(RBX::RbxInstance& node, int depth,
        const RBX::Vec3& localPos, std::vector<std::pair<RBX::RbxInstance, RBX::Vec3>>& out)
    {
        if (node.Addr == 0 || depth > 9) return;

        std::string cls = node.GetClass();
        if (IsPartClass(cls)) {
            RBX::Vec3 pos = node.GetPos();
            float dist = Len(Sub(pos, localPos));
            if (dist <= Vars::Teleport::clickTpMaxScan)
                out.push_back({ node, pos });
        }

        for (auto& child : node.GetChildList())
            CollectClickableParts(child, depth + 1, localPos, out);
    }

    inline bool TryPartClickTP(const RBX::Mat4& viewMatrix, float mouseX, float mouseY) {
        auto hrp = PlayerListFeature::GetLocalHRP();
        if (hrp.Addr == 0) return false;

        RBX::Vec3 localPos = hrp.GetPos();
        std::vector<std::pair<RBX::RbxInstance, RBX::Vec3>> parts;
        if (Globals::workspace.Addr != 0)
            CollectClickableParts(Globals::workspace, 0, localPos, parts);

        if (parts.empty()) return false;

        float bestDist = 999999.f;
        RBX::Vec3 bestPos{};
        bool found = false;

        for (auto& entry : parts) {
            RBX::Vec2 sp = W2S::WorldToScreen(entry.second, viewMatrix);
            if (sp.X <= 0.f && sp.Y <= 0.f) continue;

            float d = Dist2D(mouseX, mouseY, sp.X, sp.Y);
            if (d <= Vars::Teleport::clickPartRadius && d < bestDist) {
                bestDist = d;
                bestPos = entry.second;
                found = true;
            }
        }

        if (!found) return false;

        bestPos.Y += Vars::Teleport::clickTpOffsetY;
        return TeleportLocalRig(bestPos);
    }

    inline bool TryRayClickTP(const RBX::Mat4& viewMatrix, float mouseX, float mouseY) {
        RBX::Vec3 camPos = GetCameraPosition();
        if (camPos.X == 0.f && camPos.Y == 0.f && camPos.Z == 0.f) {
            auto hrp = PlayerListFeature::GetLocalHRP();
            if (hrp.Addr == 0) return false;
            camPos = hrp.GetPos();
        }

        RBX::Vec3 dir = ScreenToWorldDirection(viewMatrix, mouseX, mouseY);
        RBX::Vec3 dest = Add(camPos, Scale(dir, Vars::Teleport::clickTpDistance));
        dest.Y += Vars::Teleport::clickTpOffsetY;
        return TeleportLocalRig(dest);
    }

    inline bool IsClickKeyActive() {
        int key = Vars::Teleport::clickTpKey;
        if (key == 0) return true;
        if (key == 1) return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (key == 2) return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (key == 4) return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (key == 5) return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
        if (key == 6) return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
        if (key == 16) return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        if (key == 17) return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (key == 18) return (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    inline void UpdateClickTP(const RBX::Mat4& viewMatrix) {
        if (!Vars::Teleport::clickTpEnabled) return;
        if (Vars::menuOpen) return;

        HWND roblox = FindWindowW(nullptr, L"Roblox");
        if (!roblox) roblox = FindWindowW(nullptr, L"RobloxPlayerBeta");
        if (!roblox || GetForegroundWindow() != roblox) return;

        if (!IsClickKeyActive()) {
            clickStatus = "Hold TP key + LMB";
            return;
        }

        static bool wasLmb = false;
        bool lmb = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        if (lmb && !wasLmb) {
            POINT pt{};
            GetCursorPos(&pt);
            float mx = static_cast<float>(pt.x);
            float my = static_cast<float>(pt.y);

            bool ok = false;
            if (Vars::Teleport::clickTpMode == 0)
                ok = TryRayClickTP(viewMatrix, mx, my);
            else
                ok = TryPartClickTP(viewMatrix, mx, my);

            if (ok) {
                clickStatus = "Teleported";
                if (Vars::Teleport::clickTpNotify)
                    Notify::Push("Click TP", "Teleported", Notify::Type::Success, 2.f);
            } else {
                clickStatus = "No target";
            }
        }

        wasLmb = lmb;
    }
}
