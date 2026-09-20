#pragma once
#include "../../../Game/SDK/SDK.h"
#include "../../Globals/Globals.h"
#include "../../Vars/Vars.h"
#include "../Misc/Notifications.h"
#include <string>
#include <algorithm>
#include <cctype>

namespace MM2 {

    enum class Role : int {
        Innocent = 0,
        Murderer = 1,
        Sheriff = 2
    };

    inline std::string murdererName;
    inline std::string sheriffName;

    inline std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    inline bool NameIsKnife(const std::string& name) {
        std::string n = ToLower(name);
        if (n == "knife") return true;
        if (n.size() >= 5 && n.find("knife") != std::string::npos) return true;
        return false;
    }

    inline bool NameIsGun(const std::string& name) {
        std::string n = ToLower(name);
        return n == "gun" || n == "revolver";
    }

    inline Role ScanContainer(RBX::RbxInstance& container, Role found) {
        if (container.Addr == 0) return found;

        for (auto& child : container.GetChildList()) {
            if (child.GetClass() != "Tool") continue;

            std::string name = child.GetName();
            if (NameIsKnife(name)) return Role::Murderer;
            if (NameIsGun(name)) {
                if (found != Role::Murderer) found = Role::Sheriff;
            }
        }
        return found;
    }

    inline Role DetectTeamRole(RBX::RbxInstance& player) {
        uintptr_t teamAddr = Coms->ReadMemory<uintptr_t>(player.Addr + offsets::Team);
        if (teamAddr == 0) return Role::Innocent;

        std::string team = ToLower(RBX::RbxInstance(teamAddr).GetName());
        if (team.find("murder") != std::string::npos) return Role::Murderer;
        if (team.find("sheriff") != std::string::npos) return Role::Sheriff;
        return Role::Innocent;
    }

    inline Role DetectPlayerRole(RBX::RbxInstance& player) {
        Role role = Role::Innocent;

        auto backpack = player.FindChild("Backpack");
        role = ScanContainer(backpack, role);
        if (role == Role::Murderer) return role;

        auto character = player.GetModelRef();
        role = ScanContainer(character, role);
        if (role == Role::Murderer) return role;

        Role teamRole = DetectTeamRole(player);
        if (teamRole != Role::Innocent) return teamRole;

        return role;
    }

    inline void UpdateRoleNames() {
        static std::string lastMurderer;
        static std::string lastSheriff;

        murdererName.clear();
        sheriffName.clear();

        if (!Vars::MM2::enabled) {
            lastMurderer.clear();
            lastSheriff.clear();
            return;
        }

        auto checkPlayer = [&](RBX::RbxInstance& player) {
            Role role = DetectPlayerRole(player);
            std::string name = player.GetName();
            if (role == Role::Murderer && murdererName.empty())
                murdererName = name;
            if (role == Role::Sheriff && sheriffName.empty())
                sheriffName = name;
        };

        checkPlayer(Globals::localPlayer);

        for (auto& plr : Globals::players.GetChildList()) {
            if (plr.Addr == Globals::localPlayer.Addr) continue;
            checkPlayer(plr);
        }

        if (!murdererName.empty() && murdererName != lastMurderer) {
            Notify::Push("MM2", "Murderer: " + murdererName, Notify::Type::Error, 5.f);
            lastMurderer = murdererName;
        }
        if (Vars::MM2::showSheriff && !sheriffName.empty() && sheriffName != lastSheriff) {
            Notify::Push("MM2", "Sheriff: " + sheriffName, Notify::Type::Info, 5.f);
            lastSheriff = sheriffName;
        }
    }
}
