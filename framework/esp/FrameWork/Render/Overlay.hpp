#pragma once
// Stub overlay — Silk Kernel provides its own window/rendering via silkui.
// Data.cpp includes this for transitivity; actual window handle and dimensions
// are populated through Context::WindowWidth / Context::WindowHeight.
#include <Windows.h>
#include <d3d11.h>
#include <functional>
#include <string>
#include <imgui.h>

// Forward declarations matching the Grej Labs interface so existing feature
// code compiles without modification.
namespace FrameWork {
    namespace Overlay {
        inline HWND g_OverlayHwnd  = nullptr;
        inline HWND g_TargetHwnd   = nullptr;

        inline void   Setup(HWND target)  { g_TargetHwnd = target; }
        inline void   Initialize()        {}
        inline void   ShutDown()          {}
        inline void   UpdateWindowPos()   {}
        inline bool   IsSettuped()        { return g_TargetHwnd != nullptr; }
        inline bool   IsInitialized()     { return true; }
        inline HWND   GetOverlayWindow()  { return g_OverlayHwnd; }
        inline HWND   GetTargetWindow()   { return g_TargetHwnd; }
        inline void   dxRefresh()         {}
        inline void   dxCreateRenderTarget()   {}
        inline void   dxCleanupRenderTarget()  {}
        inline ID3D11Device*            dxGetDevice()        { return nullptr; }
        inline ID3D11DeviceContext*     dxGetDeviceContext() { return nullptr; }
        inline IDXGISwapChain*          dxGetSwapChain()     { return nullptr; }
        inline ID3D11RenderTargetView*  dxGetRenderTarget()  { return nullptr; }
        inline ImTextureID LoadTexture(const std::string&, int* = nullptr, int* = nullptr) { return 0; }
        inline void SetupWindowProcHook(std::function<void(HWND, UINT, WPARAM, LPARAM)>) {}
    }
}
