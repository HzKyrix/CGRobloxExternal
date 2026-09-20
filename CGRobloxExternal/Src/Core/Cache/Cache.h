#pragma once
#include "../../Game/SDK/SDK.h"
#include "../Globals/Globals.h"
#include "../Vars/Vars.h"
#include "../Features/MM2/MM2.h"
#include "../Features/Visuals/ScreenFilter.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

namespace PlayerCache {

    struct PartCache {
        uintptr_t addr = 0;
        std::string name;
        RBX::Vec3 size{};
        RBX::Vec3 pos{};
        RBX::Vec3 smoothPos{};
        RBX::CFrame cf{};
        RBX::CFrame smoothCf{};
        bool hasSmooth = false;
    };

    struct EspAnchors {
        float headTop = 0.f;
        float footY = 0.f;
        float centerX = 0.f;
        float centerZ = 0.f;
        float bodyHeight = 5.f;
        bool ready = false;
    };

    struct CachedPlayer {
        uintptr_t playerAddr = 0;
        uintptr_t characterAddr = 0;
        uintptr_t humanoidAddr = 0;
        uintptr_t rootPartAddr = 0;

        std::string name;
        RBX::Vec3 position{};
        RBX::Vec3 smoothPosition{};
        RBX::Vec3 velocity{};
        int health = 0;
        int maxHealth = 0;
        float distance = 0.f;

        std::vector<uintptr_t> bodyParts;
        std::vector<PartCache> parts;
        EspAnchors anchors{};
        ScreenFilter::ScreenBoxFilter screenBox;
        bool isValid = false;
        bool hasSmoothPos = false;
        int mm2Role = 0;
        float mm2RefreshTimer = 0.f;
        bool needsNameRefresh = true;
    };

    inline std::vector<CachedPlayer> players;
    inline RBX::Vec3 localPlayerPos;

    inline float SmoothT(float speed, float dt) {
        if (speed <= 0.f) return 1.f;
        return 1.f - expf(-speed * dt);
    }

    inline RBX::Vec3 LerpVec3(const RBX::Vec3& a, const RBX::Vec3& b, float t) {
        return {
            a.X + (b.X - a.X) * t,
            a.Y + (b.Y - a.Y) * t,
            a.Z + (b.Z - a.Z) * t
        };
    }

    inline void CollectBodyPartsFromEntries(const std::vector<RBX::ChildEntry>& entries, std::vector<uintptr_t>& out) {
        out.clear();
        static const char* r6Parts[] = {
            "Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg", "HumanoidRootPart"
        };
        static const char* r15Parts[] = {
            "Head", "UpperTorso", "LowerTorso",
            "LeftUpperArm", "LeftLowerArm", "LeftHand",
            "RightUpperArm", "RightLowerArm", "RightHand",
            "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
            "RightUpperLeg", "RightLowerLeg", "RightFoot",
            "HumanoidRootPart"
        };

        bool r6 = RBX::FindChildAddr(entries, "Torso") != 0;
        const char** list = r6 ? r6Parts : r15Parts;
        int count = r6 ? 7 : 16;

        for (int i = 0; i < count; i++) {
            uintptr_t addr = RBX::FindChildAddr(entries, list[i]);
            if (addr != 0) out.push_back(addr);
        }
    }

    inline void CollectBodyParts(RBX::RbxInstance& character, std::vector<uintptr_t>& out) {
        CollectBodyPartsFromEntries(character.GetChildEntries(), out);
    }

    inline PartCache* FindPartByName(CachedPlayer& plr, const char* name) {
        for (auto& p : plr.parts)
            if (p.name == name) return &p;
        return nullptr;
    }

    inline void UpdateAnchors(CachedPlayer& plr, float dt) {
        PartCache* head = FindPartByName(plr, "Head");
        PartCache* lf = FindPartByName(plr, "LeftFoot");
        PartCache* rf = FindPartByName(plr, "RightFoot");
        if (!lf) lf = FindPartByName(plr, "Left Leg");
        if (!rf) rf = FindPartByName(plr, "Right Leg");

        float headHalf = 0.55f;
        if (head && head->size.Y > 0.05f) headHalf = head->size.Y * 0.5f;

        float rawHeadTop = head ? (head->smoothPos.Y + headHalf) : (plr.smoothPosition.Y + 1.6f);
        float rawFootY = plr.smoothPosition.Y - 2.8f;
        if (lf) rawFootY = (std::min)(rawFootY, lf->smoothPos.Y - 0.35f);
        if (rf) rawFootY = (std::min)(rawFootY, rf->smoothPos.Y - 0.35f);

        float rawCx = plr.smoothPosition.X;
        float rawCz = plr.smoothPosition.Z;
        if (head) {
            rawCx = rawCx * 0.55f + head->smoothPos.X * 0.45f;
            rawCz = rawCz * 0.55f + head->smoothPos.Z * 0.45f;
        }

        float rawHeight = rawHeadTop - rawFootY;
        rawHeight = (std::max)(rawHeight, 3.5f);
        rawHeight = (std::min)(rawHeight, 9.5f);

        const float posT = SmoothT(Vars::ESP::smoothSpeed * 0.7f, dt);
        const float sizeT = SmoothT(Vars::ESP::smoothSpeed * 0.35f, dt);

        auto& a = plr.anchors;
        if (!a.ready) {
            a.headTop = rawHeadTop;
            a.footY = rawFootY;
            a.centerX = rawCx;
            a.centerZ = rawCz;
            a.bodyHeight = rawHeight;
            a.ready = true;
            return;
        }

        a.headTop += (rawHeadTop - a.headTop) * posT;
        a.footY += (rawFootY - a.footY) * posT;
        a.centerX += (rawCx - a.centerX) * posT;
        a.centerZ += (rawCz - a.centerZ) * posT;
        a.bodyHeight += (rawHeight - a.bodyHeight) * sizeT;
    }

    inline void UpdatePartCache(PartCache& part, float dt) {
        if (part.addr == 0) return;

        uintptr_t prim = Coms->ReadMemory<uintptr_t>(part.addr + offsets::Primitive);
        if (prim == 0) return;

        part.cf = Coms->ReadMemory<RBX::CFrame>(prim + offsets::CFrame);
        part.size = Coms->ReadMemory<RBX::Vec3>(prim + offsets::PartSize);
        part.pos = part.cf.GetPosition();

        const float speed = Vars::ESP::smoothSpeed;
        const float t = SmoothT(speed, dt);

        if (!part.hasSmooth) {
            part.smoothPos = part.pos;
            part.smoothCf = part.cf;
            part.hasSmooth = true;
            return;
        }

        if (speed <= 0.f) {
            part.smoothPos = part.pos;
            part.smoothCf = part.cf;
            return;
        }

        part.smoothPos = LerpVec3(part.smoothPos, part.pos, t);
        part.smoothCf = part.cf;
        part.smoothCf.data[9] = part.smoothPos.X;
        part.smoothCf.data[10] = part.smoothPos.Y;
        part.smoothCf.data[11] = part.smoothPos.Z;

        // Rotation stays live; only position is smoothed for stable projection.
    }

    inline bool NeedsFullPartCache() {
        if (Vars::Chams::enabled) return true;
        if (Vars::ESP::enabled && Vars::ESP::skeleton) return true;
        if (Vars::TriggerBot::enabled && Vars::TriggerBot::hitPart == 2) return true;
        return false;
    }

    inline bool IsAnchorPartName(const std::string& name) {
        return name == "Head" || name == "HumanoidRootPart" || name == "Torso"
            || name == "UpperTorso" || name == "LowerTorso"
            || name == "LeftFoot" || name == "RightFoot"
            || name == "Left Leg" || name == "Right Leg";
    }

    inline bool NeedsUpdate() {
        if (Vars::ESP::enabled || Vars::Chams::enabled) return true;
        if (Vars::Aimbot::enabled || Vars::TriggerBot::enabled) return true;
        if (Vars::MM2::enabled && Vars::MM2::roleESP) return true;
        if (Vars::menuOpen && Vars::selectedTab == 3) return true;
        return false;
    }

    inline void UpdatePlayers(float dt) {
        if (Globals::players.Addr == 0) return;

        auto playerList = Globals::players.GetChildList();

        RBX::Vec3 refPos{};
        bool hasRefPos = false;

        auto localChar = Globals::localPlayer.GetModelRef();
        if (localChar.Addr != 0) {
            RBX::RbxInstance localRoot(0);
            for (auto& cached : players) {
                if (cached.playerAddr == Globals::localPlayer.Addr && cached.rootPartAddr != 0) {
                    localRoot = RBX::RbxInstance(cached.rootPartAddr);
                    break;
                }
            }
            if (localRoot.Addr == 0) {
                auto entries = localChar.GetChildEntries();
                uintptr_t rootAddr = RBX::FindChildAddr(entries, "HumanoidRootPart");
                if (rootAddr == 0) rootAddr = RBX::FindChildAddr(entries, "Torso");
                localRoot = RBX::RbxInstance(rootAddr);
            }
            if (localRoot.Addr != 0) {
                refPos = localRoot.GetPos();
                hasRefPos = true;
            }
        }

        if (!hasRefPos && Globals::camera.Addr != 0) {
            refPos = Globals::camera.GetCameraPos();
            hasRefPos = true;
        }

        if (hasRefPos) localPlayerPos = refPos;

        for (auto& cached : players)
            cached.isValid = false;

        for (auto& plr : playerList) {
            if (plr.Addr == Globals::localPlayer.Addr) continue;

            CachedPlayer* existingPlayer = nullptr;
            for (auto& cached : players) {
                if (cached.playerAddr == plr.Addr) {
                    existingPlayer = &cached;
                    break;
                }
            }

            if (existingPlayer == nullptr) {
                CachedPlayer newPlayer;
                newPlayer.playerAddr = plr.Addr;
                newPlayer.needsNameRefresh = true;
                players.push_back(newPlayer);
                existingPlayer = &players.back();
            }

            if (existingPlayer->needsNameRefresh) {
                existingPlayer->name = plr.GetName();
                existingPlayer->needsNameRefresh = false;
            }

            auto character = plr.GetModelRef();
            if (character.Addr == 0) continue;

            const bool characterChanged = existingPlayer->characterAddr != character.Addr;

            if (characterChanged) {
                existingPlayer->parts.clear();
                existingPlayer->hasSmoothPos = false;
                existingPlayer->anchors.ready = false;
                existingPlayer->screenBox.Reset();
                existingPlayer->humanoidAddr = 0;
                existingPlayer->rootPartAddr = 0;
                existingPlayer->bodyParts.clear();

                auto entries = character.GetChildEntries();
                existingPlayer->humanoidAddr = RBX::FindClassAddr(entries, "Humanoid");
                existingPlayer->rootPartAddr = RBX::FindChildAddr(entries, "HumanoidRootPart");
                if (existingPlayer->rootPartAddr == 0)
                    existingPlayer->rootPartAddr = RBX::FindChildAddr(entries, "Torso");

                static const char* r6Names[] = {
                    "Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg", "HumanoidRootPart"
                };
                static const char* r15Names[] = {
                    "Head", "UpperTorso", "LowerTorso",
                    "LeftUpperArm", "LeftLowerArm", "LeftHand",
                    "RightUpperArm", "RightLowerArm", "RightHand",
                    "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
                    "RightUpperLeg", "RightLowerLeg", "RightFoot",
                    "HumanoidRootPart"
                };
                bool r6 = RBX::FindChildAddr(entries, "Torso") != 0;
                const char** names = r6 ? r6Names : r15Names;
                int nameCount = r6 ? 7 : 16;

                for (int i = 0; i < nameCount; ++i) {
                    uintptr_t addr = RBX::FindChildAddr(entries, names[i]);
                    if (addr == 0) continue;

                    PartCache pc{};
                    pc.addr = addr;
                    pc.name = names[i];
                    pc.hasSmooth = false;
                    existingPlayer->parts.push_back(pc);
                    existingPlayer->bodyParts.push_back(addr);
                }
            }

            existingPlayer->characterAddr = character.Addr;

            if (existingPlayer->humanoidAddr == 0 || existingPlayer->rootPartAddr == 0) continue;

            RBX::RbxInstance rootPart(existingPlayer->rootPartAddr);
            RBX::Vec3 rootPos = rootPart.GetPos();
            existingPlayer->position = rootPos;

            uintptr_t prim = Coms->ReadMemory<uintptr_t>(existingPlayer->rootPartAddr + offsets::Primitive);
            if (prim != 0)
                existingPlayer->velocity = Coms->ReadMemory<RBX::Vec3>(prim + offsets::Velocity);
            else
                existingPlayer->velocity = {};

            const float speed = Vars::ESP::smoothSpeed;
            const float t = SmoothT(speed, dt);
            if (!existingPlayer->hasSmoothPos) {
                existingPlayer->smoothPosition = rootPos;
                existingPlayer->hasSmoothPos = true;
            } else if (speed <= 0.f) {
                existingPlayer->smoothPosition = rootPos;
            } else {
                RBX::Vec3 predicted = {
                    rootPos.X + existingPlayer->velocity.X * dt * 0.45f,
                    rootPos.Y + existingPlayer->velocity.Y * dt * 0.45f,
                    rootPos.Z + existingPlayer->velocity.Z * dt * 0.45f
                };
                existingPlayer->smoothPosition = LerpVec3(existingPlayer->smoothPosition, predicted, t);
            }

            existingPlayer->health = static_cast<int>(Coms->ReadMemory<float>(existingPlayer->humanoidAddr + offsets::Health));
            existingPlayer->maxHealth = static_cast<int>(Coms->ReadMemory<float>(existingPlayer->humanoidAddr + offsets::MaxHealth));
            existingPlayer->distance = hasRefPos ? rootPart.CalcDistance(refPos) : 0.f;

            const bool fullParts = NeedsFullPartCache();
            for (size_t i = 0; i < existingPlayer->parts.size(); i++) {
                if (fullParts || IsAnchorPartName(existingPlayer->parts[i].name))
                    UpdatePartCache(existingPlayer->parts[i], dt);
            }

            UpdateAnchors(*existingPlayer, dt);

            if (Vars::MM2::enabled) {
                existingPlayer->mm2RefreshTimer += dt;
                if (characterChanged || existingPlayer->mm2RefreshTimer >= 2.f) {
                    existingPlayer->mm2RefreshTimer = 0.f;
                    existingPlayer->mm2Role = static_cast<int>(MM2::DetectPlayerRole(plr));
                }
            } else {
                existingPlayer->mm2Role = 0;
            }

            existingPlayer->isValid = true;
        }

        players.erase(
            std::remove_if(players.begin(), players.end(),
                [](const CachedPlayer& p) { return !p.isValid; }),
            players.end()
        );
    }

    inline RBX::Vec3 GetSmoothedRoot(CachedPlayer& plr, float /*dt*/) {
        return plr.smoothPosition;
    }

    inline PartCache* FindPart(CachedPlayer& plr, uintptr_t addr) {
        for (auto& p : plr.parts)
            if (p.addr == addr) return &p;
        return nullptr;
    }

    inline RBX::Vec3 GetPartPos(CachedPlayer& plr, RBX::RbxInstance& part) {
        if (auto* cached = FindPart(plr, part.Addr))
            return cached->smoothPos;
        return part.GetPos();
    }
}
