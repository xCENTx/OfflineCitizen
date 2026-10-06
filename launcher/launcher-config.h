#pragma once
// Local paths, resolved relative to the launcher working directory.
// Copy the built launcher and SoloCitizen.dll beside StarCitizen.exe,
// or replace these defaults with paths to your own installation.
#ifndef LAUNCH_PARAMS
#define LAUNCH_PARAMS "launch-settings.json"
#endif
#ifndef STAR_CITIZEN_PATH
#define STAR_CITIZEN_PATH "StarCitizen.exe"
#endif
#ifndef DLL_PATH
#define DLL_PATH "SoloCitizen.dll"
#endif
