#include "gui.h"

#include "helpers/fonts.h"
#include "helpers/texture_cache.h"
#include "helpers/window_drag.h"
#include "pages/login_page.h"
#include "pages/main_page.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <windows.h>
#include <vector>

namespace
{
    main_page::state  g_main;
    login_page::state g_login;

    ImGuiStyle g_base_style = {};
    bool g_base_style_saved = false;

    // Smooth UI scaling state
    float g_smooth_scale = 1.0f;

    // Track active panel boundaries for WM_NCHITTEST pass-through
    ImRect g_active_panel_rect = {};
    ImRect g_active_popup_rect = {};
    bool   g_has_open_popup    = false;

    // Page transition animation (0 = login, 1 = main dashboard)
    float g_view_transition = 0.f;

    void apply_style()
    {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding   = 0.f;
        s.WindowBorderSize = 0.f;
        s.WindowPadding    = { 0.f, 0.f };
        s.AntiAliasedLines = true;
        s.AntiAliasedFill  = true;
        s.ChildBorderSize  = 0.f;
        s.FrameBorderSize  = 0.f;
        s.ScrollbarSize    = 0.f;

        ImVec4* c = s.Colors;
        c[ImGuiCol_WindowBg] = { 0.f, 0.f, 0.f, 0.f };
        c[ImGuiCol_Text]     = colors::text;

        if (!g_base_style_saved)
        {
            g_base_style = s;
            g_base_style_saved = true;
        }
    }
}

namespace gui
{
    void initialize(ID3D11Device* device)
    {
        colors::sync_accent_hover();
        fonts::init();
        apply_style();
        texture_cache::init(device);

        // Pre-fill demo credentials so user can log in with 1 click or type their own
        strncpy_s(g_login.username, "111", sizeof(g_login.username));
        strncpy_s(g_login.password, "111", sizeof(g_login.password));
    }

    void shutdown()
    {
        fonts::shutdown();
        texture_cache::shutdown();
    }

    void prepare_frame()
    {
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float dt = ImGui::GetIO().DeltaTime;

        // Smoothly interpolate user_scale for silky-smooth resizing
        g_smooth_scale += (ui_scale::user_scale - g_smooth_scale) * ImMin(1.f, dt * 16.f);

        ui_scale::factor = (layout::panel_w / layout::panel_design) * g_smooth_scale;
        fonts::sync_ui_scale(ui_scale::factor);

        ImGui::GetStyle() = g_base_style;
        ImGui::GetStyle().ScaleAllSizes(ui_scale::factor);
    }

    bool is_point_inside(float x, float y)
    {
        if (!g_menu_open)
            return false;

        const ImVec2 pt(x, y);
        if (g_active_panel_rect.Contains(pt))
            return true;
        if (g_has_open_popup && g_active_popup_rect.Contains(pt))
            return true;
        return false;
    }

    void render(HWND hwnd)
    {
        if (!g_menu_open)
            return;

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float dt = ImGui::GetIO().DeltaTime;

        // Golden modern aspect ratio: slightly narrower width, taller height
        const float w = ui_scale::s(layout::panel_w); // 440
        const float h = ui_scale::s(layout::panel_h); // 560

        // Keep panel position and allow smooth dragging
        static ImVec2 panel_pos = { -1.f, -1.f };
        if (panel_pos.x < 0.f || panel_pos.y < 0.f)
            panel_pos = { (display.x - w) * 0.5f, (display.y - h) * 0.5f };

        // Keep inside screen bounds
        if (panel_pos.x < 0.f) panel_pos.x = 0.f;
        if (panel_pos.y < 0.f) panel_pos.y = 0.f;
        if (panel_pos.x + w > display.x && display.x > w) panel_pos.x = display.x - w;
        if (panel_pos.y + h > display.y && display.y > h) panel_pos.y = display.y - h;

        const ImRect panel_bb(panel_pos, panel_pos + ImVec2(w, h));
        const float panel_round = ui_scale::s(layout::shell_round);

        // Update active panel bounds for pass-through hit testing
        const ImVec2 bp_min = panel_bb.Min + ImVec2(ui_scale::s(28.f), ui_scale::s(40.f));
        const ImVec2 bp_max = panel_bb.Min + ImVec2(ui_scale::s(layout::panel_w - 28.f), ui_scale::s(layout::panel_h - 40.f));
        g_active_panel_rect = ImRect(bp_min, bp_max);

        // Check if a popup is active in main_page
        g_has_open_popup = (g_main.open_popup != 0);
        if (g_has_open_popup)
        {
            // Popups 1..11 pop out to the right of the panel
            const float pop_w = ui_scale::s(220.f);
            g_active_popup_rect = ImRect(bp_max.x, bp_min.y, bp_max.x + pop_w + ui_scale::s(20.f), bp_max.y);
        }

        ImGui::SetNextWindowPos({ 0.f, 0.f }, ImGuiCond_Always);
        ImGui::SetNextWindowSize(display, ImGuiCond_Always);

        ImGui::Begin(layout::main_window_id, nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Outer smooth subtle dark shadow for floating window depth
        for (int i = 4; i >= 1; --i)
        {
            const float shadow_off = (float)i * ui_scale::s(2.5f);
            dl->AddRectFilled(bp_min - ImVec2(shadow_off, shadow_off),
                              bp_max + ImVec2(shadow_off, shadow_off),
                              ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.04f * (5 - i))),
                              panel_round + shadow_off);
        }

        // Draw the dark sleek GUI container window
        dl->AddRectFilled(bp_min, bp_max, ImGui::GetColorU32(colors::bg), panel_round);
        dl->AddRect(bp_min, bp_max, ImGui::GetColorU32(colors::divider), panel_round, 0, ui_scale::s(1.0f));

        // Smooth View Transition between Login and Main Dashboard
        const float target_transition = g_login.is_logged_in ? 1.0f : 0.0f;
        g_view_transition += (target_transition - g_view_transition) * ImMin(1.f, dt * 10.f);

        bool blocks_drag = false;

        // Render Login View if not fully transitioned to main
        if (g_view_transition < 0.999f)
        {
            g_login.page_alpha = 1.0f - g_view_transition;
            login_page::draw(panel_bb, g_login, blocks_drag);
        }

        // Render Main Dashboard View when transition begins
        if (g_view_transition > 0.001f)
        {
            main_page::draw(panel_bb, g_main, blocks_drag);
        }

        // Allow smooth dragging from the top header area of the panel
        const ImRect top_header(bp_min, ImVec2(bp_max.x - ui_scale::s(60.f), bp_min.y + ui_scale::s(48.f)));
        window_drag::handle_panel(panel_pos, top_header, blocks_drag);

        if (g_main.exit_pressed || g_login.exit_pressed)
            ::PostQuitMessage(0);

        (void)blocks_drag;
        (void)hwnd;
        ImGui::End();
    }
}
