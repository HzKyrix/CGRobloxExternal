#pragma once
#include "../../Vars/Vars.h"
#include "../../../Render/Fonts.h"
#include "../../../Render/UiTheme.h"
#include "../../../Render/ImGui/imgui.h"
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

namespace Notify {

    enum class Type {
        Info,
        Success,
        Warning,
        Error
    };

    struct Toast {
        std::string title;
        std::string message;
        Type type = Type::Info;
        std::chrono::steady_clock::time_point created{};
        float duration = 4.f;
        ImGuiID animId = 0;
    };

    inline std::vector<Toast> toasts;
    inline ImGuiID nextAnimId = 1;

    inline int TypeIndex(Type type) {
        switch (type) {
            case Type::Success: return 1;
            case Type::Warning: return 2;
            case Type::Error: return 3;
            default: return 0;
        }
    }

    inline void Push(const std::string& title, const std::string& message, Type type = Type::Info, float duration = 4.f) {
        if (!Vars::Misc::notificationsEnabled) return;

        Toast t;
        t.title = title;
        t.message = message;
        t.type = type;
        t.created = std::chrono::steady_clock::now();
        t.duration = duration;
        t.animId = nextAnimId++;

        toasts.push_back(t);
        if (toasts.size() > 8)
            toasts.erase(toasts.begin());
    }

    inline void Update() {
        if (toasts.empty()) return;

        auto now = std::chrono::steady_clock::now();
        toasts.erase(
            std::remove_if(toasts.begin(), toasts.end(),
                [&](const Toast& t) {
                    float elapsed = std::chrono::duration<float>(now - t.created).count();
                    return elapsed >= t.duration + 0.35f;
                }),
            toasts.end()
        );
    }

    inline void Render() {
        if (!Vars::Misc::notificationsEnabled || toasts.empty()) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 display = ImGui::GetIO().DisplaySize;
        auto now = std::chrono::steady_clock::now();

        ImFont* titleFont = AppFonts::Bold ? AppFonts::Bold : ImGui::GetFont();
        ImFont* bodyFont = AppFonts::Regular ? AppFonts::Regular : ImGui::GetFont();
        float titleSize = AppFonts::FontSize(titleFont, 15.f);
        float bodySize = AppFonts::FontSize(bodyFont, 14.f);

        const float padX = 18.f;
        const float toastW = 360.f;
        const float toastH = 78.f;
        const float gap = 10.f;
        const float baseY = 82.f;

        float y = baseY;

        for (size_t i = 0; i < toasts.size(); ++i) {
            const Toast& t = toasts[i];
            float elapsed = std::chrono::duration<float>(now - t.created).count();
            float remain = t.duration - elapsed;
            if (remain < 0.f) remain = 0.f;

            float slideIn = 1.f;
            if (elapsed < 0.28f)
                slideIn = UiTheme::SmoothStep(elapsed / 0.28f);

            float fadeOut = 1.f;
            if (remain < 0.45f)
                fadeOut = remain / 0.45f;
            if (elapsed < 0.15f)
                fadeOut *= elapsed / 0.15f;

            int alpha = static_cast<int>(245.f * fadeOut * slideIn);
            if (alpha < 8) continue;

            float slideX = (1.f - slideIn) * 52.f;
            float x = display.x - toastW - padX + slideX;
            ImVec2 a(x, y);
            ImVec2 b(x + toastW, y + toastH);

            UiTheme::DrawSoftShadow(dl, a, b, 14.f, 2);
            UiTheme::DrawGlassPanel(dl, a, b, 14.f,
                UiTheme::Col(10, 12, 18, alpha), UiTheme::Col(255, 255, 255, alpha / 14));

            ImU32 accent = UiTheme::TypeAccent(TypeIndex(t.type), alpha);
            UiTheme::DrawAccentBar(dl, ImVec2(a.x, a.y + 12.f), toastH - 24.f,
                accent, UiTheme::LerpCol(accent, UiTheme::Col(90, 160, 255, alpha), 0.45f));

            float iconR = 17.f;
            ImVec2 iconCenter(a.x + 30.f, a.y + toastH * 0.5f);
            dl->AddCircleFilled(iconCenter, iconR, UiTheme::Col(255, 255, 255, alpha / 16));
            dl->AddCircle(iconCenter, iconR, accent, 0, 1.5f);
            dl->AddText(titleFont, titleSize * 0.82f,
                ImVec2(iconCenter.x - 4.f, iconCenter.y - 8.f), accent,
                UiTheme::TypeIcon(TypeIndex(t.type)));

            dl->AddText(titleFont, titleSize,
                ImVec2(a.x + 56.f, a.y + 14.f), UiTheme::Col(246, 248, 252, alpha), t.title.c_str());

            ImVec2 msgSize = UiTheme::TextSize(bodyFont, bodySize, t.message.c_str());
            if (msgSize.x > toastW - 72.f) {
                std::string clipped = t.message.substr(0, 48) + "...";
                dl->AddText(bodyFont, bodySize,
                    ImVec2(a.x + 56.f, a.y + 38.f), UiTheme::Col(165, 172, 188, alpha), clipped.c_str());
            } else {
                dl->AddText(bodyFont, bodySize,
                    ImVec2(a.x + 56.f, a.y + 38.f), UiTheme::Col(165, 172, 188, alpha), t.message.c_str());
            }

            float progress = remain / t.duration;
            UiTheme::DrawProgressBar(dl,
                ImVec2(a.x + 56.f, b.y - 13.f),
                ImVec2(b.x - 16.f, b.y - 9.f),
                progress,
                UiTheme::Col(255, 255, 255, alpha / 12),
                accent);

            y += toastH + gap;
        }
    }
}
