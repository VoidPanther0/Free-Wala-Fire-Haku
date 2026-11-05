#include "core/device.h"
#include "core/window.h"
#include "app/gui.h"
#include "helpers/screenshot.h"

#include <windows.h>
#include <thread>
#include <chrono>

#include "theme/layout.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// ── SILK ESP Integration ──────────────────────────────────────────────────────
#include "esp/emu_connect.hpp"
#include "esp/esp_overlay.hpp"
#include "esp/Cheat/Data/Data.hpp"

static HMODULE g_dll_instance = nullptr;

static DWORD WINAPI ui_thread(LPVOID)
{
    // Allow the emulator process a short moment to finish loading
    Sleep(800);

    // ── Connect to emulator (BstkVMM hook + ADB root) ─────────────────────────
    // Note: Data::Work() runs synchronously inside esp_overlay::frame() on this
    // UI thread (exactly as designed in SILK) to prevent multithread data races.
    emu_connect::start();
    // ─────────────────────────────────────────────────────────────────────────

    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, app_wnd_proc, 0L, 0L,
        g_dll_instance ? g_dll_instance : GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"ImguiPortfolio14Shell", nullptr };
    ::RegisterClassExW(&wc);

    GfxShared shared = {};
    if (!gfx_create_shared(shared))
    {
        gfx_destroy_shared(shared);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    AppSurface surface = {};

    const int win_w = ::GetSystemMetrics(SM_CXSCREEN);
    const int win_h = ::GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = ::CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST,
        wc.lpszClassName, L"Sachin Dev", WS_POPUP,
        0, 0, win_w, win_h,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (hwnd)
    {
        ::SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&surface));
    }

    if (!hwnd || !gfx_create_surface(shared, surface, hwnd))
    {
        gfx_destroy_surface(surface);
        gfx_destroy_shared(shared);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(shared.device, shared.context);
    gui::initialize(shared.device);

    surface.imgui_win32_route   = true;
    surface.quit_app_on_destroy = false; // NEVER quit emulator process on UI destroy

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        // Toggle UI with INSERT key
        static bool insert_down = false;
        if (GetAsyncKeyState(VK_INSERT) & 0x8000)
        {
            if (!insert_down)
            {
                insert_down = true;
                gui::g_menu_open = !gui::g_menu_open;

                LONG_PTR ex = ::GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                if (gui::g_menu_open)
                {
                    // Menu is OPEN -> allow mouse clicks on menu panel
                    ex &= ~WS_EX_TRANSPARENT;
                }
                else
                {
                    // Menu is HIDDEN -> 100% click-through to Discord, Desktop, BlueStacks, etc.
                    ex |= WS_EX_TRANSPARENT;
                }
                ::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex);
                ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        else
        {
            insert_down = false;
        }

        unsigned rw = 0, rh = 0;
        if (app_surface_consume_resize(surface, rw, rh))
            gfx_resize_surface(shared, surface, rw, rh);

        // Render SILK ESP overlay attached to the emulator window
        esp_overlay::frame(g_dll_instance, shared.device, shared.context);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        app_update_io_for_hwnd(hwnd, io);
        screenshot::begin_frame();
        gui::prepare_frame();
        ImGui::NewFrame();

        gui::render(hwnd);

        ImGui::Render();
        gfx_render_surface(shared, surface, ImGui::GetDrawData());
    }

    gui::shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    gfx_destroy_surface(surface);
    gfx_destroy_shared(shared);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        g_dll_instance = hModule;
        ::DisableThreadLibraryCalls(hModule);
        ::CreateThread(nullptr, 0, ui_thread, nullptr, 0, nullptr);
    }
    return TRUE;
}
