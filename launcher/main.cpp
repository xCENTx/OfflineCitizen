#pragma once
#include <windows.h>
#include <string>
#include <fstream>

//	LIBS
#include <exMemory/exMemory.hpp>
#include <json/json.hpp>
#include <user-definitions.h>

//	write message to console, appends new line and waits for key input to exit
int exitWithCode(const std::string& msg, const int& code)
{
	printf("%s\npress any key to exit . . .\n", msg.c_str());
	//	getchar();	//	wait for key input
	return code;
}

int TerminateProcess(HANDLE hProcess, DWORD exitCode = 0)
{
	if (!hProcess)
		return false;

	static LPTHREAD_START_ROUTINE fnExitProcess = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "ExitProcess");
	if (!fnExitProcess)
		return false;

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, fnExitProcess, (LPVOID)exitCode, 0, 0);
	if (!hThread)
		return false;

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return true;
}

int main()
{
	AllocConsole();

	///	Read JSON file 'settings.json' and obtain the 'params' object
	std::ifstream settingsFile(LAUNCH_PARAMS);
	if (!settingsFile.is_open())
		return exitWithCode("[!] failed to open file.", 1);
	printf("[+] opened settings file.\n");

	nlohmann::json settings;
	try { settingsFile >> settings; }
	catch (nlohmann::json::parse_error& e)
	{
		printf("%s\n\n", e.what());	//	print error message
		return exitWithCode("[!] failed to parse file.", 2);
	}
	settingsFile.close();
	printf("[+] parsed settings file.\n");

	std::string params = settings["parameters"];
	if (params.empty())
		return exitWithCode("[!] failed to read parameters from file.", 3);
	printf("[+] read launch parameters from file.\n");

	///	Create StarCitizen Process with launch parameters from 'params' object
	STARTUPINFO sInfo;
	PROCESS_INFORMATION pInfo;
	ZeroMemory(&sInfo, sizeof(sInfo));
	sInfo.cb = sizeof(sInfo);
	ZeroMemory(&pInfo, sizeof(pInfo));
	if (!CreateProcessA(STAR_CITIZEN_PATH, LPSTR(params.c_str()), 0, 0, false, CREATE_SUSPENDED, 0, 0, (LPSTARTUPINFOA)&sInfo, &pInfo))
		return exitWithCode("[!] failed to create star citizen process.", 4);
	printf("[+] created star citizen process.\n");

	///	Inject DLL into StarCitizen Process
	if (!exMemory::LoadLibraryInjectorEx(pInfo.hProcess, DLL_PATH))
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to inject module into star citizen process.", 6);
	}

	/// Wait for DLL to load
	// launcher side
	HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, L"OfflineCitizen_Init_Finished");
	if (hEvent) {
		// after injection (or before, doesn't matter), wait:
		DWORD wait = WaitForSingleObject(hEvent, 10000 /* timeout ms or INFINITE */);
		if (wait == WAIT_OBJECT_0) {
			// DLL signalled completion
		}
		CloseHandle(hEvent);
	}

	///	Resume StarCitizen Process
	if (ResumeThread(pInfo.hThread) == -1)
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to resume star citizen process.", 15);
	}
	printf("[+] resumed star citizen process.\n");

	///	Wait for StarCitizen Process to exit
	while (true)
	{
		if (WaitForSingleObject(pInfo.hProcess, INFINITE) == WAIT_OBJECT_0)
			break;
	}

	CloseHandle(pInfo.hProcess);

	return exitWithCode("[+] star citizen process exited.", 0);
}