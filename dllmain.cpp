#include <pch.h>
#include "hack.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        if (HANDLE hThread = CreateThread(0, 0, hack, hModule, 0, 0))
            CloseHandle(hThread);
    }
    return true;
}

