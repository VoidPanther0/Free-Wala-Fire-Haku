#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace login_page
{
    struct state
    {
        char  username[64] = "111";
        char  password[64] = "111";
        bool  remember_me = true;
        bool  auto_login = false;
        bool  show_password = false;
        bool  is_logged_in = false;
        bool  exit_pressed = false;
        bool  is_loading = false;
        float loading_timer = 0.f;
        float page_alpha = 1.0f;
        float error_timer = 0.f;
        char  status_text[64] = "";
    };

    void draw(const ImRect& panel, state& st, bool& blocks_window_drag);
}
