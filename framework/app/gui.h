#pragma once

#include <windows.h>

struct ID3D11Device;

namespace gui
{
    inline bool g_menu_open = true;

    void initialize(ID3D11Device* device);
    void shutdown();
    void prepare_frame();
    void render(HWND hwnd);
    bool is_point_inside(float x, float y);
}
