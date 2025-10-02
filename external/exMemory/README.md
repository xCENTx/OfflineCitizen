# exMemory
`exMemory` is a C++ utility class designed for advanced memory manipulation tasks on external processes. It supports reading, writing, and scanning process memory, as well as managing process and module information. The class offers both static and instance-based operations for maximum flexibility.

## Features
- Attach and detach from multiple processes.
- Read and write to process memory (including pointer chains , strings & patterns).
- Patch memory with custom bytes
- enumerate modules and sections in a process.
- Pattern scanning with optional instruction-based offsets.
- Static methods for direct operations without maintaining an instance.

## Potential Future Additions
- Error Handling 
- Various DLL Injection Methods
- Code Cave Concepts
- Module Dumping

---

## Getting Started

### Prerequisites

- Windows operating system
- A modern C++ compiler (e.g., MSVC)
- `Windows.h` header for Windows API

### Installation

1. Clone the repository:
```bash
git clone https://github.com/NightFyre/exMemory.git
```
2. Include the exMemory.h file in your project:
```cpp
#include "exMemory.hpp"
```

## Usage
You can use `exMemory` in two ways: instance-based or through static methods. 
The following examples each demonstrate attaching to a process and reading the IMAGE_DOS_HEADER section before detaching from the process.

**Instance-Based Example**
```cpp
#include "exMemory.hpp"

int main() 
{
    exMemory mem = exMemory("pcsx2-qt.exe", PROCESS_ALL_ACCESS);
    const auto& pInfo = mem.GetProcessInfo();
    if (pInfo.bAttached)
    {
        const auto& value = mem.Read<IMAGE_DOS_HEADER>(pInfo.dwModuleBase);

        memory.Detach();
    }
    
    return 0;
}
```

**Static Method Example**
```cpp
#include "exMemory.hpp"

int main() 
{
    procInfo_t proc;
    if (exMemory::AttachEx("pcsx2-qt.exe", &proc, PROCESS_ALL_ACCESS))  //  attach to named process with desired access
    {
        const auto& value = exMemory::ReadEx<IMAGE_DOS_HEADER>(pInfo.hProc, pInfo.dwModuleBase);    //  read the dos header section
        
        exMemory::DetachEx(proc);
    }
    
    return 0;
}
```

## Key Methods

**Instance Methods**
- Attach/Detach
```cpp

//  Constructor
exMemory mem = exMemory("pcsx2-qt.exe");    //  attaches to pcsx2 process with default PROCESS_ALL_ACCESS rights , process information is accessible via 'mem.GetProcessInfo()'

//  custom , can also be used to overwrite existing attached process
bool Attach(const std::string& name, const DWORD& dwAccess = PROCESS_ALL_ACCESS);   //  attaches to named process with desired access
bool Detach();  //  detaches from the attached process
```

- Read/Write Memory
```cpp
// 
bool ReadMemory(const i64_t& addr, void* buffer, const DWORD& szRead);
bool ReadString(const i64_t& addr, std::string& string, const DWORD& szString = MAX_PATH);
bool WriteMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite);
bool PatchMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite);

//  template
template<typename T>
T Read(const i64_t& addr);

template<typename T>
bool Write(const i64_t& addr, T value);
```

- Pointer Chains & Pattern Scanning
```cpp
i64_t ReadPointerChain(const i64_t& addr, std::vector<unsigned int>& offsets, i64_t* lpResult);
i64_t FindPattern(const std::string& signature, i64_t* result, int padding = 0, bool isRelative = false, EASM instruction = EASM::ASM_NULL);
```

**Static Methods**
- Attach/Detach
```cpp
static bool AttachEx(const std::string& name, procInfo_t* lpProcess, const DWORD& dwDesiredAccess);
static bool DetachEx(procInfo_t& pInfo);
```

- Direct Memory Operations
```cpp
//  methods
static bool ReadMemoryEx(const HANDLE& hProc, const i64_t& addr, void* buffer, size_t szRead);
static bool ReadStringEx(const HANDLE& hProc, const i64_t& addr, const size_t& szString, std::string* lpResult);
static bool WriteMemoryEx(const HANDLE& hProc, const i64_t& addr, LPVOID buffer, DWORD szWrite);
static bool PatchMemoryEx(const HANDLE& hProc, const i64_t& addr, const void* buffer, const DWORD& szWrite);

//  templates
template<typename T>
T ReadEx(const HANDLE& hProc, const i64_t& addr);

template<typename T>
bool WriteEx(const HANDLE& hProc, const i64_t& addr, T value);
```

- Pointer Chains & Pattern Scanning
```cpp
static bool ReadPointerChainEx(const HANDLE& hProc, const i64_t& addr, const std::vector<unsigned int>& offsets, i64_t* lpResult);
static bool FindPatternEx(const HANDLE& hProc, const std::string& moduleName, const std::string& signature, i64_t* lpResult, int padding, bool isRelative, EASM instruction);
static bool FindPatternEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& signature, i64_t* lpResult, int padding, bool isRelative, EASM instruction);
```

## Advanced Features & Examples
The following examples all make use of static methods for readability.

**Pointer Chains**
- Resolve multi-level pointer chains
```cpp
procInfo_t proc;
if (exMemory::AttachEx("pcsx2-qt.exe", &proc, PROCESS_ALL_ACCESS))
{
	i64_t result;
	std::vector<unsigned int> offsets = { 0x1C , 0x30 };
	exMemory::ReadPointerChainEx(proc.hProc, 0x44D548, offsets, &result);

	//	...

	exMemory::DetachEx(proc);
}
```

**Pattern Scanning**
- Find memory patterns with custom instructions and offsets
```cpp
i64_t address = memory.FindPattern("90 90 ?? ?? E8 ?? ?? ?? ??", nullptr, 0, false);
```

**Section Walking**
- Get the base address of a section within a loaded module in an attached processes
```cpp
procInfo_t proc;
if (exMemory::AttachEx("pcsx2-qt.exe", &proc, PROCESS_ALL_ACCESS))
{
	size_t szSection = 0;
	i64_t SectionHeader = 0;
	exMemory::GetSectionHeaderAddressEx(pInfo.hProc, "pcsx2-qt.exe", ESECTIONHEADERS::SECTION_TEXT, &SectionHeader, &szSection);
	
    //	...
	
    exMemory::DetachEx(proc);
}

```

**Resolve Export Table Entries**
- Retrieve the address of an exported function
```cpp
procInfo_t proc;
if (exMemory::AttachEx("pcsx2-qt.exe", &proc, PROCESS_ALL_ACCESS))
{
    i64_t EEmem = 0;
	exMemory::GetProcAddressEx(proc.hProc, "pcsx2-qt.exe", "EEmem", &EEmem);
	
    //	...

	exMemory::DetachEx(proc);
}
```

## Enums and Structures
**Enums**
- EASM: Assembly instruction types (e.g., ASM_MOV, ASM_CALL).
- ESECTIONHEADERS: Section headers in a PE file (e.g., .text, .data).
- EINJECTION: Injection types (e.g., LOADLIBRARY, MANUAL).

**Structures**
- procInfo_t: Represents process information (ID, handle, base address, etc.).
- modInfo_t: Represents module information (base address, name, etc.).

## Performance Considerations
- Instance methods are optimized for scenarios where a process is frequently accessed.
- Static methods are ideal for one-off operations without maintaining a persistent state.
- Try to avoid repeated calls to slow methods such as `GetActiveProcessesEx` and instead prefer to cache and update results.

## Resources
- [GuidedHacking](https://guidedhacking.com)
- [UnknownCheats](https://unknowncheats.me/)
- [WinApi Process Enumeration](https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes)
- [WinApi About Memory Management](https://learn.microsoft.com/en-us/windows/win32/memory/about-memory-management)
- [WinApi Virtual Memory Functions](https://learn.microsoft.com/en-us/windows/win32/Memory/virtual-memory-functions)
- [WinApi Memory Management Reference](https://learn.microsoft.com/en-us/windows/win32/memory/memory-management-reference)