#pragma once
#include "../Vars/Vars.h"
#include "../Features/Misc/Notifications.h"
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#pragma comment(lib, "shell32.lib")

namespace ConfigSystem {

    inline std::string statusMessage;
    inline bool statusOk = true;

    inline std::string SanitizeName(std::string name) {
        std::string out;
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')
                out += c;
        }
        if (out.empty()) out = "default";
        return out;
    }

    inline std::filesystem::path GetConfigDir() {
        char appData[MAX_PATH]{};
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
            auto dir = std::filesystem::path(appData) / "CheatGlobal" / "configs";
            std::filesystem::create_directories(dir);
            return dir;
        }
        auto dir = std::filesystem::current_path() / "configs";
        std::filesystem::create_directories(dir);
        return dir;
    }

    inline std::filesystem::path GetConfigPath(const std::string& name) {
        return GetConfigDir() / (SanitizeName(name) + ".cfg");
    }

    inline void WriteBool(std::ofstream& out, const char* key, bool v) {
        out << key << '=' << (v ? 1 : 0) << '\n';
    }

    inline void WriteInt(std::ofstream& out, const char* key, int v) {
        out << key << '=' << v << '\n';
    }

    inline void WriteFloat(std::ofstream& out, const char* key, float v) {
        out << key << '=' << v << '\n';
    }

    inline bool Save(const std::string& name) {
        auto path = GetConfigPath(name);
        std::ofstream out(path, std::ios::trunc);
        if (!out.is_open()) {
            statusOk = false;
            statusMessage = "Failed to save config.";
            return false;
        }

        out << "# CheatGlobal Config v1\n";

        WriteBool(out, "aimbot.enabled", Vars::Aimbot::enabled);
        WriteBool(out, "aimbot.showFOV", Vars::Aimbot::showFOV);
        WriteFloat(out, "aimbot.fovRadius", Vars::Aimbot::fovRadius);
        WriteFloat(out, "aimbot.smoothing", Vars::Aimbot::smoothing);
        WriteInt(out, "aimbot.aimTarget", Vars::Aimbot::aimTarget);
        WriteInt(out, "aimbot.aimMethod", Vars::Aimbot::aimMethod);
        WriteInt(out, "aimbot.aimbotKey", Vars::Aimbot::aimbotKey);

        WriteBool(out, "triggerbot.enabled", Vars::TriggerBot::enabled);
        WriteInt(out, "triggerbot.triggerKey", Vars::TriggerBot::triggerKey);
        WriteInt(out, "triggerbot.hitPart", Vars::TriggerBot::hitPart);
        WriteFloat(out, "triggerbot.triggerRadius", Vars::TriggerBot::triggerRadius);
        WriteFloat(out, "triggerbot.hitboxPad", Vars::TriggerBot::hitboxPad);
        WriteFloat(out, "triggerbot.maxDistance", Vars::TriggerBot::maxDistance);
        WriteFloat(out, "triggerbot.delayMs", Vars::TriggerBot::delayMs);
        WriteFloat(out, "triggerbot.cooldownMs", Vars::TriggerBot::cooldownMs);
        WriteBool(out, "triggerbot.teamCheck", Vars::TriggerBot::teamCheck);
        WriteBool(out, "triggerbot.ignoreDead", Vars::TriggerBot::ignoreDead);
        WriteBool(out, "triggerbot.skipWhileFiring", Vars::TriggerBot::skipWhileFiring);

        WriteBool(out, "esp.enabled", Vars::ESP::enabled);
        WriteBool(out, "esp.boxes", Vars::ESP::boxes);
        WriteBool(out, "esp.names", Vars::ESP::names);
        WriteBool(out, "esp.distance", Vars::ESP::distance);
        WriteBool(out, "esp.healthBar", Vars::ESP::healthBar);
        WriteBool(out, "esp.skeleton", Vars::ESP::skeleton);
        WriteBool(out, "esp.tracers", Vars::ESP::tracers);
        WriteInt(out, "esp.tracerOrigin", Vars::ESP::tracerOrigin);
        WriteFloat(out, "esp.smoothSpeed", Vars::ESP::smoothSpeed);
        WriteFloat(out, "esp.boxPadding", Vars::ESP::boxPadding);
        WriteFloat(out, "esp.boxWidthRatio", Vars::ESP::boxWidthRatio);
        WriteFloat(out, "esp.screenStabilize", Vars::ESP::screenStabilize);
        WriteFloat(out, "esp.screenBeta", Vars::ESP::screenBeta);
        WriteBool(out, "esp.stableBox", Vars::ESP::stableBox);

        WriteBool(out, "chams.enabled", Vars::Chams::enabled);
        WriteBool(out, "chams.fill", Vars::Chams::fill);
        WriteBool(out, "chams.outline", Vars::Chams::outline);
        WriteBool(out, "chams.glow", Vars::Chams::glow);
        WriteBool(out, "chams.showRoot", Vars::Chams::showRoot);
        WriteBool(out, "chams.teamColor", Vars::Chams::teamColor);
        WriteFloat(out, "chams.opacity", Vars::Chams::opacity);
        WriteFloat(out, "chams.colorR", Vars::Chams::colorR);
        WriteFloat(out, "chams.colorG", Vars::Chams::colorG);
        WriteFloat(out, "chams.colorB", Vars::Chams::colorB);

        WriteBool(out, "local.speedEnabled", Vars::Local::speedEnabled);
        WriteFloat(out, "local.walkSpeed", Vars::Local::walkSpeed);
        WriteBool(out, "local.jumpEnabled", Vars::Local::jumpEnabled);
        WriteFloat(out, "local.jumpPower", Vars::Local::jumpPower);
        WriteBool(out, "local.flyEnabled", Vars::Local::flyEnabled);
        WriteFloat(out, "local.flySpeed", Vars::Local::flySpeed);
        WriteBool(out, "local.noclipEnabled", Vars::Local::noclipEnabled);

        WriteBool(out, "teleport.clickTpEnabled", Vars::Teleport::clickTpEnabled);
        WriteInt(out, "teleport.clickTpKey", Vars::Teleport::clickTpKey);
        WriteInt(out, "teleport.clickTpMode", Vars::Teleport::clickTpMode);
        WriteFloat(out, "teleport.clickTpDistance", Vars::Teleport::clickTpDistance);
        WriteFloat(out, "teleport.clickTpOffsetY", Vars::Teleport::clickTpOffsetY);
        WriteFloat(out, "teleport.clickPartRadius", Vars::Teleport::clickPartRadius);
        WriteFloat(out, "teleport.clickTpMaxScan", Vars::Teleport::clickTpMaxScan);
        WriteFloat(out, "teleport.stageEndOffsetY", Vars::Teleport::stageEndOffsetY);
        WriteBool(out, "teleport.clickTpNotify", Vars::Teleport::clickTpNotify);

        WriteFloat(out, "players.teleportOffsetY", Vars::Players::teleportOffsetY);
        WriteFloat(out, "players.bringSideOffset", Vars::Players::bringSideOffset);
        WriteFloat(out, "players.bringOffsetY", Vars::Players::bringOffsetY);

        WriteBool(out, "mm2.enabled", Vars::MM2::enabled);
        WriteBool(out, "mm2.roleESP", Vars::MM2::roleESP);
        WriteBool(out, "mm2.showSheriff", Vars::MM2::showSheriff);
        WriteBool(out, "mm2.murdererHUD", Vars::MM2::murdererHUD);

        WriteBool(out, "bladeball.enabled", Vars::BladeBall::enabled);
        WriteBool(out, "bladeball.requireHighlight", Vars::BladeBall::requireHighlight);
        WriteBool(out, "bladeball.useClick", Vars::BladeBall::useClick);
        WriteBool(out, "bladeball.proMode", Vars::BladeBall::proMode);
        WriteBool(out, "bladeball.burstParry", Vars::BladeBall::burstParry);
        WriteBool(out, "bladeball.dualInput", Vars::BladeBall::dualInput);
        WriteFloat(out, "bladeball.parryDistance", Vars::BladeBall::parryDistance);
        WriteFloat(out, "bladeball.parryTiming", Vars::BladeBall::parryTiming);
        WriteFloat(out, "bladeball.distanceOffset", Vars::BladeBall::distanceOffset);
        WriteFloat(out, "bladeball.timingScale", Vars::BladeBall::timingScale);
        WriteFloat(out, "bladeball.reactionMs", Vars::BladeBall::reactionMs);
        WriteFloat(out, "bladeball.minBallSpeed", Vars::BladeBall::minBallSpeed);
        WriteFloat(out, "bladeball.maxTrackDistance", Vars::BladeBall::maxTrackDistance);
        WriteFloat(out, "bladeball.directionDot", Vars::BladeBall::directionDot);
        WriteFloat(out, "bladeball.cooldownMs", Vars::BladeBall::cooldownMs);
        WriteFloat(out, "bladeball.clashDistance", Vars::BladeBall::clashDistance);
        WriteFloat(out, "bladeball.emergencyDistance", Vars::BladeBall::emergencyDistance);
        WriteFloat(out, "bladeball.pingCompensation", Vars::BladeBall::pingCompensation);
        WriteInt(out, "bladeball.parryKey", Vars::BladeBall::parryKey);

        WriteBool(out, "misc.antiAfkEnabled", Vars::Misc::antiAfkEnabled);
        WriteBool(out, "misc.antiAfkNotify", Vars::Misc::antiAfkNotify);
        WriteBool(out, "misc.notificationsEnabled", Vars::Misc::notificationsEnabled);
        WriteFloat(out, "misc.antiAfkInterval", Vars::Misc::antiAfkInterval);

        WriteBool(out, "autoclicker.enabled", Vars::AutoClicker::enabled);
        WriteInt(out, "autoclicker.holdKey", Vars::AutoClicker::holdKey);
        WriteInt(out, "autoclicker.clickButton", Vars::AutoClicker::clickButton);
        WriteBool(out, "autoclicker.onlyFocused", Vars::AutoClicker::onlyFocused);
        WriteInt(out, "autoclicker.maxCps", Vars::AutoClicker::maxCps);

        statusOk = true;
        statusMessage = "Saved: " + SanitizeName(name);
        Notify::Push("Config", "Saved " + SanitizeName(name), Notify::Type::Success, 3.f);
        return true;
    }

    inline bool ApplyValue(const std::string& key, const std::string& val) {
        try {
            if (key == "aimbot.enabled") { Vars::Aimbot::enabled = std::stoi(val) != 0; return true; }
            if (key == "aimbot.showFOV") { Vars::Aimbot::showFOV = std::stoi(val) != 0; return true; }
            if (key == "aimbot.fovRadius") { Vars::Aimbot::fovRadius = std::stof(val); return true; }
            if (key == "aimbot.smoothing") { Vars::Aimbot::smoothing = std::stof(val); return true; }
            if (key == "aimbot.aimTarget") { Vars::Aimbot::aimTarget = std::stoi(val); return true; }
            if (key == "aimbot.aimMethod") { Vars::Aimbot::aimMethod = std::stoi(val); return true; }
            if (key == "aimbot.aimbotKey") { Vars::Aimbot::aimbotKey = std::stoi(val); return true; }

            if (key == "triggerbot.enabled") { Vars::TriggerBot::enabled = std::stoi(val) != 0; return true; }
            if (key == "triggerbot.triggerKey") { Vars::TriggerBot::triggerKey = std::stoi(val); return true; }
            if (key == "triggerbot.hitPart") { Vars::TriggerBot::hitPart = std::stoi(val); return true; }
            if (key == "triggerbot.triggerRadius") { Vars::TriggerBot::triggerRadius = std::stof(val); return true; }
            if (key == "triggerbot.hitboxPad") { Vars::TriggerBot::hitboxPad = std::stof(val); return true; }
            if (key == "triggerbot.maxDistance") { Vars::TriggerBot::maxDistance = std::stof(val); return true; }
            if (key == "triggerbot.delayMs") { Vars::TriggerBot::delayMs = std::stof(val); return true; }
            if (key == "triggerbot.cooldownMs") { Vars::TriggerBot::cooldownMs = std::stof(val); return true; }
            if (key == "triggerbot.teamCheck") { Vars::TriggerBot::teamCheck = std::stoi(val) != 0; return true; }
            if (key == "triggerbot.ignoreDead") { Vars::TriggerBot::ignoreDead = std::stoi(val) != 0; return true; }
            if (key == "triggerbot.skipWhileFiring") { Vars::TriggerBot::skipWhileFiring = std::stoi(val) != 0; return true; }

            if (key == "esp.enabled") { Vars::ESP::enabled = std::stoi(val) != 0; return true; }
            if (key == "esp.boxes") { Vars::ESP::boxes = std::stoi(val) != 0; return true; }
            if (key == "esp.names") { Vars::ESP::names = std::stoi(val) != 0; return true; }
            if (key == "esp.distance") { Vars::ESP::distance = std::stoi(val) != 0; return true; }
            if (key == "esp.healthBar") { Vars::ESP::healthBar = std::stoi(val) != 0; return true; }
            if (key == "esp.skeleton") { Vars::ESP::skeleton = std::stoi(val) != 0; return true; }
            if (key == "esp.tracers") { Vars::ESP::tracers = std::stoi(val) != 0; return true; }
            if (key == "esp.tracerOrigin") { Vars::ESP::tracerOrigin = std::stoi(val); return true; }
            if (key == "esp.smoothSpeed") { Vars::ESP::smoothSpeed = std::stof(val); return true; }
            if (key == "esp.boxPadding") { Vars::ESP::boxPadding = std::stof(val); return true; }
            if (key == "esp.boxWidthRatio") { Vars::ESP::boxWidthRatio = std::stof(val); return true; }
            if (key == "esp.screenStabilize") { Vars::ESP::screenStabilize = std::stof(val); return true; }
            if (key == "esp.screenBeta") { Vars::ESP::screenBeta = std::stof(val); return true; }
            if (key == "esp.stableBox") { Vars::ESP::stableBox = std::stoi(val) != 0; return true; }

            if (key == "chams.enabled") { Vars::Chams::enabled = std::stoi(val) != 0; return true; }
            if (key == "chams.fill") { Vars::Chams::fill = std::stoi(val) != 0; return true; }
            if (key == "chams.outline") { Vars::Chams::outline = std::stoi(val) != 0; return true; }
            if (key == "chams.glow") { Vars::Chams::glow = std::stoi(val) != 0; return true; }
            if (key == "chams.showRoot") { Vars::Chams::showRoot = std::stoi(val) != 0; return true; }
            if (key == "chams.teamColor") { Vars::Chams::teamColor = std::stoi(val) != 0; return true; }
            if (key == "chams.opacity") { Vars::Chams::opacity = std::stof(val); return true; }
            if (key == "chams.colorR") { Vars::Chams::colorR = std::stof(val); return true; }
            if (key == "chams.colorG") { Vars::Chams::colorG = std::stof(val); return true; }
            if (key == "chams.colorB") { Vars::Chams::colorB = std::stof(val); return true; }

            if (key == "local.speedEnabled") { Vars::Local::speedEnabled = std::stoi(val) != 0; return true; }
            if (key == "local.walkSpeed") { Vars::Local::walkSpeed = std::stof(val); return true; }
            if (key == "local.jumpEnabled") { Vars::Local::jumpEnabled = std::stoi(val) != 0; return true; }
            if (key == "local.jumpPower") { Vars::Local::jumpPower = std::stof(val); return true; }
            if (key == "local.flyEnabled") { Vars::Local::flyEnabled = std::stoi(val) != 0; return true; }
            if (key == "local.flySpeed") { Vars::Local::flySpeed = std::stof(val); return true; }
            if (key == "local.noclipEnabled") { Vars::Local::noclipEnabled = std::stoi(val) != 0; return true; }

            if (key == "teleport.clickTpEnabled") { Vars::Teleport::clickTpEnabled = std::stoi(val) != 0; return true; }
            if (key == "teleport.clickTpKey") { Vars::Teleport::clickTpKey = std::stoi(val); return true; }
            if (key == "teleport.clickTpMode") { Vars::Teleport::clickTpMode = std::stoi(val); return true; }
            if (key == "teleport.clickTpDistance") { Vars::Teleport::clickTpDistance = std::stof(val); return true; }
            if (key == "teleport.clickTpOffsetY") { Vars::Teleport::clickTpOffsetY = std::stof(val); return true; }
            if (key == "teleport.clickPartRadius") { Vars::Teleport::clickPartRadius = std::stof(val); return true; }
            if (key == "teleport.clickTpMaxScan") { Vars::Teleport::clickTpMaxScan = std::stof(val); return true; }
            if (key == "teleport.stageEndOffsetY") { Vars::Teleport::stageEndOffsetY = std::stof(val); return true; }
            if (key == "teleport.clickTpNotify") { Vars::Teleport::clickTpNotify = std::stoi(val) != 0; return true; }

            if (key == "players.teleportOffsetY") { Vars::Players::teleportOffsetY = std::stof(val); return true; }
            if (key == "players.bringSideOffset") { Vars::Players::bringSideOffset = std::stof(val); return true; }
            if (key == "players.bringOffsetY") { Vars::Players::bringOffsetY = std::stof(val); return true; }

            if (key == "mm2.enabled") { Vars::MM2::enabled = std::stoi(val) != 0; return true; }
            if (key == "mm2.roleESP") { Vars::MM2::roleESP = std::stoi(val) != 0; return true; }
            if (key == "mm2.showSheriff") { Vars::MM2::showSheriff = std::stoi(val) != 0; return true; }
            if (key == "mm2.murdererHUD") { Vars::MM2::murdererHUD = std::stoi(val) != 0; return true; }

            if (key == "bladeball.enabled") { Vars::BladeBall::enabled = std::stoi(val) != 0; return true; }
            if (key == "bladeball.requireHighlight") { Vars::BladeBall::requireHighlight = std::stoi(val) != 0; return true; }
            if (key == "bladeball.useClick") { Vars::BladeBall::useClick = std::stoi(val) != 0; return true; }
            if (key == "bladeball.proMode") { Vars::BladeBall::proMode = std::stoi(val) != 0; return true; }
            if (key == "bladeball.burstParry") { Vars::BladeBall::burstParry = std::stoi(val) != 0; return true; }
            if (key == "bladeball.dualInput") { Vars::BladeBall::dualInput = std::stoi(val) != 0; return true; }
            if (key == "bladeball.parryDistance") { Vars::BladeBall::parryDistance = std::stof(val); return true; }
            if (key == "bladeball.parryTiming") { Vars::BladeBall::parryTiming = std::stof(val); return true; }
            if (key == "bladeball.distanceOffset") { Vars::BladeBall::distanceOffset = std::stof(val); return true; }
            if (key == "bladeball.timingScale") { Vars::BladeBall::timingScale = std::stof(val); return true; }
            if (key == "bladeball.reactionMs") { Vars::BladeBall::reactionMs = std::stof(val); return true; }
            if (key == "bladeball.minBallSpeed") { Vars::BladeBall::minBallSpeed = std::stof(val); return true; }
            if (key == "bladeball.maxTrackDistance") { Vars::BladeBall::maxTrackDistance = std::stof(val); return true; }
            if (key == "bladeball.directionDot") { Vars::BladeBall::directionDot = std::stof(val); return true; }
            if (key == "bladeball.cooldownMs") { Vars::BladeBall::cooldownMs = std::stof(val); return true; }
            if (key == "bladeball.clashDistance") { Vars::BladeBall::clashDistance = std::stof(val); return true; }
            if (key == "bladeball.emergencyDistance") { Vars::BladeBall::emergencyDistance = std::stof(val); return true; }
            if (key == "bladeball.pingCompensation") { Vars::BladeBall::pingCompensation = std::stof(val); return true; }
            if (key == "bladeball.parryKey") { Vars::BladeBall::parryKey = std::stoi(val); return true; }

            if (key == "misc.antiAfkEnabled") { Vars::Misc::antiAfkEnabled = std::stoi(val) != 0; return true; }
            if (key == "misc.antiAfkNotify") { Vars::Misc::antiAfkNotify = std::stoi(val) != 0; return true; }
            if (key == "misc.notificationsEnabled") { Vars::Misc::notificationsEnabled = std::stoi(val) != 0; return true; }
            if (key == "misc.antiAfkInterval") { Vars::Misc::antiAfkInterval = std::stof(val); return true; }

            if (key == "autoclicker.enabled") { Vars::AutoClicker::enabled = std::stoi(val) != 0; return true; }
            if (key == "autoclicker.holdKey") { Vars::AutoClicker::holdKey = std::stoi(val); return true; }
            if (key == "autoclicker.clickButton") { Vars::AutoClicker::clickButton = std::stoi(val); return true; }
            if (key == "autoclicker.onlyFocused") { Vars::AutoClicker::onlyFocused = std::stoi(val) != 0; return true; }
            if (key == "autoclicker.maxCps") { Vars::AutoClicker::maxCps = std::stoi(val); return true; }
        } catch (...) {}
        return false;
    }

    inline bool Load(const std::string& name) {
        auto path = GetConfigPath(name);
        if (!std::filesystem::exists(path)) {
            statusOk = false;
            statusMessage = "Config not found.";
            return false;
        }

        std::ifstream in(path);
        if (!in.is_open()) {
            statusOk = false;
            statusMessage = "Failed to open config.";
            return false;
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            ApplyValue(key, val);
        }

        statusOk = true;
        statusMessage = "Loaded: " + SanitizeName(name);
        Notify::Push("Config", "Loaded " + SanitizeName(name), Notify::Type::Success, 3.f);
        return true;
    }

    inline std::vector<std::string> List() {
        std::vector<std::string> names;
        for (auto& entry : std::filesystem::directory_iterator(GetConfigDir())) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".cfg") continue;
            names.push_back(entry.path().stem().string());
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    inline bool Delete(const std::string& name) {
        auto path = GetConfigPath(name);
        if (!std::filesystem::exists(path)) {
            statusOk = false;
            statusMessage = "Config not found.";
            return false;
        }
        std::error_code ec;
        std::filesystem::remove(path, ec);
        if (ec) {
            statusOk = false;
            statusMessage = "Failed to delete config.";
            return false;
        }
        statusOk = true;
        statusMessage = "Deleted: " + SanitizeName(name);
        return true;
    }

    inline void LoadDefaultOnStartup() {
        auto path = GetConfigPath("default");
        if (std::filesystem::exists(path))
            Load("default");
    }
}
