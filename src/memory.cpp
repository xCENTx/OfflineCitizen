#include <pch.h>
#include "memory.h"
#include <exMemory/exMemory.hpp>	//	

bool memory::hooker::bInitialized = false;

#define HK_ASSERT assert(memory::hooker::bInitialized != false);

//	obtains access to protected members and methods of exMemory
class exMemoryHelper : protected exMemory
{
public:
	using exMemory::GetProcWindowEx;
	using exMemory::EnumWindowData;
};

bool memory::init()
{
	dwPID = GetCurrentProcessId();
	dwModule = reinterpret_cast<__int64>(GetModuleHandleA(nullptr));
	scMenu = GetMainProcWndw();


	MH_STATUS status = MH_Initialize();
	if (status != MH_OK)
	{
		printf("MH_Initialize failed with status: %s\n", MH_StatusToString(status));
		return false;
	}

	hooker::bInitialized = true;

	return true;
}

bool memory::shutdown()
{
	dwPID = 0;
	dwModule = 0;
	scMenu = nullptr;

	MH_DisableHook(MH_ALL_HOOKS);

	hooker::bInitialized = false;

	return MH_Uninitialize() == MH_OK;
}

void memory::update()
{

}

HWND memory::GetMainProcWndw()
{
	exMemoryHelper::EnumWindowData eDat;
	eDat.procId = GetCurrentProcessId();
	if (EnumWindows(exMemoryHelper::GetProcWindowEx, reinterpret_cast<LPARAM>(&eDat)))
		return eDat.hwnd;
	return nullptr;
}

HWND memory::GetProcWndw(const std::string& wndwTitle)
{
	HWND scMenu = FindWindowExA(0, 0, 0, wndwTitle.c_str());
	if (!scMenu || scMenu == INVALID_HANDLE_VALUE)
		return nullptr;

	DWORD dwPID = GetCurrentProcessId();
	if (!GetWindowThreadProcessId(scMenu, &dwPID))
		return nullptr;

	return scMenu;
}

bool memory::Patch(const unsigned __int64& addr, const std::vector<unsigned char>& patch)
{
	DWORD oldprotect;
	size_t size = patch.size();
	if (size == 0)
		return false;

	if (!VirtualProtect(reinterpret_cast<void*>(addr), size, PAGE_EXECUTE_READWRITE, &oldprotect))
		return false;

	memcpy(reinterpret_cast<void*>(addr), patch.data(), size);

	return VirtualProtect(reinterpret_cast<void*>(addr), size, oldprotect, &oldprotect) != 0;
}

bool memory::hooker::Create(void* pTarget, void** Original, void* Function)
{
	HK_ASSERT;

	MH_STATUS status = MH_CreateHook(pTarget, Function, Original);
	if (status != MH_OK)
	{
		printf("MH_CreateHook failed with status: %s\n", MH_StatusToString(status));
		return false;
	}

	status = MH_EnableHook(pTarget);
	if (status != MH_OK)
	{
		printf("MH_EnableHook failed with status: %s\n", MH_StatusToString(status));
		return false;
	}

	return true;
}

bool memory::hooker::Disable(void* pTarget, const bool& bDisableAll)
{
	HK_ASSERT;

	if (!bInitialized)
	{
		printf("Memory hooker not initialized.\n");
		return false;
	}

	MH_STATUS status = bDisableAll ? MH_DisableHook(MH_ALL_HOOKS) : MH_DisableHook(pTarget);
	if (status != MH_OK)
	{
		printf("MH_DisableHook failed with status: %s\n", MH_StatusToString(status));
		return false;
	}
	return true;
}

bool memory::hooker::Remove(void* pTarget)
{
	HK_ASSERT;

	if (!Disable(pTarget))
		return false;

	MH_STATUS status = MH_RemoveHook(pTarget);
	if (status != MH_OK)
	{
		printf("MH_RemoveHook failed with status: %s\n", MH_StatusToString(status));
		return false;
	}
	return true;
}

///-------------------------------------------------------------------------------------------------
#include <fstream>
#include <sstream>
//	#include <shellapi.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
//	#include "include/Server.h"

//	#include <commctrl.h>
//	#pragma comment(lib, "comctl32.lib")

static exCrashHandler::NetworkCrashCallback g_NetworkCrashCallback = nullptr;

void exCrashHandler::RegisterNetworkCallback(exCrashHandler::NetworkCrashCallback cb)
{
	g_NetworkCrashCallback = cb;
}

void exCrashHandler::SetCrashHandler(const wchar_t* logPath, const wchar_t* dumpPath)
{
	_logPath = logPath;
	_dumpPath = dumpPath;
	SetUnhandledExceptionFilter(HandleException);
}

LONG WINAPI exCrashHandler::HandleException(EXCEPTION_POINTERS* pExceptionInfo)
{
	std::wstring trace;
	if (GetStackTrace(pExceptionInfo, &trace))
	{

		if (g_NetworkCrashCallback)
			g_NetworkCrashCallback(pExceptionInfo, trace);

		ShowCrashDialog();
		//	ShowCrashTaskDialog();
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

bool exCrashHandler::GetStackTrace(EXCEPTION_POINTERS* pExceptionInfo, std::wstring* outTrace)
{
	std::wstring stackTrace;
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	SymInitialize(process, NULL, TRUE);    
	
	CONTEXT context = *pExceptionInfo->ContextRecord;
	STACKFRAME64 frame = {};
	DWORD machineType;

#if defined(_M_X64)
	machineType = IMAGE_FILE_MACHINE_AMD64;
	frame.AddrPC.Offset = context.Rip;
	frame.AddrFrame.Offset = context.Rsp;
	frame.AddrStack.Offset = context.Rsp;
#elif defined(_M_IX86)
#error Unsupported platform
	machineType = IMAGE_FILE_MACHINE_I386;
	frame.AddrPC.Offset = context.Eip;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrStack.Offset = context.Esp;
#else
#error Unsupported platform
#endif

	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;    

	
	std::wostringstream crashText;

	crashText << L"--- Crash Call Stack ---\n";

	for (int i = 0; i < 64; ++i) {
		if (!StackWalk64(machineType, process, thread, &frame, &context, NULL,
			SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}

		DWORD64 addr = frame.AddrPC.Offset;
		if (addr == 0) 
			break;

		/* get module */
		DWORD64 moduleBase = SymGetModuleBase64(process, addr);
		WCHAR moduleName[MAX_PATH] = L"<unknown>";
		if (moduleBase != 0) {
			HMODULE hModule = reinterpret_cast<HMODULE>(moduleBase);
			GetModuleFileNameW(hModule, moduleName, MAX_PATH);
		}

		/* begin log */
		crashText << L"[" << i << L"] 0x" << std::hex << addr;

		if (moduleBase != 0) {
			DWORD64 relativeOffset = addr - moduleBase;
			crashText << L" (" << moduleName << L" + 0x" << std::hex << relativeOffset << L")";
		}

#ifdef _DEBUG
		/* debug : function + file + line info */
		DWORD64 displacement = 0;
		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
		PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		if (SymFromAddr(process, addr, &displacement, symbol)) {
			crashText << L" - " << symbol->Name;
		}

		IMAGEHLP_LINE64 line = {};
		DWORD dwDisplacement = 0;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		if (SymGetLineFromAddr64(process, addr, &dwDisplacement, &line)) {
			crashText << L" [" << line.FileName << L":" << std::dec << line.LineNumber << L"]";
		}
#endif

		crashText << L"\n";
	}

#ifdef _DEBUG
	// write the captured text to a file
	std::wofstream logFile(_logPath);  // Open log file in append mode
	if (logFile.is_open()) {
		logFile << crashText.str();  // Write everything at once to the file
		logFile.close();  // Close the file after writing
	}
#endif

	*outTrace = crashText.str();  // Pass the stack trace to the caller

	return crashText.str().size() > 0;
}

bool exCrashHandler::GetMinidump(EXCEPTION_POINTERS* pExceptionInfo, std::wstring* pDump) 
{
	HANDLE hFile = CreateFileW(_dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) 
		return false;

	MINIDUMP_EXCEPTION_INFORMATION mdei;
	mdei.ThreadId = GetCurrentThreadId();
	mdei.ExceptionPointers = pExceptionInfo;
	mdei.ClientPointers = FALSE;

	MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		&mdei,
		nullptr,
		nullptr
	);

	return CloseHandle(hFile);
}

void exCrashHandler::ShowCrashDialog()
{

#ifdef _DEBUG

	int result = MessageBoxW(
		nullptr,
		L"StarCitizen - NightCity has unfortunately crashed.\n\nA crash log was created.You may view the log and provide it to support,\n\nWould you like to view the crash log?",
		L"NightCity Crash Handler",
		MB_ICONERROR | MB_YESNO
	);

	if (result != IDYES)
		return;

	ShellExecuteW(nullptr, L"open", _logPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

	return;

#endif

	MessageBoxW(
		nullptr,
		L"StarCitizen - NightCity has unfortunately crashed",
		L"NightCity Crash Handler",
		MB_ICONERROR | MB_YESNO
	);
}