#pragma once

// Windows
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <TlHelp32.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

// Utilities
#include <FrameWork/Utilities/Misc/Misc.hpp>
#include <FrameWork/Utilities/Memory/Memory.hpp>
#include <FrameWork/Utilities/FindRender/Render.hpp>

// ImGui (Silk Kernel vendor copy)
#include <imgui.h>

#define XorStr(str) (str)
#define SafeCall(str) (str)
