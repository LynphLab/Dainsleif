﻿#include "pch.h"
#include "RWtoml.h"
#include "Hacks/Aimbot.h"
#include "Hacks/Glow.h"
#include "Hacks/AntiRecoil.h"
#include "Hacks/Triggerbot.h"
#include "Hook/GraphicHook.h"

bool bQuit, bAimbot, bGlowHack, bAntiRecoil, bTriggerBot;
int fov;
bool g_ShowMenu = false;

TCHAR dir[ MAX_PATH ];
std::string filename;//const char* dir = "C:/Users/PC/Dainsleif"; //directory savedata will be saved.

VOID WINAPI Detach(LPVOID lpParameter)
{
    unhookEndScene();

    fclose(stdout);
    FreeConsole();
}

DWORD WINAPI fMain(LPVOID lpParameter)
{
    //Create console window
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

    SHGetSpecialFolderPath(NULL, dir, CSIDL_COMMON_DOCUMENTS, 0); //Find the Document directory location
    filename = static_cast<std::string>(dir) + "/Dainsleif/savedata.toml"; //Set file path.

    std::filesystem::path path{filename};
    std::filesystem::create_directories(path.parent_path());
    if (!std::filesystem::exists(path))
    {
        std::ofstream stream{path};
        stream.close();
    }

    RWtoml::ParseFile(filename);

    hookEndScene();

    std::vector<Entity*> entityList;

    while (true)
    {
        static bool inGame = false;
        if (bQuit)
        {
            RWtoml::WriteFile(filename);
            break;
        }

        int gameState = *reinterpret_cast<int*>((*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(GetModuleHandle("engine.dll")) + dwClientState) + dwClientState_State));

        if (gameState != 6) {   //Not 6 means user's in menu.
            if (inGame) //true means user used to be in game.
            {
                RWtoml::WriteFile(filename);
                inGame = false;
            }
            g_ShowMenu = false;
        }

        if (gameState == 6 && !inGame)
            inGame = true;

        if (!*reinterpret_cast<uintptr_t*>(GetLocalPlayer()))
            continue;

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            if (gameState == 6 && *reinterpret_cast<uintptr_t*>(GetLocalPlayer()))
            {
                g_ShowMenu = !g_ShowMenu;
                if (!g_ShowMenu)
                    RWtoml::WriteFile(filename);
            }
        }

        static bool bInitLocalPlayer = false;
        if (!bInitLocalPlayer) {
            GetLocalPlayer()->SetFOV(fov);
        }

        if (bAimbot || bTriggerBot || bGlowHack || bAntiRecoil) {
            entityList = GetEntities();
        }

        if (bAimbot)
        {
            Aimbot::Run(entityList);
        }

        if (bGlowHack)
        {
            for (Entity* ent : entityList)
            {
                Glow::Run(ent);
            }
        }

        if (bAntiRecoil)
        {
            AntiRecoil::Run();
        }

        if (bTriggerBot)
        {
            Triggerbot::Run(entityList);
        }

        Sleep(1); //sleep for performance aspect
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParameter), EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        HANDLE hThread = CreateThread(nullptr, 0, fMain, hModule, 0, nullptr);
        if (hThread)
        {
            CloseHandle(hThread);
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH && !lpReserved)
    {
        Detach(hModule);
    }
    return TRUE;
}