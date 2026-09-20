#include "Memory/Communication.h"
#include "Game/Offsets/Offsets.h"
#include "Game/SDK/SDK.h"
#include "Render/Render.h"
#include "Render/Menu.h"
#include "Core/Globals/Globals.h"
#include "Core/Cache/Cache.h"
#include "Core/Features/Visuals/Visuals.h"
#include "Core/Features/Aimbot/Aimbot.h"
#include "Core/Features/TriggerBot/TriggerBot.h"
#include "Core/Features/Movement/Movement.h"
#include "Core/Features/BladeBall/BladeBall.h"
#include "Core/Features/PlayerList/PlayerList.h"
#include "Core/Features/Misc/AntiAFK.h"
#include "Core/Features/Misc/AutoClicker.h"
#include "Core/Features/Misc/Notifications.h"
#include "Core/Features/Teleport/Teleport.h"
#include "Core/Game/GameInit.h"
#include "Game/Offsets/OffsetUpdater.h"
#include "Core/Config/Config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <atomic>

bool IsGameRunning(const wchar_t* windowTitle)
{
    HWND hwnd = FindWindowW(NULL, windowTitle);
    return hwnd != NULL;
}

std::atomic<bool> running(true);


void LocalPlayerThread() {
    while (running) {
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        auto humanoid = character.FindChildByClass("Humanoid");
        if (humanoid.Addr == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (Vars::Local::speedEnabled) {
            RBX::ModifyWalkSpeed(humanoid, Vars::Local::walkSpeed);
        }

        if (Vars::Local::jumpEnabled) {
            RBX::ModifyJumpPower(humanoid, Vars::Local::jumpPower);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}




int main() {
    std::cout << "[*] Searching for Roblox...\n";
    
    while (!Coms->Connect(L"RobloxPlayerBeta.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    system("cls");
    
    auto baseAddr = Coms->GetBase();
    std::cout << "[+] Process ID: " << Coms->GetPID() << "\n";
    std::cout << "[+] Base Address: 0x" << std::hex << baseAddr << std::dec << "\n";

    OffsetUpdater::EnsureSynced(true, true);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    system("cls");

    std::cout << "[*] Waiting for game attach...\n";
    while (!GameInit::Update()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    GameInit::AttachGlobals(true);

    std::cout << "[+] VisualEngine: 0x" << std::hex << Globals::renderEngine.Addr << std::dec << "\n";
    std::cout << "[+] Workspace: 0x" << std::hex << Globals::workspace.Addr << std::dec << "\n";
    std::cout << "[+] Players: 0x" << std::hex << Globals::players.Addr << std::dec << "\n";
    std::cout << "[+] Camera: 0x" << std::hex << Globals::camera.Addr << std::dec << "\n";
    std::cout << "[+] LocalPlayer: 0x" << std::hex << Globals::localPlayer.Addr << std::dec << "\n\n";
    

    OverlayWindow overlay;
    if (!overlay.Initialize()) {
        std::cout << "[!] Failed to initialize overlay\n";
        return -1;
    }

    std::cout << "[+] Overlay initialized\n";
    std::cout << "[*] CHEAT GLOBAL by Hz1337\n";
    std::cout << "[*] Press INSERT to toggle menu\n";
    ConfigSystem::LoadDefaultOnStartup();
    std::cout << "[*] Config folder: " << ConfigSystem::GetConfigDir().string() << "\n\n";
    Notify::Push("CHEAT GLOBAL", "Ready | INSERT for menu", Notify::Type::Info, 5.f);

    std::thread localThread(LocalPlayerThread);
    BladeBall::RunThread();
    AutoClicker::RunThread();

    static float cacheTimer = 0.f;
    static float mm2Timer = 0.f;
    static float gameCheckTimer = 0.f;
    constexpr float kCacheInterval = 1.f / 20.f;
    constexpr float kMm2Interval = 1.5f;

    auto NeedsGameplayFeatures = []() {
        return Vars::ESP::enabled || Vars::Chams::enabled || Vars::Aimbot::enabled
            || Vars::TriggerBot::enabled || Vars::Teleport::clickTpEnabled
            || (Vars::MM2::enabled && Vars::MM2::roleESP);
    };

    while (Coms->IsConnected()) 
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Vars::menuOpen = !Vars::menuOpen;
        }

        GameInit::Update();

        const bool gameReady = Globals::dataModel.Addr != 0
            && Globals::renderEngine.Addr != 0
            && Globals::players.Addr != 0;

        overlay.BeginFrame();

        const float dt = ImGui::GetIO().DeltaTime > 0.f ? ImGui::GetIO().DeltaTime : 0.016f;

        gameCheckTimer += dt;
        if (gameCheckTimer >= 1.f) {
            gameCheckTimer = 0.f;
            if (!IsGameRunning(L"Roblox")) break;
        }

        if (gameReady) {
            cacheTimer += dt;
            if (PlayerCache::NeedsUpdate() && cacheTimer >= kCacheInterval) {
                PlayerCache::UpdatePlayers(cacheTimer);
                cacheTimer = 0.f;
            }

            mm2Timer += dt;
            if (Vars::MM2::enabled && mm2Timer >= kMm2Interval) {
                MM2::UpdateRoleNames();
                mm2Timer = 0.f;
            }

            PlayerListFeature::UpdateBringHold();
            Movement::Update(dt);
            AntiAFK::Update();
        }
        
        overlay.RenderMenu();
        Notify::Update();
        Notify::Render();
                   
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        static auto lastTime = std::chrono::high_resolution_clock::now();
        static int frameCount = 0;
        static int fps = 0;

        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

        if (elapsed >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }

        MenuUI::RenderWatermark(fps);

        if (Vars::Aimbot::enabled && Vars::Aimbot::showFOV) {
            POINT p;
            GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), 32, 2.0f);
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(255, 255, 255, 255), 32, 1.0f);
        }

        if (gameReady && NeedsGameplayFeatures()) {
            auto viewMatrix = Globals::renderEngine.GetViewMat();
            if (Vars::Teleport::clickTpEnabled)
                TeleportFeatures::UpdateClickTP(viewMatrix);
            if (Vars::Aimbot::enabled)
                Aimbot::RunAimbot(viewMatrix);
            if (Vars::TriggerBot::enabled)
                TriggerBot::Run(viewMatrix);
            if (Vars::ESP::enabled || Vars::Chams::enabled || (Vars::MM2::enabled && Vars::MM2::roleESP))
                Visuals::RenderESP(drawList, viewMatrix);
        }
        
        overlay.EndFrame();
    }
    
    running = false;
    BladeBall::StopThread();
    AutoClicker::StopThread();
    localThread.join();

    overlay.Cleanup();
    
    return 0;
}
