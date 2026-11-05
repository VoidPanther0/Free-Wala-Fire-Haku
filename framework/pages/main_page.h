#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace main_page
{
    struct state
    {
        bool  exit_pressed = false;
        int   active_nav = 0;    // selected sidebar tab

        // Scroll / tab-switch animation
        float content_scroll = 0.f;        // current (animated) vertical scroll offset
        float content_scroll_target = 0.f; // where the scroll is easing toward
        int   prev_nav = 0;                // last tab, to detect tab switches
        float tab_anim = 1.f;              // 0..1 fade/slide-in animation for the active tab

        // Which slide-out popup is open (0 = none). Indices match the cog popup_idx
        // passed to the settings rows and the switch in draw_app_content():
        //   1 Aim Key/Mode  2 Smoothing  3 Aimbone  4 Box Color  5 Skeleton Color
        //   6 Trigger Key/Mode  7 Box Type  8 Name Color  9 Health Color  10 Tracer Color  11 Accent Color
        int   open_popup = 0;

        // --- Aimbot tab ---
        bool  aim_active = true;
        float aim_fov = 90.f;
        float aim_smooth = 12.f;
        int   aim_bone = 0;             // 0 = Head, 1 = Neck, 2 = Chest, 3 = Pelvis
        bool  aim_visible_only = true;
        bool  aim_sticky = false;
        float aim_delay = 50.0f;        // also used for trigger delay
        int   aim_keybind = 0x02;       // VK_RBUTTON
        int   aim_mode = 0;             // 0 = Hold, 1 = Toggle
        int   smooth_mode = 0;          // 0 = Gradual, 1 = Decelerate, 2 = Static
        bool  binding_key = false;      // listening for a new aim keybind

        // Recoil control / misc
        bool  combat_rcs = true;
        float combat_rcs_h = 50.f;
        float combat_rcs_v = 80.f;
        bool  combat_bunnyhop = false;

        // Triggerbot
        bool  trigger_active = false;
        int   trigger_keybind = 0x05;   // VK_XBUTTON1
        int   trigger_mode = 0;         // 0 = Hold, 1 = Toggle
        bool  binding_trigger_key = false;

        // --- Visuals tab ---
        bool  esp_active = false;
        bool  esp_box = true;
        bool  esp_fill_box = false;
        int   esp_box_type = 0;         // 0 = Corner, 1 = Full, 2 = 3D
        bool  esp_skeleton = false;
        bool  esp_head_circle = false;
        bool  esp_tracer = false;
        bool  esp_snaplines = false;
        int   esp_line_anchor = 2;      // 0 = Top, 1 = Center, 2 = Bottom
        bool  esp_arrows = true;
        bool  esp_radar = false;
        bool  esp_names = true;
        bool  esp_health = true;
        bool  esp_distance = true;
        bool  esp_weapon = false;
        bool  esp_ignore_bots = false;
        float esp_render_range = 300.f;
        float esp_glow = 4.5f;
        float esp_thickness = 1.5f;
        float esp_fill_opacity = 0.25f;

        bool  visual_fov_circle = false;
        bool  visual_crosshair = false;
        bool  visual_watermark = true;
        bool  visual_fps = true;

        // ESP colors (edited by the color-picker popups)
        ImVec4 esp_color_box = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
        ImVec4 esp_color_skeleton = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 esp_color_name = ImVec4(0.59f, 0.59f, 0.70f, 1.0f);
        ImVec4 esp_color_health = ImVec4(0.30f, 1.0f, 0.40f, 1.0f);
        ImVec4 esp_color_tracer = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);

        // --- Loader tab ---
        bool  hook_detours = true;
        bool  thread_hijack = false;
        bool  bypass_active = true;

        // --- Config tab ---
        int   active_config = 0;        // 0 = Default, 1 = Legit, 2 = Rage
        float ui_opacity = 0.95f;
        float ui_scale = 1.0f;          // global interface scale (drives ui_scale::user_scale)

        // Color-picker working HSV state (synced from the active color while a picker is open)
        float pick_h = 0.f, pick_s = 0.f, pick_v = 0.f, pick_a = 1.f;
        int   pick_init_for = 0;        // popup id the HSV state is initialized for (0 = none)
    };

    void draw(const ImRect& panel, state& s, bool& blocks_window_drag);
}
