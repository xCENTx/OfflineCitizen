#pragma once

class memory
{
public:
	enum class EMEM_ASM_TYPE : int
	{
		NONE = 0,
		MOV = 1,
		CALL = 2,
		LEA = 3,
		CMP = 4,
	};

public:
	explicit memory() = default;
	~memory() = default;

public:
	//	initializes the memory class
	bool init();

	//	shuts down the memory class
	bool shutdown();

	//	updates the memory class
	void update();

public:
	//	obtains the main process window handle
	static HWND GetMainProcWndw();

	//	obtains the process window handle by title
	static HWND GetProcWndw(const std::string& wndwTitle);

public:
	//	template<typename T>
	//	static T GetAddr(const DWORD& dwOffset) { return reinterpret_cast<T>(__int64(GetModuleHandle(0)) + dwOffset); }

	//	template<typename T>
	//	static T GetAddr(const DWORD& dwOffset, const std::string& moduleName) { return reinterpret_cast<T>(__int64(GetModuleHandle(moduleName.c_str())) + dwOffset); }

	template<typename T>
	static T CleanPointer(const unsigned __int64& ptr) { return reinterpret_cast<T>(ptr & 0xFFFFFFFF); }

	template<typename T>
	T Read(const unsigned __int64& addr) { return *reinterpret_cast<T*>(addr); }

	template<typename T>
	static bool Write(const unsigned __int64& addr, const T& value) { (T*)addr = value; }

	static bool Patch(const unsigned __int64& addr, const std::vector<unsigned char>& patch);

	static uintptr_t PatternScan(const uintptr_t& dwModule, const char* Signature, const bool& bRelative, const int& spacing, const EMEM_ASM_TYPE& iType = EMEM_ASM_TYPE::NONE);

public:
	struct hooker
	{
		static bool bInitialized;
		static bool Create(void* pTarget, void** Original, void* Function);
		static bool Disable(void* pTarget, const bool& bDisableAll = false);
		static bool Remove(void* pTarget);
	};

private:
	DWORD dwPID;	//	process id
	__int64 dwModule;	//	base address of the main module
	HWND scMenu;	//	process window handle
};

class exCrashHandler
{
public:
	static void SetCrashHandler(const wchar_t* logPath, const wchar_t* dumpPath);

private:
	static LONG WINAPI HandleException(EXCEPTION_POINTERS* pExceptionInfo);
	static bool GetMinidump(EXCEPTION_POINTERS* pExceptionInfo, std::wstring* outDump);
	static bool GetStackTrace(EXCEPTION_POINTERS* pExceptionInfo, std::wstring* outTrace);
	static void WriteMinidump(std::wstring dump);
	static void WriteStackTrace(std::wstring trace);
	static void ShowCrashTaskDialog();
	static void ShowCrashTaskDialogWithStackTrace(const std::wstring& stackText);
	static void ShowCrashDialog();


private:
	static inline std::wstring _logPath;
	static inline std::wstring _dumpPath;
};