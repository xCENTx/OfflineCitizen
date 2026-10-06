#pragma once

inline bool bRunning = false;
inline __int64 dwModule = 0;

DWORD hack(LPVOID hModule);

void init();
void shutdown();


void initAppWindow();
void shutdownAppWindow();


void initHooks();
void initConsole();