#pragma once
#include <cmath>
#include <algorithm>
#include <string>

namespace ScreenFilter {

    struct OneEuro1D {
        float xPrev = 0.f;
        float dxPrev = 0.f;
        bool init = false;

        void Reset() { init = false; }

        float Filter(float x, float dt, float mincutoff, float beta, float dcutoff = 1.0f) {
            if (dt < 0.00005f) dt = 0.00005f;
            if (!init) {
                xPrev = x;
                dxPrev = 0.f;
                init = true;
                return x;
            }

            const float dx = (x - xPrev) / dt;
            const float tauD = 1.f / (2.f * 3.14159265f * dcutoff);
            const float aD = 1.f / (1.f + tauD / dt);
            const float dxHat = aD * dx + (1.f - aD) * dxPrev;
            const float cutoff = mincutoff + beta * fabsf(dxHat);
            const float tau = 1.f / (2.f * 3.14159265f * cutoff);
            const float a = 1.f / (1.f + tau / dt);
            const float result = a * x + (1.f - a) * xPrev;
            xPrev = result;
            dxPrev = dxHat;
            return result;
        }
    };

    struct ScreenBoxFilter {
        OneEuro1D cx, cy, w, h;

        void Reset() {
            cx.Reset(); cy.Reset(); w.Reset(); h.Reset();
        }

        void Apply(float& minX, float& minY, float& maxX, float& maxY, float dt,
            float mincutoff, float beta)
        {
            float centerX = (minX + maxX) * 0.5f;
            float centerY = (minY + maxY) * 0.5f;
            float width = maxX - minX;
            float height = maxY - minY;

            centerX = cx.Filter(centerX, dt, mincutoff, beta * 1.15f);
            centerY = cy.Filter(centerY, dt, mincutoff, beta * 1.15f);
            width = w.Filter(width, dt, mincutoff * 3.5f, beta * 0.35f);
            height = h.Filter(height, dt, mincutoff * 3.5f, beta * 0.35f);

            width = (std::max)(width, 6.f);
            height = (std::max)(height, 10.f);

            minX = centerX - width * 0.5f;
            maxX = centerX + width * 0.5f;
            minY = centerY - height * 0.5f;
            maxY = centerY + height * 0.5f;

            minX = floorf(minX * 2.f + 0.5f) / 2.f;
            minY = floorf(minY * 2.f + 0.5f) / 2.f;
            maxX = floorf(maxX * 2.f + 0.5f) / 2.f;
            maxY = floorf(maxY * 2.f + 0.5f) / 2.f;
        }
    };

    struct ScreenPointFilter {
        OneEuro1D fx, fy;

        void Reset() { fx.Reset(); fy.Reset(); }

        void Apply(float& x, float& y, float dt, float mincutoff, float beta) {
            x = fx.Filter(x, dt, mincutoff, beta);
            y = fy.Filter(y, dt, mincutoff, beta);
            x = floorf(x * 2.f + 0.5f) / 2.f;
            y = floorf(y * 2.f + 0.5f) / 2.f;
        }
    };

    inline bool IsAnchorPart(const std::string& name) {
        return name == "Head" || name == "Torso" || name == "UpperTorso" || name == "LowerTorso" ||
            name == "LeftFoot" || name == "RightFoot" || name == "Left Leg" || name == "Right Leg" ||
            name == "HumanoidRootPart";
    }
}
