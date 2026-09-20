#pragma once
#include "../../Vars/Vars.h"
#include <Windows.h>
#include <atomic>
#include <thread>
#include <chrono>

namespace AutoClicker {

    inline std::atomic<bool> threadRunning{ false };
    inline std::atomic<int> clicksPerSecond{ 0 };
    inline std::atomic<bool> isActive{ false };

    inline std::atomic<HWND> cachedRobloxWnd{ nullptr };
    inline std::atomic<long long> lastWndLookupMs{ 0 };

    inline long long NowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    inline HWND GetRobloxWindowCached() {
        const long long now = NowMs();
        const long long last = lastWndLookupMs.load(std::memory_order_relaxed);
        HWND hwnd = cachedRobloxWnd.load(std::memory_order_relaxed);

        if (hwnd && IsWindow(hwnd) && (now - last) < 500)
            return hwnd;

        hwnd = FindWindowW(nullptr, L"Roblox");
        if (!hwnd) hwnd = FindWindowW(nullptr, L"RobloxPlayerBeta");

        cachedRobloxWnd.store(hwnd, std::memory_order_relaxed);
        lastWndLookupMs.store(now, std::memory_order_relaxed);
        return hwnd;
    }

    inline bool IsRobloxFocused() {
        HWND rb = GetRobloxWindowCached();
        if (!rb) return false;
        return GetForegroundWindow() == rb;
    }

    inline bool IsHoldKeyActive() {
        const int key = Vars::AutoClicker::holdKey;
        if (key == 0) return true;
        if (key == 1) return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (key == 2) return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (key == 4) return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (key == 5) return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
        if (key == 6) return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    inline void SendClick() {
        DWORD downFlag = MOUSEEVENTF_LEFTDOWN;
        DWORD upFlag = MOUSEEVENTF_LEFTUP;
        if (Vars::AutoClicker::clickButton == 1) {
            downFlag = MOUSEEVENTF_RIGHTDOWN;
            upFlag = MOUSEEVENTF_RIGHTUP;
        }

        INPUT inputs[2]{};
        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = downFlag;
        inputs[1].type = INPUT_MOUSE;
        inputs[1].mi.dwFlags = upFlag;
        SendInput(2, inputs, sizeof(INPUT));
    }

    inline int EffectiveMaxCps() {
        int cps = Vars::AutoClicker::maxCps;
        if (cps < 20) cps = 20;
        if (cps > 500) cps = 500;
        return cps;
    }

    inline void ClickLoop() {
        int clickCount = 0;
        auto lastSecond = std::chrono::steady_clock::now();
        auto nextClick = std::chrono::steady_clock::now();

        while (threadRunning.load(std::memory_order_relaxed)) {
            if (!Vars::AutoClicker::enabled || Vars::menuOpen) {
                isActive.store(false, std::memory_order_relaxed);
                clicksPerSecond.store(0, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(12));
                continue;
            }

            if (Vars::AutoClicker::onlyFocused && !IsRobloxFocused()) {
                isActive.store(false, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(12));
                continue;
            }

            if (!IsHoldKeyActive()) {
                isActive.store(false, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
                continue;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now < nextClick) {
                const auto waitUs = std::chrono::duration_cast<std::chrono::microseconds>(nextClick - now).count();
                if (waitUs > 1500)
                    std::this_thread::sleep_for(std::chrono::microseconds(waitUs - 500));
                else
                    std::this_thread::yield();
                continue;
            }

            isActive.store(true, std::memory_order_relaxed);
            SendClick();

            clickCount++;
            const auto statsNow = std::chrono::steady_clock::now();
            const auto statsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(statsNow - lastSecond).count();
            if (statsElapsed >= 1000) {
                clicksPerSecond.store(clickCount, std::memory_order_relaxed);
                clickCount = 0;
                lastSecond = statsNow;
            }

            const int cps = EffectiveMaxCps();
            const auto interval = std::chrono::nanoseconds(1000000000LL / cps);
            nextClick = statsNow + interval;
        }

        isActive.store(false, std::memory_order_relaxed);
        clicksPerSecond.store(0, std::memory_order_relaxed);
    }

    inline void RunThread() {
        if (threadRunning.exchange(true)) return;

        std::thread([]() {
            ClickLoop();
        }).detach();
    }

    inline void StopThread() {
        threadRunning.store(false);
        isActive.store(false);
        clicksPerSecond.store(0);
        cachedRobloxWnd.store(nullptr);
    }
}
