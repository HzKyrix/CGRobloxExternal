#pragma once
#include "../../Vars/Vars.h"
#include "Notifications.h"
#include <Windows.h>
#include <chrono>

namespace AntiAFK {

    inline std::chrono::steady_clock::time_point lastPulse{};
    inline bool wasEnabled = false;

    inline HWND GetRobloxWindow() {
        HWND w = FindWindowW(nullptr, L"Roblox");
        if (w) return w;
        return FindWindowW(nullptr, L"RobloxPlayerBeta");
    }

    inline void SendIdlePulse() {
        HWND roblox = GetRobloxWindow();
        if (!roblox) return;

        INPUT inputs[2]{};
        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE;
        inputs[0].mi.dx = 2;
        inputs[0].mi.dy = 0;

        inputs[1] = inputs[0];
        inputs[1].mi.dx = -2;
        inputs[1].mi.dy = 0;

        SendInput(2, inputs, sizeof(INPUT));

        POINT pt{};
        GetCursorPos(&pt);
        PostMessageW(roblox, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x & 0xFFFF, pt.y & 0xFFFF));
    }

    inline void Update() {
        if (!Vars::Misc::antiAfkEnabled) {
            wasEnabled = false;
            return;
        }

        if (!wasEnabled) {
            wasEnabled = true;
            lastPulse = std::chrono::steady_clock::now();
            Notify::Push("Anti-AFK", "Idle kick protection active", Notify::Type::Success, 3.f);
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPulse).count();
        if (elapsed < static_cast<long long>(Vars::Misc::antiAfkInterval))
            return;

        SendIdlePulse();
        lastPulse = now;

        if (Vars::Misc::antiAfkNotify)
            Notify::Push("Anti-AFK", "Activity pulse sent", Notify::Type::Info, 2.5f);
    }
}
