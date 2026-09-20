#pragma once
#include "../../Game/SDK/SDK.h"
#include "../../Game/Offsets/Offsets.h"
#include "../../Game/Offsets/OffsetUpdater.h"
#include "../Globals/Globals.h"
#include "../Cache/Cache.h"
#include "../Vars/Vars.h"
#include "../Features/MM2/MM2.h"
#include "../Features/Misc/Notifications.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace GameInit {

    inline uintptr_t cachedDataModel = 0;
    inline uintptr_t cachedVisualEngine = 0;
    inline uint64_t cachedPlaceId = 0;
    inline bool reattaching = false;
    inline int softFailCount = 0;

    inline bool IsLikelyPointer(uintptr_t addr) {
        return addr >= 0x10000 && addr < 0x7FFFFFFFFFFF && (addr & 0x7) == 0;
    }

    inline void ResetGlobals() {
        Globals::dataModel = RBX::RbxInstance(0);
        Globals::renderEngine = RBX::RenderEngine(0);
        Globals::workspace = RBX::RbxInstance(0);
        Globals::players = RBX::RbxInstance(0);
        Globals::camera = RBX::RbxInstance(0);
        Globals::localPlayer = RBX::RbxInstance(0);
    }

    inline void ClearCaches() {
        PlayerCache::players.clear();
        Vars::Players::selectedAddr = 0;
        MM2::murdererName.clear();
        MM2::sheriffName.clear();
    }

    inline bool IsValidDataModel(uintptr_t addr) {
        if (!IsLikelyPointer(addr)) return false;

        RBX::RbxInstance inst(addr);
        const std::string cls = inst.GetClass();
        if (cls == "DataModel") return true;

        uintptr_t ws = Coms->ReadMemory<uintptr_t>(addr + offsets::Workspace);
        if (IsLikelyPointer(ws)) {
            RBX::RbxInstance wsInst(ws);
            if (wsInst.GetClass() == "Workspace") return true;
        }

        return inst.FindChildByClass("Players").Addr != 0;
    }

    inline uintptr_t DataModelFromFake(uintptr_t fakeAddr) {
        if (!IsLikelyPointer(fakeAddr)) return 0;

        uintptr_t direct = Coms->ReadMemory<uintptr_t>(fakeAddr + offsets::FakeDataModelToDataModel);
        if (IsValidDataModel(direct)) return direct;

        uintptr_t nestedFake = Coms->ReadMemory<uintptr_t>(fakeAddr);
        if (nestedFake != fakeAddr) {
            direct = Coms->ReadMemory<uintptr_t>(nestedFake + offsets::FakeDataModelToDataModel);
            if (IsValidDataModel(direct)) return direct;
        }

        return 0;
    }

    inline uintptr_t DataModelFromRenderJob(uintptr_t jobAddr) {
        if (!IsLikelyPointer(jobAddr)) return 0;

        uintptr_t direct = Coms->ReadMemory<uintptr_t>(jobAddr + offsets::RenderJobToDataModel);
        if (IsValidDataModel(direct)) return direct;

        uintptr_t fake = Coms->ReadMemory<uintptr_t>(jobAddr + offsets::RenderJobToFakeDataModel);
        return DataModelFromFake(fake);
    }

    inline void TryDataModelCandidate(uintptr_t candidate, std::vector<uintptr_t>& out) {
        if (!IsValidDataModel(candidate)) return;
        for (uintptr_t existing : out) {
            if (existing == candidate) return;
        }
        out.push_back(candidate);
    }

    inline bool ReadDataModel(uintptr_t& outDataModel) {
        uintptr_t baseAddr = Coms->GetBase();
        if (baseAddr == 0) return false;

        std::vector<uintptr_t> candidates;
        candidates.reserve(8);

        uintptr_t fakeDirect = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::FakeDataModelPointer);
        TryDataModelCandidate(DataModelFromFake(fakeDirect), candidates);

        uintptr_t fakeIndirect = Coms->ReadMemory<uintptr_t>(fakeDirect);
        TryDataModelCandidate(DataModelFromFake(fakeIndirect), candidates);

        uintptr_t visualEngine = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::VisualEnginePointer);
        if (IsLikelyPointer(visualEngine)) {
            uintptr_t fakeFromVe = Coms->ReadMemory<uintptr_t>(visualEngine + offsets::VisualEngineToDataModel1);
            TryDataModelCandidate(DataModelFromFake(fakeFromVe), candidates);

            uintptr_t renderView = Coms->ReadMemory<uintptr_t>(visualEngine + offsets::RenderView);
            if (IsLikelyPointer(renderView)) {
                fakeFromVe = Coms->ReadMemory<uintptr_t>(renderView + offsets::VisualEngineToDataModel1);
                TryDataModelCandidate(DataModelFromFake(fakeFromVe), candidates);
            }
        }

        uintptr_t scheduler = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::TaskSchedulerPointer);
        if (IsLikelyPointer(scheduler)) {
            uintptr_t jobStart = Coms->ReadMemory<uintptr_t>(scheduler + offsets::JobStart);
            uintptr_t jobEnd = Coms->ReadMemory<uintptr_t>(scheduler + offsets::JobEnd);

            if (IsLikelyPointer(jobStart) && IsLikelyPointer(jobEnd) && jobEnd > jobStart) {
                const size_t jobCount = (jobEnd - jobStart) / 0x10;
                const size_t maxJobs = jobCount > 128 ? 128 : jobCount;

                for (size_t i = 0; i < maxJobs; ++i) {
                    uintptr_t job = Coms->ReadMemory<uintptr_t>(jobStart + i * 0x10);
                    TryDataModelCandidate(DataModelFromRenderJob(job), candidates);
                }
            }
        }

        if (candidates.empty()) return false;

        if (cachedDataModel != 0) {
            for (uintptr_t c : candidates) {
                if (c == cachedDataModel) {
                    outDataModel = c;
                    return true;
                }
            }
        }

        outDataModel = candidates.front();
        return true;
    }

    inline uintptr_t ReadVisualEngine(uintptr_t baseAddr) {
        uintptr_t visualEngine = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::VisualEnginePointer);
        if (IsLikelyPointer(visualEngine)) return visualEngine;

        uintptr_t scheduler = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::TaskSchedulerPointer);
        if (!IsLikelyPointer(scheduler)) return 0;

        uintptr_t jobStart = Coms->ReadMemory<uintptr_t>(scheduler + offsets::JobStart);
        uintptr_t jobEnd = Coms->ReadMemory<uintptr_t>(scheduler + offsets::JobEnd);
        if (!IsLikelyPointer(jobStart) || !IsLikelyPointer(jobEnd) || jobEnd <= jobStart) return 0;

        const size_t jobCount = (jobEnd - jobStart) / 0x10;
        const size_t maxJobs = jobCount > 128 ? 128 : jobCount;

        for (size_t i = 0; i < maxJobs; ++i) {
            uintptr_t job = Coms->ReadMemory<uintptr_t>(jobStart + i * 0x10);
            if (!IsLikelyPointer(job)) continue;

            uintptr_t renderView = Coms->ReadMemory<uintptr_t>(job + offsets::RenderJobToRenderView);
            if (!IsLikelyPointer(renderView)) continue;

            visualEngine = Coms->ReadMemory<uintptr_t>(renderView + offsets::VisualEngine);
            if (IsLikelyPointer(visualEngine)) return visualEngine;
        }

        return 0;
    }

    inline RBX::RbxInstance ResolveWorkspace(RBX::RbxInstance& dataModel) {
        uintptr_t wsAddr = Coms->ReadMemory<uintptr_t>(dataModel.Addr + offsets::Workspace);
        if (IsLikelyPointer(wsAddr)) {
            RBX::RbxInstance ws(wsAddr);
            if (ws.GetClass() == "Workspace") return ws;
        }

        auto workspace = dataModel.FindChildByClass("Workspace");
        if (workspace.Addr != 0) return workspace;

        return RBX::RbxInstance(0);
    }

    inline RBX::RbxInstance ResolvePlayers(RBX::RbxInstance& dataModel) {
        auto players = dataModel.FindChildByClass("Players");
        if (players.Addr != 0) return players;
        return RBX::RbxInstance(0);
    }

    inline RBX::RbxInstance ResolveCamera(RBX::RbxInstance& workspace) {
        uintptr_t camAddr = Coms->ReadMemory<uintptr_t>(workspace.Addr + offsets::Camera);
        if (IsLikelyPointer(camAddr)) {
            RBX::RbxInstance cam(camAddr);
            if (cam.GetClass() == "Camera") return cam;
        }

        auto camera = workspace.FindChildByClass("Camera");
        if (camera.Addr != 0) return camera;

        return RBX::RbxInstance(0);
    }

    inline bool AttachGlobals(bool log = false) {
        if (!Coms->IsConnected() && !Coms->Reconnect()) {
            if (log) std::cout << "[!] Attach failed: Roblox not connected\n";
            return false;
        }

        Coms->RefreshModuleBase();

        uintptr_t baseAddr = Coms->GetBase();
        if (baseAddr == 0) {
            if (log) std::cout << "[!] Attach failed: no base address\n";
            return false;
        }

        uintptr_t dataModelPtr = 0;
        if (!ReadDataModel(dataModelPtr)) {
            if (log) std::cout << "[!] Attach failed: DataModel pointer\n";
            return false;
        }

        uintptr_t visualEngine = ReadVisualEngine(baseAddr);
        if (visualEngine == 0) {
            if (log) std::cout << "[!] Attach failed: VisualEngine pointer\n";
            return false;
        }

        RBX::RbxInstance dataModel(dataModelPtr);
        auto workspace = ResolveWorkspace(dataModel);
        auto players = ResolvePlayers(dataModel);
        if (workspace.Addr == 0 || players.Addr == 0) {
            if (log) std::cout << "[!] Attach failed: Workspace/Players (join a game)\n";
            return false;
        }

        uintptr_t localPlayerAddr = Coms->ReadMemory<uintptr_t>(players.Addr + offsets::LocalPlayer);
        if (localPlayerAddr != 0 && !IsLikelyPointer(localPlayerAddr))
            localPlayerAddr = 0;

        Globals::dataModel = dataModel;
        Globals::renderEngine = RBX::RenderEngine(visualEngine);
        Globals::workspace = workspace;
        Globals::players = players;
        Globals::camera = ResolveCamera(workspace);
        Globals::localPlayer = localPlayerAddr != 0 ? RBX::RbxInstance(localPlayerAddr) : RBX::RbxInstance(0);

        cachedDataModel = dataModelPtr;
        cachedVisualEngine = visualEngine;
        cachedPlaceId = Coms->ReadMemory<uint64_t>(dataModelPtr + offsets::PlaceId);
        softFailCount = 0;

        if (log) {
            bool gameLoaded = Coms->ReadMemory<bool>(dataModelPtr + offsets::GameLoaded);
            std::cout << "[+] Game attached | PlaceId: " << cachedPlaceId
                << " | Loaded: " << (gameLoaded ? "yes" : "no")
                << " | LocalPlayer: " << (localPlayerAddr != 0 ? "yes" : "lobby")
                << " | DataModel: 0x" << std::hex << dataModelPtr << std::dec << "\n";
        }

        return true;
    }

    inline void RefreshRuntimeLight() {
        if (Globals::dataModel.Addr != 0) {
            uintptr_t wsAddr = Coms->ReadMemory<uintptr_t>(Globals::dataModel.Addr + offsets::Workspace);
            if (IsLikelyPointer(wsAddr)) Globals::workspace = RBX::RbxInstance(wsAddr);
        }

        if (Globals::players.Addr != 0) {
            uintptr_t lp = Coms->ReadMemory<uintptr_t>(Globals::players.Addr + offsets::LocalPlayer);
            if (IsLikelyPointer(lp)) Globals::localPlayer = RBX::RbxInstance(lp);
        }

        if (Globals::workspace.Addr != 0) {
            uintptr_t camAddr = Coms->ReadMemory<uintptr_t>(Globals::workspace.Addr + offsets::Camera);
            if (IsLikelyPointer(camAddr)) Globals::camera = RBX::RbxInstance(camAddr);
        }

        uintptr_t baseAddr = Coms->GetBase();
        if (baseAddr != 0) {
            uintptr_t ve = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::VisualEnginePointer);
            if (IsLikelyPointer(ve)) Globals::renderEngine = RBX::RenderEngine(ve);
        }
    }

    inline void RefreshRuntime() {
        RefreshRuntimeLight();

        if (Globals::dataModel.Addr != 0) {
            auto workspace = ResolveWorkspace(Globals::dataModel);
            if (workspace.Addr != 0) Globals::workspace = workspace;

            auto players = ResolvePlayers(Globals::dataModel);
            if (players.Addr != 0) Globals::players = players;
        }

        if (Globals::workspace.Addr != 0) {
            auto cam = ResolveCamera(Globals::workspace);
            if (cam.Addr != 0) Globals::camera = cam;
        }
    }

    inline bool QuickStillAttached() {
        if (!Coms->IsConnected() || Globals::dataModel.Addr == 0 || cachedDataModel == 0)
            return false;

        uintptr_t baseAddr = Coms->GetBase();
        if (baseAddr == 0) return false;

        uintptr_t fake = Coms->ReadMemory<uintptr_t>(baseAddr + offsets::FakeDataModelPointer);
        if (!IsLikelyPointer(fake)) return false;

        uintptr_t dm = Coms->ReadMemory<uintptr_t>(fake + offsets::FakeDataModelToDataModel);
        return dm == cachedDataModel;
    }

    inline bool NeedsHardReattachFull() {
        if (!Coms->IsConnected()) return true;

        if (Globals::dataModel.Addr == 0 || cachedDataModel == 0)
            return true;

        uintptr_t currentDataModel = 0;
        if (!ReadDataModel(currentDataModel) || currentDataModel == 0)
            return true;

        if (currentDataModel != cachedDataModel)
            return true;

        uintptr_t baseAddr = Coms->GetBase();
        if (baseAddr == 0) return true;

        uintptr_t visualEngine = ReadVisualEngine(baseAddr);
        if (visualEngine != 0 && cachedVisualEngine != 0 && visualEngine != cachedVisualEngine)
            return true;

        uint64_t placeId = Coms->ReadMemory<uint64_t>(currentDataModel + offsets::PlaceId);
        if (placeId != 0 && cachedPlaceId != 0 && placeId != cachedPlaceId)
            return true;

        return false;
    }

    inline bool NeedsHardReattach() {
        if (!QuickStillAttached()) return true;

        static auto lastFullCheck = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (lastFullCheck.time_since_epoch().count() != 0) {
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - lastFullCheck).count();
            if (sec < 5) return false;
        }
        lastFullCheck = now;
        return NeedsHardReattachFull();
    }

    inline bool NeedsSoftHeal() {
        if (Globals::dataModel.Addr == 0) return true;
        if (Globals::workspace.Addr == 0 || Globals::players.Addr == 0) return true;
        return false;
    }

    inline bool Update() {
        static auto lastAttempt = std::chrono::steady_clock::time_point{};
        static auto lastVersionCheck = std::chrono::steady_clock::time_point{};
        static auto lastRefresh = std::chrono::steady_clock::time_point{};
        static auto lastUpdateTick = std::chrono::steady_clock::time_point{};
        static int attachFailCount = 0;

        auto now = std::chrono::steady_clock::now();
        if (Globals::dataModel.Addr != 0 && !reattaching) {
            if (lastUpdateTick.time_since_epoch().count() != 0) {
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTick).count();
                if (ms < 100)
                    return Globals::players.Addr != 0;
            }
        }
        lastUpdateTick = now;

        if (!Coms->IsConnected()) {
            if (!Coms->Reconnect()) return false;
            ClearCaches();
            ResetGlobals();
            reattaching = true;
            cachedDataModel = 0;
            cachedVisualEngine = 0;
            cachedPlaceId = 0;
        }

        if (lastVersionCheck.time_since_epoch().count() == 0 ||
            std::chrono::duration_cast<std::chrono::seconds>(now - lastVersionCheck).count() >= 45) {
            lastVersionCheck = now;
            const std::string installed = OffsetUpdater::GetInstalledRobloxVersion();
            if (!installed.empty() && installed != offsets::RobloxVersion && offsets::AutoSyncEnabled) {
                OffsetUpdater::EnsureSynced(false, true);
            }
        }

        if (!NeedsHardReattach()) {
            if (NeedsSoftHeal()) {
                RefreshRuntime();
                softFailCount++;
            } else {
                softFailCount = 0;
            }

            if (softFailCount < 12) {
                reattaching = false;
                if (lastRefresh.time_since_epoch().count() == 0 ||
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRefresh).count() >= 750) {
                    RefreshRuntimeLight();
                    lastRefresh = now;
                }
                return Globals::dataModel.Addr != 0 && Globals::players.Addr != 0;
            }
        }

        if (!reattaching) {
            ClearCaches();
            ResetGlobals();
            reattaching = true;
            cachedDataModel = 0;
            cachedVisualEngine = 0;
            cachedPlaceId = 0;
            softFailCount = 0;
        }

        now = std::chrono::steady_clock::now();
        if (lastAttempt.time_since_epoch().count() != 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttempt).count();
            if (ms < 250) return Globals::dataModel.Addr != 0;
        }
        lastAttempt = now;

        if (AttachGlobals(false)) {
            reattaching = false;
            softFailCount = 0;
            attachFailCount = 0;
            static uint64_t lastNotifiedPlace = 0;
            if (cachedPlaceId != 0 && cachedPlaceId != lastNotifiedPlace) {
                Notify::Push("Game", "Joined place " + std::to_string(cachedPlaceId), Notify::Type::Info, 4.f);
                lastNotifiedPlace = cachedPlaceId;
            }
            return true;
        }

        attachFailCount++;
        if (attachFailCount >= 8 && offsets::AutoSyncEnabled) {
            attachFailCount = 0;
            if (OffsetUpdater::EnsureSynced(false, true)) {
                cachedDataModel = 0;
                cachedVisualEngine = 0;
            }
        }

        return Globals::dataModel.Addr != 0;
    }
}
