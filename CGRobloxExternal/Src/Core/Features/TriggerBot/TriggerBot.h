#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Game/Offsets/Offsets.h"
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Globals/Globals.h"
#include "../../../Core/Vars/Vars.h"
#include <Windows.h>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace TriggerBot {

    inline bool IsKeyActive(int key) {
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

    inline bool IsSameTeam(uintptr_t playerAddr) {
        if (Globals::localPlayer.Addr == 0 || playerAddr == 0) return false;
        uintptr_t localTeam = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + offsets::Team);
        uintptr_t theirTeam = Coms->ReadMemory<uintptr_t>(playerAddr + offsets::Team);
        if (localTeam == 0 || theirTeam == 0) return false;
        return localTeam == theirTeam;
    }

    inline float Dist2D(float ax, float ay, float bx, float by) {
        float dx = ax - bx;
        float dy = ay - by;
        return sqrtf(dx * dx + dy * dy);
    }

    inline bool PointInRect(float px, float py, float minX, float minY, float maxX, float maxY) {
        return px >= minX && px <= maxX && py >= minY && py <= maxY;
    }

    inline bool WorldToScreenValid(const RBX::Vec3& world, const RBX::Mat4& vm, RBX::Vec2& out) {
        RBX::Vec4 q;
        q.X = world.X * vm.data[0] + world.Y * vm.data[1] + world.Z * vm.data[2] + vm.data[3];
        q.Y = world.X * vm.data[4] + world.Y * vm.data[5] + world.Z * vm.data[6] + vm.data[7];
        q.Z = world.X * vm.data[8] + world.Y * vm.data[9] + world.Z * vm.data[10] + vm.data[11];
        q.W = world.X * vm.data[12] + world.Y * vm.data[13] + world.Z * vm.data[14] + vm.data[15];
        if (q.W < 0.08f) return false;

        float iw = 1.f / q.W;
        float sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        out.X = (sw * 0.5f * q.X * iw) + (sw * 0.5f);
        out.Y = -(sh * 0.5f * q.Y * iw) + (sh * 0.5f);
        return true;
    }

    inline bool PartOnCursor(const PlayerCache::PartCache& part, const RBX::Mat4& vm,
        float cx, float cy, float pad)
    {
        if (part.addr == 0) return false;

        auto pos = part.smoothCf.GetPosition();
        auto right = part.smoothCf.GetRightVector();
        auto up = part.smoothCf.GetUpVector();
        auto look = part.smoothCf.GetLookVector();

        float hx = (part.size.X > 0.05f) ? part.size.X * 0.5f : 0.5f;
        float hy = (part.size.Y > 0.05f) ? part.size.Y * 0.5f : 0.5f;
        float hz = (part.size.Z > 0.05f) ? part.size.Z * 0.5f : 0.5f;

        float minX = 999999.f, minY = 999999.f;
        float maxX = -999999.f, maxY = -999999.f;
        int valid = 0;
        RBX::Vec2 sp;

        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2) {
                    RBX::Vec3 corner{
                        pos.X + right.X * (x * hx) + up.X * (y * hy) + look.X * (z * hz),
                        pos.Y + right.Y * (x * hx) + up.Y * (y * hy) + look.Y * (z * hz),
                        pos.Z + right.Z * (x * hx) + up.Z * (y * hy) + look.Z * (z * hz)
                    };
                    if (!WorldToScreenValid(corner, vm, sp)) continue;
                    minX = (std::min)(minX, sp.X);
                    minY = (std::min)(minY, sp.Y);
                    maxX = (std::max)(maxX, sp.X);
                    maxY = (std::max)(maxY, sp.Y);
                    valid++;
                }

        if (valid < 3) return false;
        return PointInRect(cx, cy, minX - pad, minY - pad, maxX + pad, maxY + pad);
    }

    inline bool BoneOnCursor(const PlayerCache::CachedPlayer& plr, const RBX::Mat4& vm,
        float cx, float cy, float radius, int hitPart)
    {
        if (hitPart == 2) {
            for (auto& part : plr.parts) {
                if (PartOnCursor(part, vm, cx, cy, Vars::TriggerBot::hitboxPad))
                    return true;
            }
            return false;
        }

        RBX::RbxInstance target{ 0 };
        auto character = RBX::RbxInstance(plr.characterAddr);
        if (hitPart == 0)
            target = character.FindChild("Head");
        else if (hitPart == 1) {
            target = RBX::RbxInstance(plr.rootPartAddr);
            if (target.Addr == 0)
                target = character.FindChild("UpperTorso");
            if (target.Addr == 0)
                target = character.FindChild("Torso");
        }

        if (target.Addr == 0) return false;

        RBX::Vec3 pos = target.GetPos();
        RBX::Vec2 screen = W2S::WorldToScreen(pos, vm);
        if (screen.X == 0.f && screen.Y == 0.f) return false;

        return Dist2D(cx, cy, screen.X, screen.Y) <= radius;
    }

    inline bool PlayerOnCrosshair(const PlayerCache::CachedPlayer& plr, const RBX::Mat4& vm,
        float cx, float cy)
    {
        if (!plr.isValid) return false;
        if (plr.playerAddr == Globals::localPlayer.Addr) return false;
        if (plr.distance > Vars::TriggerBot::maxDistance) return false;
        if (Vars::TriggerBot::teamCheck && IsSameTeam(plr.playerAddr)) return false;
        if (Vars::TriggerBot::ignoreDead && plr.maxHealth > 0 && plr.health <= 0) return false;

        return BoneOnCursor(plr, vm, cx, cy, Vars::TriggerBot::triggerRadius, Vars::TriggerBot::hitPart);
    }

    inline void TapFire() {
        INPUT down{};
        down.type = INPUT_MOUSE;
        down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        INPUT up{};
        up.type = INPUT_MOUSE;
        up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        INPUT arr[2] = { down, up };
        SendInput(2, arr, sizeof(INPUT));
    }

    inline void Run(const RBX::Mat4& viewMatrix) {
        if (!Vars::TriggerBot::enabled) return;
        if (!IsKeyActive(Vars::TriggerBot::triggerKey)) return;
        if (Vars::TriggerBot::skipWhileFiring && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) return;

        POINT mouse{};
        GetCursorPos(&mouse);
        float cx = static_cast<float>(mouse.x);
        float cy = static_cast<float>(mouse.y);

        bool onTarget = false;
        for (auto& plr : PlayerCache::players) {
            if (PlayerOnCrosshair(plr, viewMatrix, cx, cy)) {
                onTarget = true;
                break;
            }
        }

        static auto targetSince = std::chrono::steady_clock::time_point{};
        static auto lastFire = std::chrono::steady_clock::now() - std::chrono::seconds(1);
        auto now = std::chrono::steady_clock::now();

        if (!onTarget) {
            targetSince = std::chrono::steady_clock::time_point{};
            return;
        }

        if (targetSince.time_since_epoch().count() == 0)
            targetSince = now;

        auto delayMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - targetSince).count();
        if (delayMs < static_cast<long long>(Vars::TriggerBot::delayMs))
            return;

        auto sinceFire = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFire).count();
        if (sinceFire < static_cast<long long>(Vars::TriggerBot::cooldownMs))
            return;

        TapFire();
        lastFire = now;
        targetSince = std::chrono::steady_clock::time_point{};
    }
}
