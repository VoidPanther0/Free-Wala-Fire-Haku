#pragma once

#include "imgui.h"

#include <cmath>

namespace colors
{
    inline ImVec4 accent       = { 255.f / 255.f, 200.f / 255.f, 0.f / 255.f, 1.f }; // Gold Yellow
    inline ImVec4 accent_hover = { 255.f / 255.f, 215.f / 255.f, 40.f / 255.f, 1.f };
    inline ImVec4 accent_purple= { 220.f / 255.f, 160.f / 255.f, 0.f / 255.f, 1.f }; // Warm Orange Accent

    // Backdrop: matching Portfolio #1 dark layout
    inline constexpr ImVec4 backdrop_login = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f };
    inline constexpr ImVec4 backdrop_app   = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f };

    inline constexpr ImVec4 bg              = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f }; // c_colors::layout
    inline constexpr ImVec4 panel           = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f }; // c_colors::child
    inline constexpr ImVec4 topbar          = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f };
    inline constexpr ImVec4 widget          = { 33.f / 255.f, 33.f / 255.f, 39.f / 255.f, 1.f }; // c_colors::widget
    inline constexpr ImVec4 widget_hover    = { 44.f / 255.f, 44.f / 255.f, 52.f / 255.f, 1.f }; // c_colors::checkbox_hover
    inline constexpr ImVec4 surface         = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f };
    inline constexpr ImVec4 surface_hover   = { 33.f / 255.f, 33.f / 255.f, 39.f / 255.f, 1.f };
    inline constexpr ImVec4 control         = { 33.f / 255.f, 33.f / 255.f, 39.f / 255.f, 1.f };
    inline constexpr ImVec4 control_hover   = { 44.f / 255.f, 44.f / 255.f, 52.f / 255.f, 1.f };
    inline constexpr ImVec4 popup           = { 28.f / 255.f, 28.f / 255.f, 33.f / 255.f, 1.f }; // c_colors::popup
    inline constexpr ImVec4 module_box      = { 24.f / 255.f, 24.f / 255.f, 28.f / 255.f, 1.f };
    inline constexpr ImVec4 module_box_hover = { 33.f / 255.f, 33.f / 255.f, 39.f / 255.f, 1.f };
    inline constexpr ImVec4 text            = { 1.f, 1.f, 1.f, 1.f };
    inline constexpr ImVec4 text_muted      = { 111.f / 255.f, 111.f / 255.f, 135.f / 255.f, 1.f }; // c_colors::text2
    inline constexpr ImVec4 text_secondary  = { 151.f / 255.f, 151.f / 255.f, 179.f / 255.f, 1.f }; // c_colors::text3
    inline constexpr ImVec4 danger          = { 244.f / 255.f, 63.f / 255.f, 94.f / 255.f, 1.f };
    inline constexpr ImVec4 divider         = { 35.f / 255.f, 35.f / 255.f, 43.f / 255.f, 1.f }; // c_colors::seperator
    inline constexpr ImVec4 scroll_track     = { 20.f / 255.f, 20.f / 255.f, 23.f / 255.f, 1.f };
    inline constexpr ImVec4 scroll_thumb     = { 40.f / 255.f, 40.f / 255.f, 49.f / 255.f, 1.f }; // c_colors::popup_scroll_thumb
    inline constexpr ImVec4 dropdown_bg      = { 28.f / 255.f, 28.f / 255.f, 33.f / 255.f, 1.f };

    inline void sync_accent_hover()
    {
        accent_hover = {
            fminf(accent.x * 1.08f + 0.04f, 1.f),
            fminf(accent.y * 1.08f + 0.04f, 1.f),
            fminf(accent.z * 1.08f + 0.04f, 1.f),
            accent.w };
    }
}
