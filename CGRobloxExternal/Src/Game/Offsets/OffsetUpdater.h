#pragma once
#include "Offsets.h"
#include "../../Memory/Communication.h"
#include <windows.h>
#include <winhttp.h>
#include <ShlObj.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <chrono>
#include <cctype>
                                
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")

namespace OffsetUpdater {

    struct MappingEntry {
        const char* key;
        const char* apiPath;
        uintptr_t* value;
    };

    inline MappingEntry* GetMappings(size_t& outCount) {
        static MappingEntry mappings[] = {
            {"Adornee", "Misc.Adornee", &offsets::Adornee},
            {"Anchored", "PrimitiveFlags.Anchored", &offsets::Anchored},
            {"AnchoredMask", "PrimitiveFlags.Anchored", &offsets::AnchoredMask},
            {"AnimationId", "Misc.AnimationId", &offsets::AnimationId},
            {"AttributeToNext", "Attribute.Size", &offsets::AttributeToNext},
            {"AttributeToValue", "Attribute.Value", &offsets::AttributeToValue},
            {"AutoJumpEnabled", "Humanoid.AutoJumpEnabled", &offsets::AutoJumpEnabled},
            {"BeamBrightness", "Beam.Brightness", &offsets::BeamBrightness},
            {"BeamLightEmission", "Beam.LightEmission", &offsets::BeamLightEmission},
            {"BeamLightInfuence", "Beam.LightInfluence", &offsets::BeamLightInfuence},
            {"CFrame", "Primitive.Rotation", &offsets::CFrame},
            {"Camera", "Workspace.CurrentCamera", &offsets::Camera},
            {"CameraMaxZoomDistance", "Player.MaxZoomDistance", &offsets::CameraMaxZoomDistance},
            {"CameraMinZoomDistance", "Player.MinZoomDistance", &offsets::CameraMinZoomDistance},
            {"CameraMode", "Player.CameraMode", &offsets::CameraMode},
            {"CameraPos", "Camera.Position", &offsets::CameraPos},
            {"CameraRotation", "Camera.Rotation", &offsets::CameraRotation},
            {"CameraSubject", "Camera.CameraSubject", &offsets::CameraSubject},
            {"CameraType", "Camera.CameraType", &offsets::CameraType},
            {"CanCollide", "PrimitiveFlags.CanCollide", &offsets::CanCollide},
            {"CanCollideMask", "PrimitiveFlags.CanCollide", &offsets::CanCollideMask},
            {"CanTouch", "PrimitiveFlags.CanTouch", &offsets::CanTouch},
            {"CanTouchMask", "PrimitiveFlags.CanTouch", &offsets::CanTouchMask},
            {"Children", "Instance.ChildrenStart", &offsets::Children},
            {"ChildrenEnd", "Instance.ChildrenEnd", &offsets::ChildrenEnd},
            {"ClassDescriptor", "Instance.ClassDescriptor", &offsets::ClassDescriptor},
            {"ClassDescriptorToClassName", "Instance.ClassName", &offsets::ClassDescriptorToClassName},
            {"ClickDetectorMaxActivationDistance", "ClickDetector.MaxActivationDistance", &offsets::ClickDetectorMaxActivationDistance},
            {"ClockTime", "Lighting.ClockTime", &offsets::ClockTime},
            {"CreatorId", "DataModel.CreatorId", &offsets::CreatorId},
            {"DataModelPrimitiveCount", "DataModel.PrimitiveCount", &offsets::DataModelPrimitiveCount},
            {"DataModelToRenderView1", "DataModel.ToRenderView1", &offsets::DataModelToRenderView1},
            {"DataModelToRenderView2", "DataModel.ToRenderView2", &offsets::DataModelToRenderView2},
            {"DataModelToRenderView3", "DataModel.ToRenderView3", &offsets::DataModelToRenderView3},
            {"DecalTexture", "Textures.Decal_Texture", &offsets::DecalTexture},
            {"Dimensions", "VisualEngine.Dimensions", &offsets::Dimensions},
            {"DisplayName", "Player.DisplayName", &offsets::DisplayName},
            {"EvaluateStateMachine", "Humanoid.EvaluateStateMachine", &offsets::EvaluateStateMachine},
            {"FOV", "Camera.FieldOfView", &offsets::FOV},
            {"FakeDataModelPointer", "FakeDataModel.Pointer", &offsets::FakeDataModelPointer},
            {"FakeDataModelToDataModel", "FakeDataModel.RealDataModel", &offsets::FakeDataModelToDataModel},
            {"FogColor", "Lighting.FogColor", &offsets::FogColor},
            {"FogEnd", "Lighting.FogEnd", &offsets::FogEnd},
            {"FogStart", "Lighting.FogStart", &offsets::FogStart},
            {"FrameVisible", "GuiObject.Visible", &offsets::FrameVisible},
            {"GameId", "DataModel.GameId", &offsets::GameId},
            {"GameLoaded", "DataModel.GameLoaded", &offsets::GameLoaded},
            {"Gravity", "World.Gravity", &offsets::Gravity},
            {"Health", "Humanoid.Health", &offsets::Health},
            {"HealthDisplayDistance", "Humanoid.HealthDisplayDistance", &offsets::HealthDisplayDistance},
            {"HipHeight", "Humanoid.HipHeight", &offsets::HipHeight},
            {"HumanoidDisplayName", "Humanoid.DisplayName", &offsets::HumanoidDisplayName},
            {"HumanoidState", "Humanoid.HumanoidState", &offsets::HumanoidState},
            {"HumanoidStateId", "Humanoid.HumanoidStateID", &offsets::HumanoidStateId},
            {"InputObject", "MouseService.InputObject", &offsets::InputObject},
            {"JobEnd", "TaskScheduler.JobEnd", &offsets::JobEnd},
            {"JobId", "DataModel.JobId", &offsets::JobId},
            {"JobStart", "TaskScheduler.JobStart", &offsets::JobStart},
            {"Job_Name", "TaskScheduler.JobName", &offsets::Job_Name},
            {"JobsPointer", "TaskScheduler.Pointer", &offsets::JobsPointer},
            {"JumpPower", "Humanoid.JumpPower", &offsets::JumpPower},
            {"JumpHeight", "Humanoid.JumpHeight", &offsets::JumpHeight},
            {"LocalPlayer", "Player.LocalPlayer", &offsets::LocalPlayer},
            {"LocalScriptByteCode", "LocalScript.ByteCode", &offsets::LocalScriptByteCode},
            {"LocalScriptBytecodePointer", "ByteCode.Pointer", &offsets::LocalScriptBytecodePointer},
            {"LocalScriptHash", "LocalScript.Hash", &offsets::LocalScriptHash},
            {"MaxHealth", "Humanoid.MaxHealth", &offsets::MaxHealth},
            {"MaxSlopeAngle", "Humanoid.MaxSlopeAngle", &offsets::MaxSlopeAngle},
            {"MeshPartColor3", "BasePart.Color3", &offsets::MeshPartColor3},
            {"MeshPartTexture", "MeshPart.Texture", &offsets::MeshPartTexture},
            {"ModelInstance", "Player.ModelInstance", &offsets::ModelInstance},
            {"ModuleScriptByteCode", "ModuleScript.ByteCode", &offsets::ModuleScriptByteCode},
            {"ModuleScriptBytecodePointer", "ByteCode.Pointer", &offsets::ModuleScriptBytecodePointer},
            {"ModuleScriptHash", "ModuleScript.Hash", &offsets::ModuleScriptHash},
            {"MoonTextureId", "Sky.MoonTextureId", &offsets::MoonTextureId},
            {"MousePosition", "MouseService.MousePosition", &offsets::MousePosition},
            {"MoveDirection", "Humanoid.MoveDirection", &offsets::MoveDirection},
            {"Name", "Instance.Name", &offsets::Name},
            {"NameDisplayDistance", "Humanoid.NameDisplayDistance", &offsets::NameDisplayDistance},
            {"NameSize", "Misc.StringLength", &offsets::NameSize},
            {"OutdoorAmbient", "Lighting.OutdoorAmbient", &offsets::OutdoorAmbient},
            {"Parent", "Instance.Parent", &offsets::Parent},
            {"PartSize", "Primitive.Size", &offsets::PartSize},
            {"PlaceId", "DataModel.PlaceId", &offsets::PlaceId},
            {"PlayerMouse", "Player.Mouse", &offsets::PlayerMouse},
            {"Position", "Primitive.Position", &offsets::Position},
            {"Primitive", "BasePart.Primitive", &offsets::Primitive},
            {"PrimitiveFlags", "Primitive.Flags", &offsets::PrimitiveFlags},
            {"PrimitiveValidateValue", "Primitive.Validate", &offsets::PrimitiveValidateValue},
            {"ProximityPromptActionText", "ProximityPrompt.ActionText", &offsets::ProximityPromptActionText},
            {"ProximityPromptEnabled", "ProximityPrompt.Enabled", &offsets::ProximityPromptEnabled},
            {"ProximityPromptGamepadKeyCode", "ProximityPrompt.GamepadKeyCode", &offsets::ProximityPromptGamepadKeyCode},
            {"ProximityPromptHoldDuraction", "ProximityPrompt.HoldDuration", &offsets::ProximityPromptHoldDuraction},
            {"ProximityPromptMaxActivationDistance", "ProximityPrompt.MaxActivationDistance", &offsets::ProximityPromptMaxActivationDistance},
            {"ReadOnlyGravity", "Workspace.ReadOnlyGravity", &offsets::ReadOnlyGravity},
            {"RenderJobToDataModel", "RenderJob.RealDataModel", &offsets::RenderJobToDataModel},
            {"RenderJobToFakeDataModel", "RenderJob.FakeDataModel", &offsets::RenderJobToFakeDataModel},
            {"RenderJobToRenderView", "RenderJob.RenderView", &offsets::RenderJobToRenderView},
            {"RenderView", "VisualEngine.RenderView", &offsets::RenderView},
            {"RigType", "Humanoid.RigType", &offsets::RigType},
            {"Rotation", "Primitive.Rotation", &offsets::Rotation},
            {"ScreenGuiEnabled", "GuiObject.ScreenGui_Enabled", &offsets::ScreenGuiEnabled},
            {"ScriptContext", "DataModel.ScriptContext", &offsets::ScriptContext},
            {"Sit", "Humanoid.Sit", &offsets::Sit},
            {"SkyboxBk", "Sky.SkyboxBk", &offsets::SkyboxBk},
            {"SkyboxDn", "Sky.SkyboxDn", &offsets::SkyboxDn},
            {"SkyboxFt", "Sky.SkyboxFt", &offsets::SkyboxFt},
            {"SkyboxLf", "Sky.SkyboxLf", &offsets::SkyboxLf},
            {"SkyboxRt", "Sky.SkyboxRt", &offsets::SkyboxRt},
            {"SkyboxUp", "Sky.SkyboxUp", &offsets::SkyboxUp},
            {"SoundId", "Sound.SoundId", &offsets::SoundId},
            {"StarCount", "Sky.StarCount", &offsets::StarCount},
            {"StringLength", "Misc.StringLength", &offsets::StringLength},
            {"SunTextureId", "Sky.SunTextureId", &offsets::SunTextureId},
            {"TaskSchedulerMaxFPS", "TaskScheduler.MaxFPS", &offsets::TaskSchedulerMaxFPS},
            {"TaskSchedulerPointer", "TaskScheduler.Pointer", &offsets::TaskSchedulerPointer},
            {"Team", "Player.Team", &offsets::Team},
            {"TeamColor", "Player.TeamColor", &offsets::TeamColor},
            {"TextLabelText", "GuiObject.Text", &offsets::TextLabelText},
            {"TextLabelVisible", "GuiObject.Visible", &offsets::TextLabelVisible},
            {"Tool_Grip_Position", "Tool.Grip", &offsets::Tool_Grip_Position},
            {"Transparency", "BasePart.Transparency", &offsets::Transparency},
            {"UserId", "Player.UserId", &offsets::UserId},
            {"Value", "Misc.Value", &offsets::Value},
            {"Velocity", "Primitive.AssemblyLinearVelocity", &offsets::Velocity},
            {"ViewportSize", "Camera.ViewportSize", &offsets::ViewportSize},
            {"VisualEngine", "RenderView.VisualEngine", &offsets::VisualEngine},
            {"VisualEnginePointer", "VisualEngine.Pointer", &offsets::VisualEnginePointer},
            {"VisualEngineToDataModel1", "VisualEngine.FakeDataModel", &offsets::VisualEngineToDataModel1},
            {"VisualEngineToDataModel2", "FakeDataModel.RealDataModel", &offsets::VisualEngineToDataModel2},
            {"WalkSpeed", "Humanoid.Walkspeed", &offsets::WalkSpeed},
            {"WalkSpeedCheck", "Humanoid.WalkspeedCheck", &offsets::WalkSpeedCheck},
            {"Workspace", "DataModel.Workspace", &offsets::Workspace},
            {"WorkspaceToWorld", "Workspace.World", &offsets::WorkspaceToWorld},
            {"viewmatrix", "VisualEngine.ViewMatrix", &offsets::viewmatrix},
        };
        outCount = sizeof(mappings) / sizeof(mappings[0]);
        return mappings;
    }

    inline std::filesystem::path GetCachePath() {
        wchar_t exePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        return std::filesystem::path(exePath).parent_path() / L"cheat_offsets_cache.json";
    }

    inline std::string WideToUtf8(const std::wstring& input) {
        if (input.empty()) return {};
        int size = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
        std::string out(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), out.data(), size, nullptr, nullptr);
        return out;
    }

    inline std::wstring Utf8ToWide(const std::string& input) {
        if (input.empty()) return {};
        int size = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), nullptr, 0);
        std::wstring out(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), out.data(), size);
        return out;
    }

    inline std::string ExtractJsonString(const std::string& json, const char* key) {
        const std::string needle = std::string("\"") + key + "\":\"";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return {};
        pos += needle.size();
        size_t end = json.find('"', pos);
        if (end == std::string::npos) return {};
        return json.substr(pos, end - pos);
    }

    inline size_t SkipWs(const std::string& json, size_t pos) {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t'))
            ++pos;
        return pos;
    }

    inline std::optional<size_t> FindMatchingBrace(const std::string& json, size_t openPos) {
        if (openPos >= json.size() || json[openPos] != '{') return std::nullopt;
        int depth = 0;
        bool inString = false;
        for (size_t i = openPos; i < json.size(); ++i) {
            char c = json[i];
            if (c == '"' && (i == 0 || json[i - 1] != '\\')) inString = !inString;
            if (inString) continue;
            if (c == '{') ++depth;
            else if (c == '}') {
                --depth;
                if (depth == 0) return i;
            }
        }
        return std::nullopt;
    }

    inline std::optional<uint64_t> ParseNumberAt(const std::string& json, size_t pos) {
        pos = SkipWs(json, pos);
        size_t end = pos;
        while (end < json.size() && (isdigit(static_cast<unsigned char>(json[end])) || json[end] == '-'))
            ++end;
        if (end == pos) return std::nullopt;
        try {
            return std::stoull(json.substr(pos, end - pos));
        } catch (...) {
            return std::nullopt;
        }
    }

    inline std::optional<uint64_t> GetNestedOffset(const std::string& json, const char* path) {
        std::string current = json;
        std::string fullPath = path;
        size_t start = 0;

        while (true) {
            size_t dot = fullPath.find('.', start);
            std::string key = (dot == std::string::npos) ? fullPath.substr(start) : fullPath.substr(start, dot - start);
            start = (dot == std::string::npos) ? std::string::npos : dot + 1;

            const std::string quoted = "\"" + key + "\":";
            size_t pos = current.find(quoted);
            if (pos == std::string::npos) return std::nullopt;
            pos += quoted.size();
            pos = SkipWs(current, pos);

            if (start == std::string::npos) {
                return ParseNumberAt(current, pos);
            }

            if (pos >= current.size() || current[pos] != '{') return std::nullopt;
            auto close = FindMatchingBrace(current, pos);
            if (!close) return std::nullopt;
            current = current.substr(pos, *close - pos + 1);
        }
    }

    inline std::optional<std::string> HttpGet(const std::wstring& host, const std::wstring& path) {
        HINTERNET session = WinHttpOpen(L"CHEAT GLOBAL OffsetUpdater/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!session) return std::nullopt;

        HINTERNET connect = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!connect) {
            WinHttpCloseHandle(session);
            return std::nullopt;
        }

        HINTERNET request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!request) {
            WinHttpCloseHandle(connect);
            WinHttpCloseHandle(session);
            return std::nullopt;
        }

        std::string body;
        if (WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD size = 0;
            do {
                DWORD available = 0;
                if (!WinHttpQueryDataAvailable(request, &available) || available == 0) break;
                std::vector<char> buffer(available);
                if (!WinHttpReadData(request, buffer.data(), available, &size)) break;
                body.append(buffer.data(), size);
            } while (size > 0);
        }

        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);

        if (body.empty()) return std::nullopt;
        return body;
    }

    inline std::optional<std::string> DownloadOffsetsJson(const std::string& versionHint) {
        const std::wstring host = L"offsets.imtheo.lol";

        if (!versionHint.empty()) {
            std::wstring path = Utf8ToWide("/" + versionHint + "/offsets.json");
            if (auto data = HttpGet(host, path)) return data;
        }

        return HttpGet(host, L"/offsets.json");
    }

    inline std::string GetInstalledRobloxVersion() {
        if (Coms->IsConnected()) {
            wchar_t pathBuf[MAX_PATH]{};
            DWORD size = MAX_PATH;
            if (QueryFullProcessImageNameW(Coms->GetHandle(), 0, pathBuf, &size)) {
                std::filesystem::path exePath(pathBuf);
                std::string folder = WideToUtf8(exePath.parent_path().filename().wstring());
                if (folder.rfind("version-", 0) == 0) return folder;
            }
        }

        wchar_t localAppData[MAX_PATH]{};
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData))) {
            std::filesystem::path versionsRoot = std::filesystem::path(localAppData) / "Roblox" / "Versions";
            if (std::filesystem::exists(versionsRoot)) {
                std::filesystem::path best;
                std::filesystem::file_time_type bestTime{};
                for (const auto& entry : std::filesystem::directory_iterator(versionsRoot)) {
                    if (!entry.is_directory()) continue;
                    std::string name = WideToUtf8(entry.path().filename().wstring());
                    if (name.rfind("version-", 0) != 0) continue;
                    if (!std::filesystem::exists(entry.path() / "RobloxPlayerBeta.exe")) continue;
                    auto t = entry.last_write_time();
                    if (best.empty() || t > bestTime) {
                        best = entry.path();
                        bestTime = t;
                    }
                }
                if (!best.empty()) return WideToUtf8(best.filename().wstring());
            }
        }

        return {};
    }

    inline bool ValidateCritical(const std::string& json) {
        struct Critical { const char* path; uint64_t minValue; };
        static Critical critical[] = {
            {"FakeDataModel.Pointer", 0x100000},
            {"VisualEngine.Pointer", 0x100000},
            {"FakeDataModel.RealDataModel", 1},
            {"Instance.Name", 1},
            {"Instance.ChildrenStart", 1},
            {"BasePart.Primitive", 1},
        };

        for (const auto& c : critical) {
            auto val = GetNestedOffset(json, c.path);
            if (!val || *val < c.minValue) return false;
        }
        return true;
    }

    inline int ApplyJson(const std::string& json, bool log = false) {
        size_t count = 0;
        MappingEntry* mappings = GetMappings(count);
        int updated = 0;
        int matched = 0;

        for (size_t i = 0; i < count; ++i) {
            auto val = GetNestedOffset(json, mappings[i].apiPath);
            if (!val) continue;
            matched++;
            uintptr_t next = static_cast<uintptr_t>(*val);
            if (*mappings[i].value != next) {
                *mappings[i].value = next;
                updated++;
                if (log) {
                    std::cout << "[+] offset " << mappings[i].key << " = 0x"
                        << std::hex << next << std::dec << "\n";
                }
            }
        }

        if (matched < 25) return -1;
        return updated;
    }

    inline bool SaveCache(const std::string& json) {
        try {
            std::ofstream out(GetCachePath(), std::ios::binary | std::ios::trunc);
            if (!out) return false;
            out << json;
            return true;
        } catch (...) {
            return false;
        }
    }

    inline std::optional<std::string> LoadCache() {
        try {
            std::ifstream in(GetCachePath(), std::ios::binary);
            if (!in) return std::nullopt;
            return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        } catch (...) {
            return std::nullopt;
        }
    }

    inline bool SyncFromJson(const std::string& json, const std::string& sourceLabel, bool log) {
        if (!ValidateCritical(json)) {
            if (log) std::cout << "[!] Offset sync failed: critical validation\n";
            return false;
        }

        int result = ApplyJson(json, log);
        if (result < 0) {
            if (log) std::cout << "[!] Offset sync failed: not enough mappings matched\n";
            return false;
        }

        std::string remoteVersion = ExtractJsonString(json, "Roblox Version");
        if (!remoteVersion.empty()) offsets::RobloxVersion = remoteVersion;
        offsets::OffsetSource = sourceLabel;
        offsets::LastSyncOk = true;
        SaveCache(json);

        if (log) {
            std::cout << "[+] Offsets synced (" << sourceLabel << ") | version "
                << offsets::RobloxVersion << " | changed " << result << "\n";
        }
        return true;
    }

    inline bool EnsureSynced(bool log = false, bool force = false) {
        if (!offsets::AutoSyncEnabled && !force) return offsets::LastSyncOk;

        static auto lastAttempt = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (!force && lastAttempt.time_since_epoch().count() != 0) {
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - lastAttempt).count();
            if (sec < 20) return offsets::LastSyncOk;
        }
        lastAttempt = now;

        const std::string installed = GetInstalledRobloxVersion();
        const bool versionMismatch = !installed.empty() && installed != offsets::RobloxVersion;

        if (!force && !versionMismatch && offsets::LastSyncOk) {
            if (log) std::cout << "[*] Offsets already match " << offsets::RobloxVersion << "\n";
            return true;
        }

        if (log) {
            std::cout << "[*] Auto offset sync";
            if (!installed.empty()) std::cout << " | Roblox " << installed;
            if (versionMismatch) std::cout << " | embedded " << offsets::RobloxVersion;
            std::cout << "\n";
        }

        if (auto cached = LoadCache()) {
            std::string cacheVersion = ExtractJsonString(*cached, "Roblox Version");
            if (!installed.empty() && cacheVersion == installed) {
                if (SyncFromJson(*cached, "cache", log)) return true;
            }
        }

        if (auto downloaded = DownloadOffsetsJson(installed)) {
            std::string remoteVersion = ExtractJsonString(*downloaded, "Roblox Version");
            if (!installed.empty() && !remoteVersion.empty() && remoteVersion != installed) {
                if (log) {
                    std::cout << "[!] Offset service version " << remoteVersion
                        << " != installed " << installed << " (trying anyway)\n";
                }
            }
            if (SyncFromJson(*downloaded, "live", log)) return true;
        }

        if (log) std::cout << "[!] Auto offset sync failed; using embedded offsets\n";
        offsets::LastSyncOk = false;
        return false;
    }
}
