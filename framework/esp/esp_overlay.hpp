#pragma once
// Transparent click-through ESP overlay drawn over the emulator render window.
// Runs on the UI thread with its own ImGui context (shares the Silk D3D11 device).
// Also runs Data::Work() on this thread so the entity map is never read while being rebuilt.

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <Cheat/Options.hpp>
#include <Cheat/Data/Data.hpp>
#include <Cheat/FreeFireSDK/Context.hpp>
#include <Cheat/Features/Visuals/PlayerEsp.hpp>
#include <FrameWork/Utilities/FindRender/Render.hpp>
#include <FrameWork/Render/Assets.hpp>

namespace esp_overlay {

    using Microsoft::WRL::ComPtr;

    inline HWND                         g_window  = nullptr;
    inline HWND                         g_target  = nullptr;
    inline ImGuiContext*                g_ctx     = nullptr;
    inline ComPtr<IDXGISwapChain>       g_swap;
    inline ComPtr<ID3D11RenderTargetView> g_rtv;
    inline int                          g_w = 0, g_h = 0;
    inline ULONGLONG                    g_last_find = 0;
    inline bool                         g_affinity_on = false;
    inline const wchar_t*               g_class = L"SilkEspOverlay";

    inline LRESULT CALLBACK wnd_proc( HWND h, UINT m, WPARAM w, LPARAM l ) { return ::DefWindowProcW( h, m, w, l ); }

    inline bool target_rect( RECT& r )
    {
        if ( !g_target || !::IsWindow( g_target ) ) return false;
        if ( !::GetClientRect( g_target, &r ) ) return false;
        ::MapWindowPoints( g_target, nullptr, reinterpret_cast< LPPOINT >( &r ), 2 );
        return ( r.right - r.left ) > 0 && ( r.bottom - r.top ) > 0;
    }

    inline void make_rtv( )
    {
        ComPtr<ID3D11Texture2D> back;
        if ( SUCCEEDED( g_swap->GetBuffer( 0, IID_PPV_ARGS( &back ) ) ) )
        {
            ID3D11Device* dev = nullptr;
            g_swap->GetDevice( IID_PPV_ARGS( &dev ) );
            if ( dev ) { dev->CreateRenderTargetView( back.Get( ), nullptr, &g_rtv ); dev->Release( ); }
        }
    }

    inline bool create( HINSTANCE inst, ID3D11Device* device, ID3D11DeviceContext* dctx, const RECT& r )
    {
        WNDCLASSEXW wc{ sizeof( wc ) };
        wc.lpfnWndProc = wnd_proc;
        wc.hInstance = inst;
        wc.lpszClassName = g_class;
        wc.hCursor = ::LoadCursor( nullptr, IDC_ARROW );
        ::RegisterClassExW( &wc );

        g_w = r.right - r.left;
        g_h = r.bottom - r.top;
        g_window = ::CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
            g_class, L"", WS_POPUP | WS_VISIBLE, r.left, r.top, g_w, g_h, nullptr, nullptr, inst, nullptr );
        if ( !g_window ) return false;

        const MARGINS m{ -1, -1, -1, -1 };
        ::DwmExtendFrameIntoClientArea( g_window, &m );
        ::SetLayeredWindowAttributes( g_window, RGB( 0, 0, 0 ), 255, LWA_ALPHA );

        ComPtr<IDXGIDevice> dxgi_dev;
        ComPtr<IDXGIAdapter> adapter;
        ComPtr<IDXGIFactory> factory;
        if ( FAILED( device->QueryInterface( IID_PPV_ARGS( &dxgi_dev ) ) ) ||
             FAILED( dxgi_dev->GetAdapter( &adapter ) ) ||
             FAILED( adapter->GetParent( IID_PPV_ARGS( &factory ) ) ) )
            return false;

        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 2;
        sd.OutputWindow = g_window;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        if ( FAILED( factory->CreateSwapChain( device, &sd, &g_swap ) ) ) return false;
        make_rtv( );

        ImGuiContext* previous = ImGui::GetCurrentContext( );
        g_ctx = ImGui::CreateContext( );
        ImGui::SetCurrentContext( g_ctx );
        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        ImFont* bold = io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\segoeuib.ttf", 15.f );
        ImFont* semi = io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\seguisb.ttf", 14.f );
        if ( !bold ) bold = io.Fonts->AddFontDefault( );
        if ( !semi ) semi = bold;
        Assets::InterExtraBold = bold;
        Assets::InterSemiBold = semi;
        Assets::icomoon = bold;
        Assets::lexend_bold = bold;
        Assets::lexend_regular = semi;

        const bool ok = ImGui_ImplWin32_Init( g_window ) && ImGui_ImplDX11_Init( device, dctx );
        ImGui::SetCurrentContext( previous );
        return ok;
    }

    inline void resize( const RECT& r )
    {
        const int w = r.right - r.left, h = r.bottom - r.top;
        ::MoveWindow( g_window, r.left, r.top, w, h, FALSE );
        if ( w != g_w || h != g_h )
        {
            g_rtv.Reset( );
            g_swap->ResizeBuffers( 0, w, h, DXGI_FORMAT_UNKNOWN, 0 );
            make_rtv( );
            g_w = w; g_h = h;
        }
    }

    // Call once per loop iteration, before the Silk UI frame.
    inline void frame( HINSTANCE inst, ID3D11Device* device, ID3D11DeviceContext* dctx )
    {
        // Don't touch memory or render until emu_connect finishes hooking BstkVMM & ADB
        if ( !Context::IsReady )
            return;

        // Locate the emulator render window (retry once per second).
        if ( ( !g_target || !::IsWindow( g_target ) ) && ::GetTickCount64( ) - g_last_find > 1000 )
        {
            g_last_find = ::GetTickCount64( );
            g_target = FrameWork::FindeRender::FindRenderWindow( );
        }

        RECT r{};
        const bool have_target = target_rect( r );
        if ( have_target )
        {
            Context::WindowWidth  = r.right - r.left;
            Context::WindowHeight = r.bottom - r.top;
        }
        else
        {
            if ( Context::WindowWidth  <= 0 ) Context::WindowWidth  = ::GetSystemMetrics( SM_CXSCREEN );
            if ( Context::WindowHeight <= 0 ) Context::WindowHeight = ::GetSystemMetrics( SM_CYSCREEN );
        }

        try {
            FrameWork::Data::Work( );
        } catch ( ... ) {}

        if ( !have_target ) { if ( g_window ) ::ShowWindow( g_window, SW_HIDE ); return; }

        if ( !g_window )
        {
            if ( !create( inst, device, dctx, r ) ) return;
        }
        else
        {
            if ( !::IsWindowVisible( g_window ) ) ::ShowWindow( g_window, SW_SHOWNOACTIVATE );
            resize( r );
        }

        if ( g_Options.Visuals.HideFromCapture != g_affinity_on )
        {
            g_affinity_on = g_Options.Visuals.HideFromCapture;
            ::SetWindowDisplayAffinity( g_window, g_affinity_on ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE );
        }

        ImGuiContext* previous = ImGui::GetCurrentContext( );
        ImGui::SetCurrentContext( g_ctx );

        ImGui_ImplDX11_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );
        {
            if ( g_Options.Misc.Screen.ShowAimbotFov )
            {
                const auto& c = g_Options.Misc.Screen.AimbotFovColor;
                ImGui::GetBackgroundDrawList( )->AddCircle(
                    ImVec2( ImGui::GetIO( ).DisplaySize.x / 2, ImGui::GetIO( ).DisplaySize.y / 2 ),
                    static_cast< float >( g_Options.LegitBot.AimBot.FOVRange ), ImColor( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ] ), 360 );
            }
            if ( g_Options.Visuals.ESP.Players.Enabled )
            {
                try {
                    Cheat::ESP::Players( );
                } catch ( ... ) {}
            }
        }
        ImGui::EndFrame( );
        ImGui::Render( );

        const float clear[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
        ID3D11RenderTargetView* rtv = g_rtv.Get( );
        if ( !rtv )
        {
            ImGui::SetCurrentContext( previous );
            return;
        }
        dctx->OMSetRenderTargets( 1, &rtv, nullptr );
        dctx->ClearRenderTargetView( rtv, clear );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
        g_swap->Present( 0, 0 );

        ImGui::SetCurrentContext( previous );
    }

} // namespace esp_overlay
