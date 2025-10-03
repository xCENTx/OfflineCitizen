// Precompiled header file for the project.
// 
#define NC_DEV 1
#define _USE_MATH_DEFINES 1
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1

#include <Windows.h>
#include <assert.h>
#include <shellapi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <mutex>
#include <memory>
#include <algorithm>
#include <functional>
#include <dwmapi.h>
#include <shlobj.h>

/* */
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

/* */
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

/* */
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

/* */
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_DEBUG_TOOLS 
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

/* */
#include <MinHook.h>

/* */
#include <detours.h>
#pragma comment(lib, "external/detours.lib")

/* */
#include <xorstr.hpp>	//	encrypted strings
#define __ xorstr_

/**/
#include <json.hpp>