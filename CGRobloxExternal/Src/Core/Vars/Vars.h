#pragma once

namespace Vars {
    inline bool menuOpen = false;
    inline int selectedTab = 0;

    namespace Aimbot {
        inline bool enabled = false;
        inline bool showFOV = false;
        inline float fovRadius = 100.0f;
        inline float smoothing = 5.0f;
        inline int aimTarget = 0;
        inline int aimMethod = 0;
        inline int aimbotKey = 2;
    }

    namespace TriggerBot {
        inline bool enabled = false;
        inline int triggerKey = 0;
        inline int hitPart = 0;
        inline float triggerRadius = 14.0f;
        inline float hitboxPad = 2.0f;
        inline float maxDistance = 400.0f;
        inline float delayMs = 0.0f;
        inline float cooldownMs = 75.0f;
        inline bool teamCheck = true;
        inline bool ignoreDead = true;
        inline bool skipWhileFiring = true;
    }

    namespace ESP {
        inline bool enabled = false;
        inline bool boxes = false;
        inline bool names = false;
        inline bool distance = false;
        inline bool healthBar = false;
        inline bool skeleton = false;
        inline bool tracers = false;
        inline int tracerOrigin = 0;
        inline float smoothSpeed = 28.0f;
        inline float boxPadding = 0.0f;
        inline float boxWidthRatio = 0.28f;
        inline float screenStabilize = 0.55f;
        inline float screenBeta = 0.14f;
        inline bool stableBox = true;
    }

    namespace Chams {
        inline bool enabled = false;
        inline bool fill = true;
        inline bool outline = true;
        inline bool glow = true;
        inline bool showRoot = false;
        inline bool teamColor = true;
        inline float opacity = 0.42f;
        inline float colorR = 0.55f;
        inline float colorG = 0.38f;
        inline float colorB = 1.00f;
    }

    namespace Local {
        inline bool speedEnabled = false;
        inline float walkSpeed = 16.0f;
        inline bool jumpEnabled = false;
        inline float jumpPower = 50.0f;
        inline bool flyEnabled = false;
        inline float flySpeed = 50.0f;
        inline bool noclipEnabled = false;
    }

    namespace Teleport {
        inline bool clickTpEnabled = false;
        inline int clickTpKey = 18;
        inline int clickTpMode = 1;
        inline float clickTpDistance = 250.0f;
        inline float clickTpOffsetY = 3.0f;
        inline float clickPartRadius = 28.0f;
        inline float clickTpMaxScan = 600.0f;
        inline float stageEndOffsetY = 3.0f;
        inline bool clickTpNotify = false;
    }

    namespace Players {
        inline uintptr_t selectedAddr = 0;
        inline float teleportOffsetY = 3.0f;
        inline float bringSideOffset = 4.0f;
        inline float bringOffsetY = 0.0f;
        inline bool bringHoldActive = false;
        inline char searchBuf[64]{};
    }

    namespace MM2 {
        inline bool enabled = false;
        inline bool roleESP = true;
        inline bool showSheriff = true;
        inline bool murdererHUD = true;
    }

    namespace BladeBall {
        inline bool enabled = false;
        inline bool requireHighlight = true;
        inline bool useClick = false;
        inline bool proMode = true;
        inline bool burstParry = true;
        inline bool dualInput = true;
        inline float parryDistance = 35.0f;
        inline float parryTiming = 0.18f;
        inline float distanceOffset = 5.0f;
        inline float timingScale = 1.0f;
        inline float reactionMs = 20.0f;
        inline float minBallSpeed = 25.0f;
        inline float maxTrackDistance = 80.0f;
        inline float directionDot = 0.65f;
        inline float cooldownMs = 90.0f;
        inline float clashDistance = 14.0f;
        inline float emergencyDistance = 10.0f;
        inline float pingCompensation = 0.045f;
        inline int parryKey = 0x46;
    }

    namespace Config {
        inline char nameBuf[64] = "default";
        inline int selectedIndex = 0;
    }

    namespace Misc {
        inline bool antiAfkEnabled = false;
        inline float antiAfkInterval = 55.0f;
        inline bool antiAfkNotify = false;
        inline bool notificationsEnabled = true;
    }

    namespace AutoClicker {
        inline bool enabled = false;
        inline int holdKey = 0;
        inline int clickButton = 0;
        inline bool onlyFocused = true;
        inline int maxCps = 120;
    }
}
