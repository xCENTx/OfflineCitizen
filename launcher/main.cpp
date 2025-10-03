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

	///	Attach to StarCitizen Process
	exMemory mem("StarCitizen.exe");
	if (!mem.bAttached)
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to attach to star citizen process.", 6);
	}
	printf("[+] attached to star citizen process.\n");

	///	Patch EAC Events
	const auto& fn = mem.FindPattern("E8 ? ? ? ? 40 88 B7 ? ? ? ? 48 8B CF", 0, EASM::ASM_CALL);
	if (!fn)
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to find EAC event function.", 7);
	}
	printf("[+] found EAC event function at address 0x%p\n", (void*)fn);

	std::vector<unsigned char> patch = { 0xC3 };
	if (!mem.PatchMemory(fn, patch.data(), patch.size()))
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to patch EAC event function.", 8);
	}
	printf("[+] patched EAC event function.\n");

	///	Patch Game Crash Reporter
	const auto fnCrashReporter = mem.FindPattern("BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0");
	if (!fnCrashReporter)
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to find game crash reporter function.", 9);
	}
	printf("[+] found game crash reporter function at address 0x%p\n", (void*)fnCrashReporter);

	std::vector<unsigned char> patchCrashReporter = { 0x90, 0xE9 };
	if (!mem.PatchMemory(fnCrashReporter, patchCrashReporter.data(), patchCrashReporter.size()))
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to patch game crash reporter function.", 10);
	}
	printf("[+] patched game crash reporter function.\n");

	{

		std::string path = DLL_PATH;

		//  allocate memory
		void* addr = VirtualAllocEx(pInfo.hProcess, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!addr)
		{
			TerminateProcess(pInfo.hProcess);
			return exitWithCode("[!] failed to allocate memory in star citizen process.", 11);
		}
		printf("[+] allocated memory in star citizen process at address 0x%p\n", addr);

		//  write to memory
		if (!WriteProcessMemory(pInfo.hProcess, addr, path.c_str(), path.size() + 1, 0))
		{
			VirtualFreeEx(pInfo.hProcess, addr, 0, MEM_RELEASE);
			TerminateProcess(pInfo.hProcess);
			return exitWithCode("[!] failed to write memory in star citizen process.", 12);
		}
		printf("[+] wrote module path to star citizen process memory.\n");

		//  create thread
		HANDLE hThread = CreateRemoteThread(pInfo.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, addr, 0, 0);
		if (!hThread)
		{
			VirtualFreeEx(pInfo.hProcess, addr, 0, MEM_RELEASE);
			TerminateProcess(pInfo.hProcess);
			return exitWithCode("[!] failed to create remote thread in star citizen process.", 13);
		}
		WaitForSingleObject(hThread, INFINITE);
		printf("[+] created remote thread in star citizen process.\n");

		DWORD exitCode = 0;
		GetExitCodeThread(hThread, &exitCode);
		if (!exitCode)
		{
			TerminateProcess(pInfo.hProcess);
			return exitWithCode("[!] failed to load test module in star citizen process.", 14);
		}
		printf("[+] injected module at address 0x%p\n", (void*)exitCode);

		//  close handle
		CloseHandle(hThread);
	}

	///	Resume StarCitizen Process
	if (ResumeThread(pInfo.hThread) == -1)
	{
		TerminateProcess(pInfo.hProcess);
		return exitWithCode("[!] failed to resume star citizen process.", 15);
	}
	printf("[+] resumed star citizen process.\n");

	///	Wait for StarCitizen Process to exit
	bool bInjected{ false };
	while (mem.bAttached)
	{
		if (WaitForSingleObject(pInfo.hProcess, INFINITE) == WAIT_OBJECT_0)
			break;
	}


	mem.Detach();
	CloseHandle(pInfo.hProcess);

	return exitWithCode("[+] star citizen process exited.", 0);
}