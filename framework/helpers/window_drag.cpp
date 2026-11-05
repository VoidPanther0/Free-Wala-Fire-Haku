#include "window_drag.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <windows.h>

namespace window_drag
{
    namespace
    {
        struct drag_state
        {
            bool    active       = false;
            POINT   cursor_start = {};
            ImVec2  pos_start    = {};
            ImVec2  pos_target   = {};
        };

        drag_state g_panel_drag;
    }

    void handle_panel(ImVec2& panel_pos, const ImRect& drag_rect, bool block_drag)
    {
        if (block_drag)
        {
            g_panel_drag.active = false;
            return;
        }

        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const bool in_drag_rect = drag_rect.Contains(mouse);

        ImGuiWindow* hovered_window = GImGui ? GImGui->HoveredWindow : nullptr;
        const bool over_child = hovered_window && (hovered_window->Flags & ImGuiWindowFlags_ChildWindow);
        const bool over_popup = hovered_window
            && (hovered_window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip));

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_drag_rect
            && !ImGui::IsAnyItemActive() && !over_child && !over_popup)
        {
            g_panel_drag.active = true;
            ::GetCursorPos(&g_panel_drag.cursor_start);
            g_panel_drag.pos_start  = panel_pos;
            g_panel_drag.pos_target = panel_pos;
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (g_panel_drag.active)
            {
                panel_pos = g_panel_drag.pos_target;
                g_panel_drag.active = false;
            }
        }

        if (!g_panel_drag.active)
            return;

        POINT cur = {};
        ::GetCursorPos(&cur);
        g_panel_drag.pos_target.x = g_panel_drag.pos_start.x + (float)(cur.x - g_panel_drag.cursor_start.x);
        g_panel_drag.pos_target.y = g_panel_drag.pos_start.y + (float)(cur.y - g_panel_drag.cursor_start.y);

        // Exponential smoothing for fluid 60/120fps motion
        const float dt = ImGui::GetIO().DeltaTime;
        const float lerp_k = ImClamp(dt * 35.f, 0.f, 1.f);
        panel_pos.x += (g_panel_drag.pos_target.x - panel_pos.x) * lerp_k;
        panel_pos.y += (g_panel_drag.pos_target.y - panel_pos.y) * lerp_k;
    }

    void handle(HWND hwnd, const ImRect& drag_rect, bool block_drag)
    {
        if (!hwnd)
            return;

        static bool active = false;
        static POINT mouse_start = {};
        static POINT win_start   = {};

        if (block_drag)
        {
            active = false;
            return;
        }

        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const bool in_drag_rect = drag_rect.Contains(mouse);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_drag_rect && !ImGui::IsAnyItemHovered())
        {
            active = true;
            ::GetCursorPos(&mouse_start);
            RECT rc = {};
            ::GetWindowRect(hwnd, &rc);
            win_start.x = rc.left;
            win_start.y = rc.top;
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            active = false;

        if (!active)
            return;

        POINT cur = {};
        ::GetCursorPos(&cur);
        ::SetWindowPos(hwnd, nullptr,
            win_start.x + (cur.x - mouse_start.x),
            win_start.y + (cur.y - mouse_start.y),
            0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}
