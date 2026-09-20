#pragma once
#include "../../../Game/SDK/SDK.h"
#include "../../Globals/Globals.h"
#include "../../Vars/Vars.h"
#include <Windows.h>
#include <cmath>
#include <string>

namespace Movement {

    inline RBX::Vec3 Normalize(const RBX::Vec3& v) {
        float len = sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
        if (len < 0.0001f) return {};
        return { v.X / len, v.Y / len, v.Z / len };
    }

    inline RBX::Vec3 Add(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X + b.X, a.Y + b.Y, a.Z + b.Z };
    }

    inline RBX::Vec3 Scale(const RBX::Vec3& v, float s) {
        return { v.X * s, v.Y * s, v.Z * s };
    }

    inline bool IsCharacterPart(const std::string& cls) {
        return cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" || cls == "TrussPart";
    }

    inline void SetPartCanCollide(uintptr_t partAddr, bool canCollide) {
        if (partAddr == 0) return;
        uintptr_t prim = Coms->ReadMemory<uintptr_t>(partAddr + offsets::Primitive);
        if (prim == 0) return;

        uint8_t flags = Coms->ReadMemory<uint8_t>(prim + offsets::PrimitiveFlags);
        if (canCollide)
            flags |= static_cast<uint8_t>(offsets::CanCollideMask);
        else
            flags &= ~static_cast<uint8_t>(offsets::CanCollideMask);
        Coms->WriteMemory(prim + offsets::PrimitiveFlags, flags);
    }

    inline void ApplyNoclip(bool enabled) {
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return;

        for (auto& child : character.GetChildList()) {
            if (!IsCharacterPart(child.GetClass())) continue;
            SetPartCanCollide(child.Addr, !enabled);
        }
    }

    inline RBX::CFrame GetCameraRotation() {
        if (Globals::camera.Addr == 0) return {};
        return Coms->ReadMemory<RBX::CFrame>(Globals::camera.Addr + offsets::CameraRotation);
    }

    inline void WriteHRPPosition(RBX::RbxInstance& hrp, const RBX::Vec3& pos) {
        uintptr_t prim = hrp.GetPrimitivePtr();
        if (prim == 0) return;

        RBX::CFrame cf = hrp.GetCFrame();
        cf.data[9] = pos.X;
        cf.data[10] = pos.Y;
        cf.data[11] = pos.Z;

        Coms->WriteMemory(prim + offsets::Position, pos);
        Coms->WriteMemory(prim + offsets::CFrame, cf);

        RBX::Vec3 zero{};
        Coms->WriteMemory(prim + offsets::Velocity, zero);
    }

    inline void RunFly(float deltaTime) {
        if (!Vars::Local::flyEnabled) return;

        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return;

        auto hrp = character.FindChild("HumanoidRootPart");
        if (hrp.Addr == 0) return;

        RBX::CFrame cam = GetCameraRotation();
        RBX::Vec3 look = cam.GetLookVector();
        RBX::Vec3 right = cam.GetRightVector();

        RBX::Vec3 move{};
        if (GetAsyncKeyState('W') & 0x8000) move = Add(move, look);
        if (GetAsyncKeyState('S') & 0x8000) move = Add(move, Scale(look, -1.f));
        if (GetAsyncKeyState('A') & 0x8000) move = Add(move, Scale(right, -1.f));
        if (GetAsyncKeyState('D') & 0x8000) move = Add(move, right);
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) move.Y += 1.f;
        if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000))
            move.Y -= 1.f;

        move = Normalize(move);
        if (move.X == 0.f && move.Y == 0.f && move.Z == 0.f) {
            uintptr_t prim = hrp.GetPrimitivePtr();
            if (prim != 0) {
                RBX::Vec3 zero{};
                Coms->WriteMemory(prim + offsets::Velocity, zero);
            }
            return;
        }

        RBX::Vec3 pos = hrp.GetPos();
        float step = Vars::Local::flySpeed * deltaTime;
        pos = Add(pos, Scale(move, step));
        WriteHRPPosition(hrp, pos);
    }

    inline void RunNoclip() {
        static bool lastState = false;
        if (Vars::Local::noclipEnabled != lastState || Vars::Local::noclipEnabled) {
            ApplyNoclip(Vars::Local::noclipEnabled);
            lastState = Vars::Local::noclipEnabled;
        }
    }

    inline void Update(float deltaTime) {
        RunNoclip();
        RunFly(deltaTime);
    }
}
