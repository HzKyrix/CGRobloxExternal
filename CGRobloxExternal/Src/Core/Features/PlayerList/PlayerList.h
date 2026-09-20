#pragma once
#include "../../../Game/SDK/SDK.h"
#include "../../Cache/Cache.h"
#include "../../Globals/Globals.h"
#include "../../Vars/Vars.h"
#include <Windows.h>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>

namespace PlayerListFeature {

    struct BringHoldState {
        uintptr_t playerAddr = 0;
        int ticksLeft = 0;
    };

    inline BringHoldState bringHold{};

    inline void CopyToClipboard(const std::string& text) {
        if (text.empty()) return;
        if (!OpenClipboard(nullptr)) return;
        EmptyClipboard();
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hg) {
            memcpy(GlobalLock(hg), text.c_str(), text.size() + 1);
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
        }
        CloseClipboard();
    }

    inline const PlayerCache::CachedPlayer* FindCached(uintptr_t playerAddr) {
        for (auto& plr : PlayerCache::players) {
            if (plr.isValid && plr.playerAddr == playerAddr)
                return &plr;
        }
        return nullptr;
    }

    inline bool IsMovablePartClass(const std::string& cls) {
        return cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" || cls == "TrussPart";
    }

    inline RBX::Vec3 Sub(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X - b.X, a.Y - b.Y, a.Z - b.Z };
    }

    inline RBX::Vec3 Add(const RBX::Vec3& a, const RBX::Vec3& b) {
        return { a.X + b.X, a.Y + b.Y, a.Z + b.Z };
    }

    inline RBX::RbxInstance GetLocalHRP() {
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return RBX::RbxInstance(0);
        return character.FindChild("HumanoidRootPart");
    }

    inline RBX::Vec3 NormalizeHorizontal(const RBX::Vec3& v) {
        float len = sqrtf(v.X * v.X + v.Z * v.Z);
        if (len < 0.0001f) return {};
        return { v.X / len, 0.f, v.Z / len };
    }

    inline bool SetPartWorldPosition(RBX::RbxInstance part, const RBX::Vec3& dest) {
        if (part.Addr == 0) return false;

        uintptr_t prim = part.GetPrimitivePtr();
        if (prim == 0) return false;

        RBX::CFrame cf = part.GetCFrame();
        cf.data[9] = dest.X;
        cf.data[10] = dest.Y;
        cf.data[11] = dest.Z;

        Coms->WriteMemory(prim + offsets::Position, dest);
        Coms->WriteMemory(prim + offsets::CFrame, cf);

        RBX::Vec3 zero{};
        Coms->WriteMemory(prim + offsets::Velocity, zero);
        return true;
    }

    inline void CollectCharacterParts(RBX::RbxInstance& character, std::vector<RBX::RbxInstance>& out) {
        if (character.Addr == 0) return;

        for (auto& child : character.GetChildList()) {
            if (child.Addr == 0) continue;

            std::string cls = child.GetClass();
            if (IsMovablePartClass(cls)) {
                out.push_back(child);
                continue;
            }

            if (cls == "Accessory") {
                auto handle = child.FindChild("Handle");
                if (handle.Addr != 0) out.push_back(handle);
            }
        }
    }

    inline RBX::RbxInstance GetTargetHRP(const PlayerCache::CachedPlayer& plr) {
        if (plr.rootPartAddr != 0)
            return RBX::RbxInstance(plr.rootPartAddr);

        if (plr.characterAddr == 0) return RBX::RbxInstance(0);
        return RBX::RbxInstance(plr.characterAddr).FindChild("HumanoidRootPart");
    }

    inline RBX::Vec3 GetBringDestination() {
        auto localHrp = GetLocalHRP();
        if (localHrp.Addr == 0) return {};

        RBX::Vec3 myPos = localHrp.GetPos();
        RBX::CFrame hrpCf = localHrp.GetCFrame();
        RBX::Vec3 side = NormalizeHorizontal(hrpCf.GetRightVector());

        if (side.X == 0.f && side.Z == 0.f && Globals::camera.Addr != 0) {
            RBX::CFrame cam = Coms->ReadMemory<RBX::CFrame>(Globals::camera.Addr + offsets::CameraRotation);
            side = NormalizeHorizontal(cam.GetRightVector());
        }
        if (side.X == 0.f && side.Z == 0.f)
            side = { 1.f, 0.f, 0.f };

        return {
            myPos.X + side.X * Vars::Players::bringSideOffset,
            myPos.Y + Vars::Players::bringOffsetY,
            myPos.Z + side.Z * Vars::Players::bringSideOffset
        };
    }

    inline bool TeleportCharacterRig(const PlayerCache::CachedPlayer& plr, const RBX::Vec3& destHrpPos) {
        RBX::RbxInstance hrp = GetTargetHRP(plr);
        if (hrp.Addr == 0) return false;

        RBX::Vec3 currentHrpPos = hrp.GetPos();
        RBX::Vec3 delta = Sub(destHrpPos, currentHrpPos);

        bool moved = false;

        if (!plr.bodyParts.empty()) {
            for (uintptr_t addr : plr.bodyParts) {
                RBX::RbxInstance part(addr);
                if (part.Addr == 0) continue;
                RBX::Vec3 newPos = Add(part.GetPos(), delta);
                if (SetPartWorldPosition(part, newPos)) moved = true;
            }
        }

        if (plr.characterAddr != 0) {
            RBX::RbxInstance character(plr.characterAddr);
            std::vector<RBX::RbxInstance> parts;
            CollectCharacterParts(character, parts);
            for (auto& part : parts) {
                RBX::Vec3 newPos = Add(part.GetPos(), delta);
                if (SetPartWorldPosition(part, newPos)) moved = true;
            }
        }

        if (!moved)
            moved = SetPartWorldPosition(hrp, destHrpPos);

        return moved;
    }

    inline bool TeleportToPosition(const RBX::Vec3& target) {
        auto hrp = GetLocalHRP();
        if (hrp.Addr == 0) return false;
        return SetPartWorldPosition(hrp, target);
    }

    inline bool TeleportToPlayer(uintptr_t characterAddr) {
        if (characterAddr == 0) return false;
        RBX::RbxInstance character(characterAddr);
        auto targetHrp = character.FindChild("HumanoidRootPart");
        if (targetHrp.Addr == 0) return false;

        RBX::Vec3 pos = targetHrp.GetPos();
        pos.Y += Vars::Players::teleportOffsetY;
        return TeleportToPosition(pos);
    }

    inline bool TeleportToCached(const PlayerCache::CachedPlayer& plr) {
        if (plr.characterAddr != 0)
            return TeleportToPlayer(plr.characterAddr);
        if (plr.rootPartAddr != 0) {
            RBX::Vec3 pos = RBX::RbxInstance(plr.rootPartAddr).GetPos();
            pos.Y += Vars::Players::teleportOffsetY;
            return TeleportToPosition(pos);
        }
        return false;
    }

    inline bool BringCachedToMe(const PlayerCache::CachedPlayer& plr) {
        if (GetLocalHRP().Addr == 0) return false;
        RBX::Vec3 dest = GetBringDestination();
        return TeleportCharacterRig(plr, dest);
    }

    inline void StartBringHold(uintptr_t playerAddr, int ticks = 150) {
        bringHold.playerAddr = playerAddr;
        bringHold.ticksLeft = ticks;
    }

    inline void BeginBringCached(const PlayerCache::CachedPlayer& plr) {
        BringCachedToMe(plr);
        StartBringHold(plr.playerAddr, 150);
    }

    inline void UpdateBringHold() {
        if (Vars::Players::bringHoldActive && Vars::Players::selectedAddr != 0) {
            const PlayerCache::CachedPlayer* plr = FindCached(Vars::Players::selectedAddr);
            if (plr && plr->isValid)
                BringCachedToMe(*plr);
        }

        if (bringHold.ticksLeft <= 0) return;

        const PlayerCache::CachedPlayer* plr = FindCached(bringHold.playerAddr);
        if (!plr || !plr->isValid) {
            bringHold.ticksLeft = 0;
            return;
        }

        BringCachedToMe(*plr);
        bringHold.ticksLeft--;
    }

    inline std::string GetPlayerTeamName(uintptr_t playerAddr) {
        if (playerAddr == 0) return "None";
        RBX::RbxInstance player(playerAddr);
        uintptr_t teamAddr = Coms->ReadMemory<uintptr_t>(player.Addr + offsets::Team);
        if (teamAddr == 0) return "None";
        return RBX::RbxInstance(teamAddr).GetName();
    }
}
