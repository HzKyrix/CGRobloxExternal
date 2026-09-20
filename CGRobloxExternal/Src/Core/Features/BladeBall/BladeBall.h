#pragma once
#include "../../../Game/SDK/SDK.h"
#include "../../Globals/Globals.h"
#include "../../Vars/Vars.h"
#include "../Misc/Notifications.h"
#include <Windows.h>
#include <chrono>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>

namespace BladeBall {

    inline std::atomic<bool> threadRunning{ false };

    inline bool ballDetected = false;
    inline bool inArena = false;
    inline float trackedDistance = 0.f;
    inline float trackedSpeed = 0.f;
    inline float trackedTimeToHit = 0.f;
    inline float trackedThreshold = 0.f;
    inline int trackedBallCount = 0;
    inline std::string statusText = "Idle";

    struct VelTrack {
        RBX::Vec3 pos{};
        double time = 0.0;
    };

    inline std::unordered_map<uintptr_t, VelTrack> s_velTrack;

    inline RBX::Vec3 Sub(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X - b.X, a.Y - b.Y, a.Z - b.Z };
    }

    inline float Len(const RBX::Vec3& v) {
        return sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    }

    inline RBX::Vec3 Norm(const RBX::Vec3& v) {
        float l = Len(v);
        if (l < 0.0001f) return {};
        return { v.X / l, v.Y / l, v.Z / l };
    }

    inline float Dot(const RBX::Vec3& a, const RBX::Vec3& b) {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    inline double Now() {
        return std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    inline RBX::Vec3 ReadVel(RBX::RbxInstance part) {
        uintptr_t prim = part.GetPrimitivePtr();
        if (prim == 0) return {};
        return Coms->ReadMemory<RBX::Vec3>(prim + offsets::Velocity);
    }

    inline RBX::Vec3 GetVelocity(RBX::RbxInstance part, const RBX::Vec3& pos) {
        RBX::Vec3 mem = ReadVel(part);
        auto& tr = s_velTrack[part.Addr];
        RBX::Vec3 best = mem;

        if (tr.time > 0.0) {
            double dt = Now() - tr.time;
            if (dt > 0.0002 && dt < 0.2) {
                RBX::Vec3 inst{
                    static_cast<float>((pos.X - tr.pos.X) / dt),
                    static_cast<float>((pos.Y - tr.pos.Y) / dt),
                    static_cast<float>((pos.Z - tr.pos.Z) / dt)
                };
                if (Len(inst) > Len(best))
                    best = inst;
            }
        }

        tr.pos = pos;
        tr.time = Now();
        return best;
    }

    inline RBX::RbxInstance ResolvePart(RBX::RbxInstance inst) {
        if (inst.Addr == 0) return RBX::RbxInstance(0);
        std::string cls = inst.GetClass();
        if (cls == "Part" || cls == "MeshPart" || cls == "UnionOperation")
            return inst;
        auto p = inst.FindChildByClass("Part");
        if (p.Addr == 0) p = inst.FindChildByClass("MeshPart");
        return p;
    }

    inline RBX::RbxInstance GetBallsFolder() {
        if (Globals::workspace.Addr == 0) return RBX::RbxInstance(0);
        return Globals::workspace.FindChild("Balls");
    }

    inline bool IsInArena() {
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return false;
        return character.FindChild("Highlight").Addr != 0;
    }

    inline bool CanPlay() {
        if (!Vars::BladeBall::enabled) return false;
        if (Globals::localPlayer.Addr == 0) return false;
        return IsInArena();
    }

    inline HWND GetRobloxWindow() {
        HWND w = FindWindowW(nullptr, L"Roblox");
        if (w) return w;
        return FindWindowW(nullptr, L"RobloxPlayerBeta");
    }

    inline void SendParryInput() {
        HWND roblox = GetRobloxWindow();
        if (roblox) {
            DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
            DWORD rbThread = GetWindowThreadProcessId(roblox, nullptr);
            if (fgThread != rbThread) {
                AttachThreadInput(fgThread, rbThread, TRUE);
                SetForegroundWindow(roblox);
                AttachThreadInput(fgThread, rbThread, FALSE);
            } else {
                SetForegroundWindow(roblox);
            }
        }

        auto tapKey = [](WORD vk) {
            INPUT down{};
            down.type = INPUT_KEYBOARD;
            down.ki.wVk = vk;
            INPUT up{};
            up.type = INPUT_KEYBOARD;
            up.ki.wVk = vk;
            up.ki.dwFlags = KEYEVENTF_KEYUP;
            INPUT arr[2] = { down, up };
            SendInput(2, arr, sizeof(INPUT));
        };

        auto tapClick = []() {
            INPUT down{};
            down.type = INPUT_MOUSE;
            down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            INPUT up{};
            up.type = INPUT_MOUSE;
            up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            INPUT arr[2] = { down, up };
            SendInput(2, arr, sizeof(INPUT));
        };

        if (Vars::BladeBall::useClick || Vars::BladeBall::dualInput)
            tapClick();

        WORD key = static_cast<WORD>(Vars::BladeBall::parryKey);
        if (key == 0) key = 0x46;

        if (!Vars::BladeBall::useClick || Vars::BladeBall::dualInput)
            tapKey(key);
    }

    struct BallThreat {
        RBX::RbxInstance part{ 0 };
        float distance = 0.f;
        float speed = 0.f;
        float toward = 0.f;
        float aimDot = 0.f;
        float timeToHit = 999.f;
        float threshold = 0.f;
    };

    inline void CollectActiveBalls(std::vector<RBX::RbxInstance>& out, RBX::RbxInstance parent, int depth = 0) {
        if (parent.Addr == 0 || depth > 5) return;

        for (auto& child : parent.GetChildList()) {
            if (child.Addr == 0) continue;

            if (child.FindChild("zoomies").Addr != 0) {
                RBX::RbxInstance part = ResolvePart(child);
                if (part.Addr != 0) out.push_back(part);
                continue;
            }

            std::string cls = child.GetClass();
            if (cls == "Model" || cls == "Folder")
                CollectActiveBalls(out, child, depth + 1);
        }
    }

    inline float GetMinAimDot() {
        float dot = Vars::BladeBall::directionDot;
        if (Vars::BladeBall::requireHighlight) dot += 0.1f;
        if (dot < 0.55f) dot = 0.55f;
        if (dot > 0.92f) dot = 0.92f;
        return dot;
    }

    inline float CalcThreshold(float speed) {
        float capped = speed - 9.5f;
        if (capped < 0.f) capped = 0.f;
        if (capped > 650.f) capped = 650.f;

        float divisor = 2.4f + capped * 0.002f;
        float t = Vars::BladeBall::pingCompensation + (speed / divisor);
        t *= Vars::BladeBall::timingScale;
        t += Vars::BladeBall::parryTiming * 0.5f;
        t += Vars::BladeBall::reactionMs * 0.001f;

        if (t < 0.08f) t = 0.08f;
        if (t > 0.32f) t = 0.32f;
        return t;
    }

    inline BallThreat FindThreat(const RBX::Vec3& playerPos, const RBX::Vec3& playerVel) {
        BallThreat best{};
        best.timeToHit = 999.f;

        std::vector<RBX::RbxInstance> balls;
        auto folder = GetBallsFolder();
        if (folder.Addr != 0)
            CollectActiveBalls(balls, folder);

        trackedBallCount = static_cast<int>(balls.size());
        if (balls.empty()) return best;

        const float minDot = GetMinAimDot();
        const float maxDist = Vars::BladeBall::maxTrackDistance;

        for (auto& part : balls) {
            if (part.Addr == 0) continue;

            RBX::Vec3 pos = part.GetPos();
            float dist = Len(Sub(playerPos, pos));
            if (dist > maxDist) continue;

            RBX::Vec3 vel = GetVelocity(part, pos);
            float speed = Len(vel);
            if (speed < Vars::BladeBall::minBallSpeed) continue;

            if (dist > 50.f && speed < 40.f) continue;

            RBX::Vec3 toPlayer = Norm(Sub(playerPos, pos));
            RBX::Vec3 velDir = Norm(vel);
            float dot = Dot(toPlayer, velDir);
            if (dot < minDot) continue;

            float toward = Dot(vel, toPlayer) - Dot(playerVel, toPlayer);
            if (toward < 8.f) continue;

            float hitDist = dist - Vars::BladeBall::distanceOffset;
            if (hitDist < 0.f) hitDist = 0.f;

            float timeToHit = hitDist / toward;
            if (timeToHit < 0.f) timeToHit = 0.f;
            if (timeToHit > 2.5f) continue;

            if (timeToHit < best.timeToHit) {
                best.part = part;
                best.distance = dist;
                best.speed = speed;
                best.toward = toward;
                best.aimDot = dot;
                best.timeToHit = timeToHit;
                best.threshold = CalcThreshold(speed);
            }
        }

        return best;
    }

    inline bool ShouldParry(const BallThreat& ball) {
        if (ball.part.Addr == 0) return false;

        if (ball.distance > Vars::BladeBall::maxTrackDistance) return false;
        if (ball.aimDot < GetMinAimDot()) return false;

        if (ball.distance <= Vars::BladeBall::emergencyDistance &&
            ball.speed > 35.f && ball.aimDot > 0.75f)
            return true;

        if (ball.timeToHit > ball.threshold) return false;
        if (ball.timeToHit < 0.02f) return false;

        return ball.distance <= Vars::BladeBall::parryDistance + 15.f;
    }

    inline void Update() {
        ballDetected = false;
        inArena = false;
        trackedDistance = 0.f;
        trackedSpeed = 0.f;
        trackedTimeToHit = 0.f;
        trackedThreshold = 0.f;

        if (!Vars::BladeBall::enabled) {
            statusText = "Disabled";
            return;
        }

        inArena = IsInArena();
        if (!inArena) {
            statusText = "Lobby - waiting for round";
            return;
        }

        auto folder = GetBallsFolder();
        if (folder.Addr == 0) {
            statusText = "No Balls folder";
            return;
        }

        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) {
            statusText = "No character";
            return;
        }

        auto hrp = character.FindChild("HumanoidRootPart");
        if (hrp.Addr == 0) {
            statusText = "No HRP";
            return;
        }

        RBX::Vec3 playerPos = hrp.GetPos();
        RBX::Vec3 playerVel = ReadVel(hrp);

        BallThreat threat = FindThreat(playerPos, playerVel);
        if (threat.part.Addr == 0) {
            statusText = "Safe - no ball targeting you";
            return;
        }

        ballDetected = true;
        trackedDistance = threat.distance;
        trackedSpeed = threat.speed;
        trackedTimeToHit = threat.timeToHit;
        trackedThreshold = threat.threshold;

        static auto lastParry = std::chrono::steady_clock::now() - std::chrono::seconds(1);

        if (!ShouldParry(threat)) {
            statusText = "Track " + std::to_string(static_cast<int>(threat.distance)) + "m | T=" +
                std::to_string(threat.timeToHit).substr(0, 4) + "s";
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastParry).count();
        if (ms < static_cast<long long>(Vars::BladeBall::cooldownMs)) {
            statusText = "Cooldown";
            return;
        }

        SendParryInput();
        lastParry = now;
        statusText = "PARRIED";
        Notify::Push("Blade Ball", "Auto parry triggered", Notify::Type::Success, 2.f);

        if (Vars::BladeBall::burstParry)
            SendParryInput();
    }

    inline void RunThread() {
        if (threadRunning.exchange(true)) return;

        std::thread([]() {
            while (threadRunning.load()) {
                if (Coms->IsConnected() && Globals::players.Addr != 0)
                    Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }).detach();
    }

    inline void StopThread() {
        threadRunning.store(false);
    }
}
