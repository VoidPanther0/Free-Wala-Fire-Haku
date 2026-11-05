#include "main_page.h"

#include "helpers/fonts.h"
#include "helpers/icons.h"
#include "theme/colors.h"
#include "theme/layout.h"
#include "theme/ui_scale.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>
#include <unordered_map>

// ── SILK ESP Integration ──────────────────────────────────────────────────────
// These headers bring in Context (entity list + ViewMatrix), W2S projection,
// and the SILK PlayerEsp drawing routines.  The g_esp_state pointer lets us
// forward the UI toggles into the draw call every frame.
#include "esp/Cheat/FreeFireSDK/Context.hpp"
#include "esp/Cheat/Options.hpp"
#include "esp/Cheat/Features/Visuals/PlayerEsp.hpp"
#include "esp/FrameWork/Math/W2S.hpp"

// Live pointer to the current main_page::state so esp_sync() can read toggles.
static main_page::state* g_esp_state = nullptr;

// Called once per frame (from draw()) to push UI toggle values into g_Options
// so the existing SILK PlayerEsp.cpp drawing code picks them up unchanged.
static void esp_sync(main_page::state& st)
{
    g_esp_state = &st;

    auto& P = g_Options.Visuals.ESP.Players;

    P.Enabled          = st.esp_active;
    P.Box              = st.esp_box;
    P.fillBox          = st.esp_fill_box;
    P.players_box      = st.esp_box_type;   // 0 = Full rect, 1 = Corner box
    P.Skeleton         = st.esp_skeleton;
    P.Lines            = st.esp_tracer;
    P.SnapLines        = st.esp_snaplines;
    P.Name             = st.esp_names;
    P.HealthBar        = st.esp_health;
    P.Distance         = st.esp_distance;
    P.WeaponInfo       = st.esp_weapon;
    P.IgnoreTrainingBots = st.esp_ignore_bots;
    P.RenderDistance   = st.esp_render_range;

    // Colour mappings from UI colour-pickers → SILK colour fields
    P.BoxColor              = st.esp_color_box;
    P.SkeletonColor         = st.esp_color_skeleton;
    P.NameColor             = st.esp_color_name;
    P.SnapLinesColor        = st.esp_color_tracer;
}
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    using ui_scale::s;

    // Build an ImU32 from a theme color with an optional alpha multiplier.
    ImU32 col(const ImVec4& c, float a = 1.f)
    {
        return ImGui::GetColorU32(ImVec4(c.x, c.y, c.z, c.w * a));
    }

    float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

    // =========================================================
    // ================== SIDEBAR & EXPANDED APP ================
    // =========================================================

    struct nav_item { const char* glyph; const char* label; };
    const nav_item k_nav[] = {
        { icons::aim,      "Aimbot" },
        { icons::eye,      "Visuals" },
        { icons::sliders,  "Loader" },
        { icons::settings, "Config" },
    };
    constexpr int k_nav_count = (int)(sizeof(k_nav) / sizeof(k_nav[0]));

    // Thin icon-only sidebar pinned to the left of the expanded panel.
    void draw_sidebar(ImDrawList* dl, const ImRect& panel, main_page::state& st, bool& blocks_drag)
    {
        const float w = s(layout::sidebar_w);
        const ImRect bar({ panel.Min.x + s(38.f), panel.Min.y + s(50.f) }, { panel.Min.x + s(38.f) + w, panel.Max.y - s(50.f) });
        const float round = s(layout::shell_round);

        // Sidebar background slab
        dl->AddRectFilled(bar.Min, bar.Max, col(colors::topbar), round);

        float y = bar.Min.y + s(20.f);

        // Custom branding emblem for "Sachin"
        const ImVec2 center = { bar.GetCenter().x, y + s(10.f) };
        const float radius = s(10.f);
        
        // Draw a stylized diamond/shield shape
        ImVec2 points[4] = {
            { center.x, center.y - radius },
            { center.x + radius, center.y },
            { center.x, center.y + radius },
            { center.x - radius, center.y }
        };
        dl->AddPolyline(points, 4, col(colors::accent), ImDrawFlags_Closed, s(2.0f));
        
        // Draw a small glowing center core
        dl->AddCircleFilled(center, s(4.f), col(colors::accent));
        dl->AddCircle(center, s(6.f), col(colors::accent, 0.4f), 0, s(1.0f));
        y += s(46.f);

        const float item = s(layout::sidebar_item);
        for (int i = 0; i < k_nav_count; ++i)
        {
            const ImRect ib({ bar.Min.x + (w - item) * 0.5f, y },
                            { bar.Min.x + (w - item) * 0.5f + item, y + item });
            const bool hovered = ib.Contains(ImGui::GetIO().MousePos);
            if (hovered) blocks_drag = true;

            ImGui::PushID(i + 100);
            ImGui::SetCursorScreenPos(ib.Min);
            ImGui::InvisibleButton("##nav", ib.GetSize());
            if (ImGui::IsItemClicked()) st.active_nav = i;
            ImGui::PopID();

            const bool sel = (st.active_nav == i);
            if (sel)
            {
                // Full background highlight effect (no outline) — filled accent tint
                dl->AddRectFilled(ib.Min, ib.Max, col(colors::accent, 0.18f), s(8.f));
            }
            else if (hovered)
            {
                dl->AddRectFilled(ib.Min, ib.Max, col(colors::widget_hover), s(8.f));
            }

            ImFont* icf = fonts::icon(15.f);
            const float ipx = fonts::design_to_pixel(15.f);
            const ImVec2 isz = icf->CalcTextSizeA(ipx, FLT_MAX, 0.f, k_nav[i].glyph);
            // Centering correction: adjust vertically by -s(1.0f) to center the icon glyph perfectly
            dl->AddText(icf, ipx,
                fonts::snap(ImVec2(ib.GetCenter().x - isz.x * 0.5f,
                    ib.GetCenter().y - isz.y * 0.5f - s(1.0f))),
                col(sel ? colors::accent : colors::text_secondary, sel ? 1.f : 0.6f), k_nav[i].glyph);

            y += item + s(layout::sidebar_gap);
        }
    }

    // Helper to lerp ImVec4 colors safely without depending on internal template overloads
    static ImVec4 lerp_color(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
    }

    // Draw a single glyph centered in 'box', rotated by 'angle' radians around its center.
    void draw_icon_rotated(ImDrawList* dl, ImFont* font, float px, const char* glyph,
        const ImRect& box, ImU32 color, float angle)
    {
        const ImVec2 gsz = font->CalcTextSizeA(px, FLT_MAX, 0.f, glyph);
        const ImVec2 center = box.GetCenter();
        const ImVec2 pos = { center.x - gsz.x * 0.5f, center.y - gsz.y * 0.5f };

        const int vtx_start = dl->VtxBuffer.Size;
        dl->AddText(font, px, fonts::snap(pos), color, glyph);
        const int vtx_end = dl->VtxBuffer.Size;

        if (angle != 0.f)
        {
            const float sn = sinf(angle), cs = cosf(angle);
            for (int i = vtx_start; i < vtx_end; ++i)
            {
                ImDrawVert& v = dl->VtxBuffer[i];
                const float dx = v.pos.x - center.x;
                const float dy = v.pos.y - center.y;
                v.pos.x = center.x + dx * cs - dy * sn;
                v.pos.y = center.y + dx * sn + dy * cs;
            }
        }
    }

    static const char* get_key_name(int key)
    {
        if (key >= ImGuiKey_MouseLeft && key <= ImGuiKey_MouseRight)
        {
            static char buf[16];
            sprintf_s(buf, sizeof(buf), "M%d", key - ImGuiKey_MouseLeft + 1);
            return buf;
        }
        return ImGui::GetKeyName((ImGuiKey)key);
    }

    // Draws all the dashboard tabs inside Sachin
    void draw_app_content(ImDrawList* dl, const ImRect& panel, main_page::state& st)
    {
        const float round = s(6.f); // Changed box rounding to sharp 6px
        const float right_x = panel.Max.x - s(38.f);
        
        // Define search bar bounding box (height 36px, situated 10px from the top)
        // Positioned right of the sidebar (38px sidebar left + 64px sidebar + 10px gap = 112px)
        const float ex_size = s(36.f);
        const ImRect exit_bb({ right_x - ex_size, panel.Min.y + s(50.f) },
                             { right_x, panel.Min.y + s(86.f) });

        const ImRect search_bb({ panel.Min.x + s(112.f), panel.Min.y + s(50.f) },
                               { right_x - ex_size - s(8.f), panel.Min.y + s(86.f) });

        // Draw search bar background slab (#18181C) with sharp 6px rounding
        const float search_round = s(6.f);
        dl->AddRectFilled(search_bb.Min, search_bb.Max, col(colors::topbar), search_round);

        // --- Exit button to the right of the search bar (0xe053) ---
        {
            ImGui::PushID("##exit_btn");
            ImGui::SetCursorScreenPos(exit_bb.Min);
            ImGui::InvisibleButton("##exit", exit_bb.GetSize());
            const bool ex_hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                st.exit_pressed = true;
            ImGui::PopID();

            dl->AddRectFilled(exit_bb.Min, exit_bb.Max, col(ex_hovered ? colors::danger : colors::topbar), search_round);

            ImFont* ex_f = fonts::icon(15.f);
            const float ex_px = fonts::design_to_pixel(15.f);
            const ImVec2 ex_isz = ex_f->CalcTextSizeA(ex_px, FLT_MAX, 0.f, icons::power);
            dl->AddText(ex_f, ex_px,
                        { exit_bb.GetCenter().x - ex_isz.x * 0.5f, exit_bb.GetCenter().y - ex_isz.y * 0.5f - s(0.5f) },
                        col(ex_hovered ? colors::text : colors::text_secondary), icons::power);
        }

        // Render input text field (hidden frame)
        static char search_buf[64] = "";

        // Query active state using the exact ImGui ID for the search input
        const ImGuiID search_id = ImGui::GetID("##search");
        const bool active = (ImGui::GetActiveID() == search_id);
        
        static float active_anim = 0.f;
        active_anim += (((active || search_buf[0] != '\0') ? 1.f : 0.f) - active_anim) * ImGui::GetIO().DeltaTime * 12.f;

        // Draw search magnifying glass icon (animates color to yellow on focus/text entry)
        ImFont* search_f = fonts::icon(14.f);
        const float search_f_px = fonts::design_to_pixel(14.f);
        const ImVec2 search_isz = search_f->CalcTextSizeA(search_f_px, FLT_MAX, 0.f, icons::search);
        const ImVec2 search_pos = {
            search_bb.Min.x + s(12.f),
            search_bb.GetCenter().y - search_isz.y * 0.5f - s(0.5f)
        };
        ImVec4 icon_color = lerp_color(colors::text_secondary, colors::accent, active_anim);
        dl->AddText(search_f, search_f_px, search_pos, col(icon_color), icons::search);

        // Draw custom text overlay (vertically centered, grey color)
        ImFont* search_text_f = fonts::inter(13.f);
        const float search_text_px = fonts::design_to_pixel(13.f);
        const ImVec2 text_sz = search_text_f->CalcTextSizeA(search_text_px, FLT_MAX, 0.f, search_buf);
        const ImVec2 text_pos = {
            search_bb.Min.x + s(32.f),
            search_bb.GetCenter().y - text_sz.y * 0.5f - s(0.5f)
        };
        if (search_buf[0] != '\0')
        {
            dl->AddText(search_text_f, search_text_px, text_pos, col(colors::text_secondary), search_buf);
        }

        // Draw smooth placeholder animation (slides left and fades)
        static float placeholder_anim = 0.f;
        const bool has_text_or_active = (search_buf[0] != '\0' || active);
        placeholder_anim += ((has_text_or_active ? 1.f : 0.f) - placeholder_anim) * ImGui::GetIO().DeltaTime * 12.f;

        if (placeholder_anim < 0.99f)
        {
            ImFont* placeholder_f = fonts::inter(13.f);
            const float placeholder_px = fonts::design_to_pixel(13.f);
            const ImVec2 placeholder_pos = {
                search_bb.Min.x + s(32.f) - placeholder_anim * s(12.f),
                search_bb.GetCenter().y - placeholder_px * 0.5f - s(0.5f)
            };
            dl->AddText(placeholder_f, placeholder_px, placeholder_pos, col(colors::text_muted, 1.f - placeholder_anim), "Search settings...");
        }

        // Standard ImGui InputText (fully transparent text & background to let custom renderer draw it)
        ImGui::PushFont(search_text_f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f)); // Invisible so we can overlay our custom styled text
        
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s(32.f), s(9.f)));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);

        ImGui::SetCursorScreenPos(search_bb.Min + ImVec2(0.f, s(2.5f)));

        ImGui::PushItemWidth(s(304.f));
        ImGui::InputText("##search", search_buf, IM_ARRAYSIZE(search_buf));
        ImGui::PopItemWidth();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        ImGui::PopFont();

        // Draw custom sliding blinking caret (typing animation) after InputText so it sits on top
        static float caret_x = 0.f;
        float target_caret_x = search_bb.Min.x + s(32.f) + text_sz.x;
        if (caret_x == 0.f || !active) caret_x = target_caret_x;
        else caret_x += (target_caret_x - caret_x) * ImGui::GetIO().DeltaTime * 20.f;

        if (active)
        {
            float blink = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 12.f);
            dl->AddLine(
                { caret_x, search_bb.Min.y + s(9.f) },
                { caret_x, search_bb.Max.y - s(9.f) },
                col(colors::accent, blink),
                s(1.5f)
            );
        }

        // --- Render the Settings Layout based on Selected Tab ---
        const float item_h = s(44.f);
        const float gap = s(10.f);

        // Scrollable content viewport: clipped to the grey backplate area below the search bar.
        const float content_top = panel.Min.y + s(96.f);
        const float content_bottom = panel.Max.y - s(48.f);
        const ImRect content_clip({ panel.Min.x + s(108.f), content_top - s(2.f) },
                                  { right_x + s(6.f), content_bottom });

        // Detect tab switches -> restart the tab-in animation and snap scroll back to the top.
        if (st.active_nav != st.prev_nav)
        {
            st.prev_nav = st.active_nav;
            st.tab_anim = 0.f;
            st.content_scroll_target = 0.f;
            st.content_scroll = 0.f;
        }
        // Ease the tab-in animation (fade + slight upward slide).
        st.tab_anim += (1.f - st.tab_anim) * ImGui::GetIO().DeltaTime * 9.f;
        if (st.tab_anim > 0.999f) st.tab_anim = 1.f;

        // Mouse-wheel scroll handling -> move the TARGET; the actual offset eases toward it.
        const bool content_hovered = content_clip.Contains(ImGui::GetIO().MousePos);
        if (content_hovered && st.open_popup == 0)
        {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.f)
                st.content_scroll_target -= wheel * s(64.f);
        }

        // Smoothly ease the current scroll toward the target (super smooth).
        st.content_scroll += (st.content_scroll_target - st.content_scroll) * ImMin(1.f, ImGui::GetIO().DeltaTime * 16.f);

        // Tab-in vertical slide offset (eases content up into place).
        const float tab_slide = (1.f - st.tab_anim) * s(18.f);
        float row_y = content_top - st.content_scroll + tab_slide;

        dl->PushClipRect(content_clip.Min, content_clip.Max, true);

        // Keybind Listening Loop for Aimbot and Triggerbot
        if (st.binding_key)
        {
            for (int i = 0; i < 512; ++i)
            {
                if (ImGui::IsKeyPressed((ImGuiKey)i))
                {
                    st.aim_keybind = i;
                    st.binding_key = false;
                    break;
                }
            }
            for (int m = 0; m < 5; ++m)
            {
                if (ImGui::IsMouseClicked(m))
                {
                    st.aim_keybind = (int)ImGuiKey_MouseLeft + m;
                    st.binding_key = false;
                    break;
                }
            }
        }
        else if (st.binding_trigger_key)
        {
            for (int i = 0; i < 512; ++i)
            {
                if (ImGui::IsKeyPressed((ImGuiKey)i))
                {
                    st.trigger_keybind = i;
                    st.binding_trigger_key = false;
                    break;
                }
            }
            for (int m = 0; m < 5; ++m)
            {
                if (ImGui::IsMouseClicked(m))
                {
                    st.trigger_keybind = (int)ImGuiKey_MouseLeft + m;
                    st.binding_trigger_key = false;
                    break;
                }
            }
        }

        // Search match helper
        auto matches_search = [&](const char* label)
        {
            if (search_buf[0] == '\0') return true;
            std::string text_str(label);
            std::string q_str(search_buf);
            std::transform(text_str.begin(), text_str.end(), text_str.begin(), ::tolower);
            std::transform(q_str.begin(), q_str.end(), q_str.begin(), ::tolower);
            return text_str.find(q_str) != std::string::npos;
        };

        // Returns true if a row spanning [row_y, row_y + h] is at least partially within the viewport.
        auto row_visible = [&](float h) -> bool
        {
            return (row_y + h) > content_top && row_y < content_bottom;
        };

        // Shared cog: bigger, always grey, rotates right on hover and back on unhover.
        static std::unordered_map<int, float> cog_anim; // popup_idx -> rotation amount [0,1]
        auto draw_cog = [&](int popup_idx, float center_x, float center_y) -> float
        {
            // returns the cog's left x (so callers can place content to its left)
            const float cog_box = s(22.f); // bigger hit/visual box
            const ImRect cog_bb({ center_x - cog_box, center_y - cog_box * 0.5f },
                                { center_x,           center_y + cog_box * 0.5f });

            ImGui::PushID(popup_idx + 7000);
            ImGui::SetCursorScreenPos(cog_bb.Min);
            ImGui::InvisibleButton("##cog", cog_bb.GetSize());
            const bool hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                st.open_popup = (st.open_popup == popup_idx) ? 0 : popup_idx;
            ImGui::PopID();

            float& a = cog_anim[popup_idx];
            a += ((hovered ? 1.f : 0.f) - a) * ImGui::GetIO().DeltaTime * 12.f;

            ImFont* icon_f = fonts::icon(16.f); // bigger
            const float icon_px = fonts::design_to_pixel(16.f);
            const float angle = a * 1.5708f; // up to 90deg clockwise
            draw_icon_rotated(dl, icon_f, icon_px, icons::cog, cog_bb,
                col(colors::text_secondary), angle); // always grey
            return cog_bb.Min.x;
        };

        // Section Title rendering helper — grey label on the left + thin divider line filling the row
        auto draw_section_title = [&](const char* title)
        {
            ImFont* tf = fonts::inter(11.5f, true);
            const float tpx = fonts::design_to_pixel(11.5f);
            const ImVec2 title_pos = { panel.Min.x + s(112.f), row_y + s(4.f) };
            const ImVec2 title_sz = tf->CalcTextSizeA(tpx, FLT_MAX, 0.f, title);
            dl->AddText(tf, tpx, title_pos, col(colors::text_muted), title);

            // Subtle divider line trailing to the right of the grey title
            const float line_y = title_pos.y + title_sz.y * 0.5f;
            const float line_x0 = title_pos.x + title_sz.x + s(10.f);
            const float line_x1 = right_x;
            if (line_x1 > line_x0)
                dl->AddLine({ line_x0, line_y }, { line_x1, line_y }, col(colors::divider), s(1.0f));

            row_y += s(26.f);
        };

        // Toggle Switch row helper
        auto draw_toggle_row = [&](const char* label, bool& value, int popup_idx, const char* inline_val_str = nullptr, const ImVec4* color_preview = nullptr)
        {
            if (!matches_search(label)) return;
            if (!row_visible(item_h)) { row_y += item_h + gap; return; }

            const ImRect box_bb({ panel.Min.x + s(112.f), row_y }, { right_x, row_y + item_h });
            dl->AddRectFilled(box_bb.Min, box_bb.Max, col(colors::topbar), round);

            const float center_y = row_y + item_h * 0.5f;

            // Text Label — larger + slightly thicker; grey when OFF, white when ON
            {
                ImFont* lbl_f = fonts::inter(12.5f, false);
                const float lbl_px = fonts::design_to_pixel(12.5f);
                dl->AddText(lbl_f, lbl_px,
                            { box_bb.Min.x + s(15.f), center_y - lbl_px * 0.5f },
                            col(value ? colors::text : colors::text_secondary), label);
            }

            const float right_x = box_bb.Max.x - s(15.f);

            // Checkbox Bounds (square)
            const ImVec2 toggle_size = { s(18.f), s(18.f) };
            const ImVec2 toggle_min = { right_x - toggle_size.x, center_y - toggle_size.y * 0.5f };
            const ImVec2 toggle_max = { right_x, center_y + toggle_size.y * 0.5f };

            float next_right_x = toggle_min.x;

            // Render Cog if popup_idx is defined
            if (popup_idx > 0)
            {
                next_right_x = draw_cog(popup_idx, toggle_min.x - s(12.f), center_y);
            }

            // Inline color preview
            if (color_preview)
            {
                const float box_size = s(12.f);
                const ImVec2 c_min = { next_right_x - s(10.f) - box_size, center_y - box_size * 0.5f };
                const ImVec2 c_max = { next_right_x - s(10.f), center_y + box_size * 0.5f };
                dl->AddRectFilled(c_min, c_max, col(*color_preview), s(2.f));
                next_right_x -= (box_size + s(10.f));
            }

            // Inline value display
            if (inline_val_str)
            {
                ImFont* val_f = fonts::inter(13.f);
                const float val_f_px = fonts::design_to_pixel(13.f);
                const ImVec2 val_sz = val_f->CalcTextSizeA(val_f_px, FLT_MAX, 0.f, inline_val_str);
                dl->AddText(val_f, val_f_px, { next_right_x - s(10.f) - val_sz.x, center_y - val_sz.y * 0.5f }, col(colors::text_secondary), inline_val_str);
            }

            // Checkbox interaction
            ImGui::PushID(label);
            ImGui::SetCursorScreenPos(toggle_min);
            ImGui::InvisibleButton("##toggle", toggle_size);
            const bool cb_hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                value = !value;
            ImGui::PopID();

            // Animate the check (fill + tick reveal)
            static std::unordered_map<std::string, float> anims;
            float& anim = anims[label];
            anim += (((value) ? 1.f : 0.f) - anim) * ImGui::GetIO().DeltaTime * 16.f;

            const float cb_round = s(2.f); // mostly square, slightly softened corners

            // Box background: fills with accent as it animates on
            const ImU32 box_bg = col(lerp_color(cb_hovered ? colors::widget_hover : colors::widget, colors::accent, anim));
            dl->AddRectFilled(toggle_min, toggle_max, box_bg, cb_round);

            // Checkmark glyph (0xe09c), fades in with the animation
            if (anim > 0.02f)
            {
                ImFont* ck_f = fonts::icon(11.f);
                const float ck_px = fonts::design_to_pixel(11.f);
                const ImVec2 ck_sz = ck_f->CalcTextSizeA(ck_px, FLT_MAX, 0.f, icons::ok_2);
                const ImVec2 ck_pos = {
                    (toggle_min.x + toggle_max.x) * 0.5f - ck_sz.x * 0.5f,
                    (toggle_min.y + toggle_max.y) * 0.5f - ck_sz.y * 0.5f
                };
                dl->AddText(ck_f, ck_px, ck_pos, col(colors::bg, anim), icons::ok_2);
            }

            row_y += item_h + gap;
        };

        // Slider row helper
        auto draw_slider_row = [&](const char* label, float& value, float min_val, float max_val, int popup_idx, const char* inline_val_str = nullptr)
        {
            if (!matches_search(label)) return;
            if (!row_visible(item_h)) { row_y += item_h + gap; return; }

            const ImRect box_bb({ panel.Min.x + s(112.f), row_y }, { right_x, row_y + item_h });
            dl->AddRectFilled(box_bb.Min, box_bb.Max, col(colors::topbar), round);

            const float center_y = row_y + item_h * 0.5f;

            // Text Label — larger + slightly thicker
            {
                ImFont* lbl_f = fonts::inter(12.5f, false);
                const float lbl_px = fonts::design_to_pixel(12.5f);
                dl->AddText(lbl_f, lbl_px,
                            { box_bb.Min.x + s(15.f), center_y - lbl_px * 0.5f },
                            col(colors::text), label);
            }

            const float right_x = box_bb.Max.x - s(15.f);

            // Optional Cog Wheel bounds
            float next_right_x = right_x;
            if (popup_idx > 0)
            {
                next_right_x = draw_cog(popup_idx, right_x, center_y);
            }

            // Slider geometry (width 100px)
            const float slider_w = s(100.f);
            const float track_h = s(4.f);  // thin grey background track
            const float fill_h  = s(10.f); // thicker colored slider on top
            const float track_r = track_h * 0.5f;
            const float fill_r  = fill_h * 0.5f;
            const float track_right = next_right_x - (popup_idx > 0 ? s(12.f) : 0.f);
            const float track_left = track_right - slider_w;
            const ImVec2 track_min = { track_left, center_y - track_h * 0.5f };
            const ImVec2 track_max = { track_right, center_y + track_h * 0.5f };

            // Format slider value and draw it to the left of the slider
            char val_buf[64];
            if (inline_val_str)
            {
                sprintf_s(val_buf, sizeof(val_buf), "%s  %.1f", inline_val_str, value);
            }
            else
            {
                sprintf_s(val_buf, sizeof(val_buf), "%.1f", value);
            }
            ImFont* val_f = fonts::inter(13.f);
            const float val_f_px = fonts::design_to_pixel(13.f);
            const ImVec2 val_sz = val_f->CalcTextSizeA(val_f_px, FLT_MAX, 0.f, val_buf);
            dl->AddText(val_f, val_f_px, { track_min.x - s(10.f) - val_sz.x, center_y - val_sz.y * 0.5f }, col(colors::text_secondary), val_buf);

            // Slider interaction
            ImGui::PushID(label);
            ImGui::SetCursorScreenPos({ track_min.x, center_y - s(12.f) });
            ImGui::InvisibleButton("##slider", ImVec2(slider_w, s(24.f)));
            if (ImGui::IsItemActive())
            {
                float mouse_x = ImGui::GetIO().MousePos.x;
                float pct = (mouse_x - track_min.x) / slider_w;
                if (pct < 0.f) pct = 0.f;
                if (pct > 1.f) pct = 1.f;
                value = min_val + pct * (max_val - min_val);
            }
            ImGui::PopID();

            // Draw thin grey background track (fully rounded)
            dl->AddRectFilled(track_min, track_max, col(colors::widget_hover), track_r);

            // Draw thicker accent slider fill on top (no thumb/ball)
            float pct = (value - min_val) / (max_val - min_val);
            if (pct < 0.f) pct = 0.f;
            if (pct > 1.f) pct = 1.f;
            const float fill_w = pct * slider_w;
            if (fill_w > 0.f)
            {
                const ImVec2 fill_min = { track_left, center_y - fill_h * 0.5f };
                const ImVec2 fill_max = { track_left + fill_w, center_y + fill_h * 0.5f };
                dl->AddRectFilled(fill_min, fill_max, col(colors::accent), fill_r); // accent slider

                // Small dark ball at the end of the fill, inside it (smaller than fill thickness)
                const float ball_r = s(3.f); // < fill_h * 0.5 (5px) so it stays inside the slider
                const float ball_cx = fill_max.x - fill_r; // tucked inside the rounded end
                dl->AddCircleFilled({ ball_cx, center_y }, ball_r, col(colors::bg));
            }

            row_y += item_h + gap;
        };

        // Value row helper: label + inline value + cog (no toggle). Used for selector-style rows.
        auto draw_value_row = [&](const char* label, const char* value_str, int popup_idx, const ImVec4* color_preview = nullptr)
        {
            if (!matches_search(label)) return;
            if (!row_visible(item_h)) { row_y += item_h + gap; return; }

            const ImRect box_bb({ panel.Min.x + s(112.f), row_y }, { right_x, row_y + item_h });
            dl->AddRectFilled(box_bb.Min, box_bb.Max, col(colors::topbar), round);

            const float center_y = row_y + item_h * 0.5f;

            {
                ImFont* lbl_f = fonts::inter(12.5f, false);
                const float lbl_px = fonts::design_to_pixel(12.5f);
                dl->AddText(lbl_f, lbl_px,
                            { box_bb.Min.x + s(15.f), center_y - lbl_px * 0.5f },
                            col(colors::text), label);
            }

            const float right_x = box_bb.Max.x - s(15.f);
            float next_right_x = right_x;

            if (popup_idx > 0)
            {
                next_right_x = draw_cog(popup_idx, right_x, center_y);
            }

            // Inline color preview swatch
            if (color_preview)
            {
                const float box_size = s(12.f);
                const ImVec2 c_min = { next_right_x - s(10.f) - box_size, center_y - box_size * 0.5f };
                const ImVec2 c_max = { next_right_x - s(10.f), center_y + box_size * 0.5f };
                dl->AddRectFilled(c_min, c_max, col(*color_preview), s(2.f));
                next_right_x -= (box_size + s(10.f));
            }

            if (value_str)
            {
                ImFont* val_f = fonts::inter(13.f);
                const float val_f_px = fonts::design_to_pixel(13.f);
                const ImVec2 val_sz = val_f->CalcTextSizeA(val_f_px, FLT_MAX, 0.f, value_str);
                dl->AddText(val_f, val_f_px, { next_right_x - s(10.f) - val_sz.x, center_y - val_sz.y * 0.5f }, col(colors::accent), value_str);
            }

            row_y += item_h + gap;
        };

        // Render Tabs
        if (st.active_nav == 0) // Aim & Combat
        {
            draw_section_title("Aimbot");

            draw_toggle_row("Aimbot enabled", st.aim_active, 1);
            draw_slider_row("Smoothing", st.aim_smooth, 1.f, 30.f, 2);

            draw_toggle_row("Aimbone", st.aim_sticky, 3);
            draw_slider_row("FOV", st.aim_fov, 1.f, 180.f, 0);
            draw_toggle_row("Visible Only", st.aim_visible_only, 0);
            draw_toggle_row("Sticky Aim", st.aim_sticky, 0);

            draw_section_title("Triggerbot");
            draw_toggle_row("Triggerbot enabled", st.trigger_active, 6);
            draw_slider_row("Trigger delay", st.aim_delay, 0.f, 150.f, 0);

            draw_section_title("Recoil Control");
            draw_toggle_row("Enable RCS", st.combat_rcs, 0);
            draw_slider_row("Horizontal", st.combat_rcs_h, 0.f, 100.f, 0);
            draw_slider_row("Vertical", st.combat_rcs_v, 0.f, 100.f, 0);

            draw_section_title("Misc");
            draw_toggle_row("Bunny Hop", st.combat_bunnyhop, 0);
        }
        else if (st.active_nav == 1) // Visuals (ESP)
        {
            draw_section_title("ESP");
            draw_toggle_row("Enable ESP", st.esp_active, 0);

            draw_toggle_row("Box ESP", st.esp_box, 4);
            draw_toggle_row("Fill Box", st.esp_fill_box, 0);
            draw_value_row("Box Type", nullptr, 7);
            draw_toggle_row("Skeleton ESP", st.esp_skeleton, 5);
            draw_toggle_row("Head Circle", st.esp_head_circle, 0);
            draw_toggle_row("Tracers", st.esp_tracer, 10);
            draw_toggle_row("Snaplines", st.esp_snaplines, 0);
            draw_toggle_row("Off-screen Arrows", st.esp_arrows, 0);
            draw_toggle_row("Mini Radar", st.esp_radar, 0);

            draw_section_title("Player Info");
            draw_toggle_row("Draw Names", st.esp_names, 8);
            draw_toggle_row("Health Bar", st.esp_health, 9);
            draw_toggle_row("Distance", st.esp_distance, 0);
            draw_toggle_row("Active Weapon", st.esp_weapon, 0);
            draw_toggle_row("Ignore Training Bots", st.esp_ignore_bots, 0);

            draw_section_title("Effects & Range");
            draw_slider_row("Max Distance", st.esp_render_range, 50.f, 500.f, 0);
            draw_slider_row("Glow thickness", st.esp_glow, 0.f, 10.f, 0);
            draw_slider_row("ESP thickness", st.esp_thickness, 0.f, 5.f, 0);
            draw_slider_row("Box fill opacity", st.esp_fill_opacity, 0.f, 1.f, 0);

            draw_section_title("World");
            draw_toggle_row("FOV Circle", st.visual_fov_circle, 0);
            draw_toggle_row("Custom Crosshair", st.visual_crosshair, 0);
            draw_toggle_row("Watermark", st.visual_watermark, 0);
            draw_toggle_row("FPS Counter", st.visual_fps, 0);
        }
        else if (st.active_nav == 2) // Loader
        {
            draw_section_title("Injection Settings");
            draw_toggle_row("Detours bypass", st.hook_detours, 0);
            draw_toggle_row("Thread hijacking", st.thread_hijack, 0);
            draw_toggle_row("Kernel bypass", st.bypass_active, 0);
        }
        else if (st.active_nav == 3) // Config
        {
            draw_section_title("Profiles");
            const char* config_names[] = { "Default Config", "Legit Config", "Rage Config" };
            float float_cfg = (float)st.active_config;
            draw_slider_row("Active Profile", float_cfg, 0.f, 2.f, 0, config_names[st.active_config % 3]);
            st.active_config = (int)float_cfg;

            draw_section_title("Appearance");
            draw_value_row("Accent Color", nullptr, 11);
            draw_slider_row("UI Opacity", st.ui_opacity, 0.2f, 1.f, 0);

            draw_section_title("Interface");
            // Global ImGui scale — grows the whole interface (sizes + fonts), not just the window.
            draw_slider_row("Interface Scale", st.ui_scale, 0.80f, 1.50f, 0);
        }

        // Push the user scale into the global so prepare_frame() rescales everything next frame.
        ui_scale::user_scale = st.ui_scale;

        dl->PopClipRect();

        // --- Scroll clamp for the content viewport (mouse-wheel only, no visible scrollbar) ---
        {
            const float viewport_h = content_bottom - content_top;
            // Back out scroll + tab slide to recover total content height.
            const float content_h = (row_y + st.content_scroll - tab_slide) - content_top;
            const float max_scroll = content_h - viewport_h > 0.f ? content_h - viewport_h : 0.f;

            // Clamp the TARGET; the visible offset eases toward it for a smooth feel.
            if (st.content_scroll_target < 0.f)          st.content_scroll_target = 0.f;
            if (st.content_scroll_target > max_scroll)   st.content_scroll_target = max_scroll;
        }

        // --- Render Slide-out Selection Popup ---
        const ImVec2 bp_min = panel.Min + ImVec2(s(28.f), s(40.f));
        const ImVec2 bp_max = panel.Min + ImVec2(s(472.f), s(460.f));

        static float popup_anim = 0.f;
        popup_anim += (((st.open_popup != 0) ? 1.f : 0.f) - popup_anim) * ImGui::GetIO().DeltaTime * 12.f;

        // Is the active popup a color picker (HSV square + hue + opacity)?
        auto is_color_popup = [](int p) {
            return p == 4 || p == 5 || p == 8 || p == 9 || p == 10 || p == 11;
        };

        // Re-sync the picker HSV from the live RGBA next time a picker opens.
        if (!is_color_popup(st.open_popup))
            st.pick_init_for = 0;

        if (popup_anim > 0.01f)
        {
            const bool color_popup = is_color_popup(st.open_popup);
            const float popup_w = color_popup ? s(232.f) : s(160.f);

            // Dynamic popup height animation depending on selected menu option
            float target_h = 110.f;
            // Color picker height = pad + square(inner_w) + gap + hue row + gap + opacity row + bottom pad
            //   inner_w = popup_w - 2*pad = 232 - 24 = 208 (design px)
            if (color_popup)            target_h = 12.f + 208.f + 8.f + 28.f + 6.f + 28.f + 12.f; // = 302
            else if (st.open_popup == 3) target_h = 142.f; // Aimbone (4 rows)
            else                         target_h = 110.f;

            static float popup_h_anim = 110.f;
            popup_h_anim += (target_h - popup_h_anim) * ImGui::GetIO().DeltaTime * 14.f;

            const float popup_h = s(popup_h_anim);
            
            // Render Popups 1, 2, 4, 5, 6 outside the main GUI shell (10px from the right boundary of the entire panel)
            // Render Popup 3 (Aimbone) sliding out from behind the grey GUI backplate border
            float target_x = bp_max.x - s(10.f);
            if (st.open_popup != 3)
            {
                // All popouts except Aimbone (3) slide out from the right edge of the whole panel
                target_x = panel.Max.x - s(10.f);
            }
            const float current_x = target_x + (popup_anim - 1.f) * s(20.f);
            const ImRect popup_bb({ current_x, bp_min.y }, { current_x + popup_w, bp_min.y + popup_h });

            // Draw background container with sharp 6px rounding and animated opacity
            dl->AddRectFilled(popup_bb.Min, popup_bb.Max, col(colors::topbar, popup_anim), s(6.f));

            const float start_y = popup_bb.Min.y + s(11.f);

            // Aimbot enabled Cog popout contents (Keybind + Mode toggle/hold)
            if (st.open_popup == 1)
            {
                // Row 1: Keybind listener
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y }, { popup_bb.Max.x, start_y + s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::InvisibleButton("##aim_keybind_btn", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.binding_key = !st.binding_key;

                    if (hovered && !st.binding_key)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    // Keybind text label
                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, col(colors::text, popup_anim), "Keybind");

                    // Keybind value text (blinks/glows yellow when binding)
                    char val_buf[32];
                    if (st.binding_key)
                    {
                        sprintf_s(val_buf, sizeof(val_buf), "[Press Key]");
                    }
                    else
                    {
                        sprintf_s(val_buf, sizeof(val_buf), "[%s]", get_key_name(st.aim_keybind));
                    }
                    const ImVec2 val_sz = bone_f->CalcTextSizeA(bone_px, FLT_MAX, 0.f, val_buf);
                    
                    ImU32 val_col = st.binding_key ? col(colors::accent, (0.5f + 0.5f * sinf((float)ImGui::GetTime() * 12.f)) * popup_anim) : col(colors::text_secondary, popup_anim);
                    dl->AddText(bone_f, bone_px, { row_bb.Max.x - s(15.f) - val_sz.x, text_y }, val_col, val_buf);
                }

                // Row 2 & 3: Mode hold / toggle
                const char* modes[] = { "Hold", "Toggle" };
                for (int i = 0; i < 2; ++i)
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y + (i + 1) * s(32.f) }, { popup_bb.Max.x, start_y + (i + 2) * s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::PushID(i + 100);
                    ImGui::InvisibleButton("##aim_mode_row", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.aim_mode = i;
                    ImGui::PopID();

                    const bool selected = (st.aim_mode == i);

                    if (hovered && !selected)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    if (selected)
                    {
                        // Selected-effect color: brighter accent tint fill + soft accent outline (no vertical bar)
                        const ImVec2 fill_min = row_bb.Min + ImVec2(s(4.f), s(2.f));
                        const ImVec2 fill_max = row_bb.Max - ImVec2(s(4.f), s(2.f));
                        dl->AddRectFilled(fill_min, fill_max, col(colors::accent, 0.22f * popup_anim), s(5.f));
                    }

                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, 
                                col(selected ? colors::accent : colors::text, popup_anim), modes[i]);

                    if (selected)
                    {
                        ImFont* check_f = fonts::icon(12.f);
                        const float check_px = fonts::design_to_pixel(12.f);
                        const ImVec2 check_sz = check_f->CalcTextSizeA(check_px, FLT_MAX, 0.f, icons::check);
                        
                        dl->AddText(check_f, check_px, { row_bb.Max.x - s(10.f) - check_sz.x, row_bb.Min.y + (row_bb.GetHeight() - check_sz.y) * 0.5f },
                                    col(colors::accent, popup_anim), icons::check);
                    }
                }
            }
            // Smoothing Cog popout contents (Gradual, Decelerate, Static)
            else if (st.open_popup == 2)
            {
                const char* smooth_options[] = { "Gradual", "Decelerate", "Static" };
                for (int i = 0; i < 3; ++i)
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y + i * s(32.f) }, { popup_bb.Max.x, start_y + (i + 1) * s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::PushID(i + 300);
                    ImGui::InvisibleButton("##smooth_row", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.smooth_mode = i;
                    ImGui::PopID();

                    const bool selected = (st.smooth_mode == i);

                    if (hovered && !selected)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    if (selected)
                    {
                        // Selected-effect color: brighter accent tint fill + soft accent outline (no vertical bar)
                        const ImVec2 fill_min = row_bb.Min + ImVec2(s(4.f), s(2.f));
                        const ImVec2 fill_max = row_bb.Max - ImVec2(s(4.f), s(2.f));
                        dl->AddRectFilled(fill_min, fill_max, col(colors::accent, 0.22f * popup_anim), s(5.f));
                    }

                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, 
                                col(selected ? colors::accent : colors::text, popup_anim), smooth_options[i]);

                    if (selected)
                    {
                        ImFont* check_f = fonts::icon(12.f);
                        const float check_px = fonts::design_to_pixel(12.f);
                        const ImVec2 check_sz = check_f->CalcTextSizeA(check_px, FLT_MAX, 0.f, icons::check);
                        
                        dl->AddText(check_f, check_px, { row_bb.Max.x - s(10.f) - check_sz.x, row_bb.Min.y + (row_bb.GetHeight() - check_sz.y) * 0.5f },
                                    col(colors::accent, popup_anim), icons::check);
                    }
                }
            }
            // Aimbone Cog popout contents (Head, Neck, Chest, Pelvis)
            else if (st.open_popup == 3)
            {
                const char* bone_names[] = { "Head", "Neck", "Chest", "Pelvis" };
                for (int i = 0; i < 4; ++i)
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y + i * s(32.f) }, { popup_bb.Max.x, start_y + (i + 1) * s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::PushID(i + 500);
                    ImGui::InvisibleButton("##bone_row", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.aim_bone = i;
                    ImGui::PopID();

                    const bool selected = (st.aim_bone == i);
                    
                    if (hovered && !selected)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    if (selected)
                    {
                        // Selected-effect color: brighter accent tint fill + soft accent outline (no vertical bar)
                        const ImVec2 fill_min = row_bb.Min + ImVec2(s(4.f), s(2.f));
                        const ImVec2 fill_max = row_bb.Max - ImVec2(s(4.f), s(2.f));
                        dl->AddRectFilled(fill_min, fill_max, col(colors::accent, 0.22f * popup_anim), s(5.f));
                    }

                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, 
                                col(selected ? colors::accent : colors::text, popup_anim), bone_names[i]);

                    if (selected)
                    {
                        ImFont* check_f = fonts::icon(12.f);
                        const float check_px = fonts::design_to_pixel(12.f);
                        const ImVec2 check_sz = check_f->CalcTextSizeA(check_px, FLT_MAX, 0.f, icons::check);
                        
                        dl->AddText(check_f, check_px, { row_bb.Max.x - s(10.f) - check_sz.x, row_bb.Min.y + (row_bb.GetHeight() - check_sz.y) * 0.5f },
                                    col(colors::accent, popup_anim), icons::check);
                    }
                }
            }
            // Color picker (HSV square + hue slider + opacity slider) — matches Portfolio #12
            else if (is_color_popup(st.open_popup))
            {
                ImVec4* color_ptr = &st.esp_color_box;
                if      (st.open_popup == 5)  color_ptr = &st.esp_color_skeleton;
                else if (st.open_popup == 8)  color_ptr = &st.esp_color_name;
                else if (st.open_popup == 9)  color_ptr = &st.esp_color_health;
                else if (st.open_popup == 10) color_ptr = &st.esp_color_tracer;
                else if (st.open_popup == 11) color_ptr = &colors::accent;
                ImVec4& value = *color_ptr;

                // Sync HSV from the stored RGBA once when this picker opens
                if (st.pick_init_for != st.open_popup)
                {
                    ImGui::ColorConvertRGBtoHSV(value.x, value.y, value.z, st.pick_h, st.pick_s, st.pick_v);
                    st.pick_a = value.w;
                    st.pick_init_for = st.open_popup;
                }

                const float pad = s(12.f);
                const float inner_w = popup_w - pad * 2.f;
                const float sq_x = popup_bb.Min.x + pad;
                const float sq_y = popup_bb.Min.y + pad;
                const float sq_size = inner_w; // square SV area

                // === SV square ===
                const ImRect sv_bb({ sq_x, sq_y }, { sq_x + sq_size, sq_y + sq_size });
                {
                    ImGui::PushID("##sv_picker");
                    ImGui::SetCursorScreenPos(sv_bb.Min);
                    ImGui::InvisibleButton("##sv", sv_bb.GetSize());
                    if (ImGui::IsItemActive())
                    {
                        const float ns = clamp01((ImGui::GetIO().MousePos.x - sv_bb.Min.x) / sv_bb.GetWidth());
                        const float nv = 1.f - clamp01((ImGui::GetIO().MousePos.y - sv_bb.Min.y) / sv_bb.GetHeight());
                        st.pick_s = ns; st.pick_v = nv;
                    }
                    ImGui::PopID();

                    const ImU32 hue_col = ImColor::HSV(st.pick_h, 1.f, 1.f);
                    const ImU32 white = col(ImVec4(1, 1, 1, 1), popup_anim);
                    const ImU32 black = col(ImVec4(0, 0, 0, 1), popup_anim);
                    const ImU32 trans = ImGui::GetColorU32(ImVec4(0, 0, 0, 0));
                    dl->PushClipRect(sv_bb.Min, sv_bb.Max, true);
                    dl->AddRectFilledMultiColor(sv_bb.Min, sv_bb.Max, white, hue_col, hue_col, white);
                    dl->AddRectFilledMultiColor(sv_bb.Min, sv_bb.Max, trans, trans, black, black);
                    dl->PopClipRect();

                    const float sel_x = sv_bb.Min.x + sv_bb.GetWidth() * st.pick_s;
                    const float sel_y = sv_bb.Min.y + sv_bb.GetHeight() * (1.f - st.pick_v);
                    dl->AddCircle({ sel_x, sel_y }, s(5.f), col(ImVec4(1, 1, 1, 1), popup_anim), 24, s(2.f));
                    dl->AddCircle({ sel_x, sel_y }, s(4.f), col(ImVec4(0, 0, 0, 0.47f), popup_anim), 24, s(1.f));
                }

                const float track_h = s(10.f);
                const float row_h = s(28.f);
                float cursor_y = sv_bb.Max.y + s(8.f);

                // === Hue slider ===
                {
                    const ImRect hue_bb({ sq_x, cursor_y + (row_h - track_h) * 0.5f },
                                        { sq_x + sq_size, cursor_y + (row_h - track_h) * 0.5f + track_h });
                    ImGui::PushID("##hue_slider");
                    ImGui::SetCursorScreenPos({ hue_bb.Min.x, cursor_y });
                    ImGui::InvisibleButton("##hue", ImVec2(sq_size, row_h));
                    if (ImGui::IsItemActive())
                        st.pick_h = clamp01((ImGui::GetIO().MousePos.x - hue_bb.Min.x) / hue_bb.GetWidth());
                    ImGui::PopID();

                    const float rad = track_h * 0.5f;
                    const int cols = (int)(hue_bb.GetWidth() / s(2.f));
                    for (int i = 0; i < cols; ++i)
                    {
                        const float t0 = (float)i / (float)cols;
                        const float t1 = (float)(i + 1) / (float)cols;
                        const ImU32 hc = ImColor::HSV((t0 + t1) * 0.5f, 1.f, 1.f);
                        ImDrawFlags flags = ImDrawFlags_RoundCornersNone;
                        if (i == 0) flags = ImDrawFlags_RoundCornersLeft;
                        else if (i == cols - 1) flags = ImDrawFlags_RoundCornersRight;
                        dl->AddRectFilled({ ImLerp(hue_bb.Min.x, hue_bb.Max.x, t0), hue_bb.Min.y },
                                          { ImLerp(hue_bb.Min.x, hue_bb.Max.x, t1), hue_bb.Max.y }, hc, rad, flags);
                    }
                    const float gw = s(5.f), gh = s(18.f);
                    const float cx = ImLerp(hue_bb.Min.x + gh * 0.5f, hue_bb.Max.x - gh * 0.5f, st.pick_h);
                    dl->AddRectFilled({ cx - gw * 0.5f, hue_bb.GetCenter().y - gh * 0.5f },
                                      { cx + gw * 0.5f, hue_bb.GetCenter().y + gh * 0.5f }, col(ImVec4(1, 1, 1, 1), popup_anim), s(3.f));
                    cursor_y += row_h + s(6.f);
                }

                // RGB from current HSV (for the opacity gradient)
                ImVec4 rgb = value;
                ImGui::ColorConvertHSVtoRGB(st.pick_h, st.pick_s, st.pick_v, rgb.x, rgb.y, rgb.z);

                // === Opacity slider ===
                {
                    const ImRect a_bb({ sq_x, cursor_y + (row_h - track_h) * 0.5f },
                                      { sq_x + sq_size, cursor_y + (row_h - track_h) * 0.5f + track_h });
                    ImGui::PushID("##alpha_slider");
                    ImGui::SetCursorScreenPos({ a_bb.Min.x, cursor_y });
                    ImGui::InvisibleButton("##alpha", ImVec2(sq_size, row_h));
                    if (ImGui::IsItemActive())
                        st.pick_a = clamp01((ImGui::GetIO().MousePos.x - a_bb.Min.x) / a_bb.GetWidth());
                    ImGui::PopID();

                    const float round = track_h * 0.5f;
                    dl->PushClipRect(a_bb.Min, a_bb.Max, true);
                    const float cell = s(6.f);
                    const ImU32 c0 = col(ImVec4(42.f / 255.f, 42.f / 255.f, 50.f / 255.f, 1.f), popup_anim);
                    const ImU32 c1 = col(ImVec4(32.f / 255.f, 32.f / 255.f, 38.f / 255.f, 1.f), popup_anim);
                    for (float y = a_bb.Min.y; y < a_bb.Max.y; y += cell)
                        for (float x = a_bb.Min.x; x < a_bb.Max.x; x += cell)
                        {
                            const int ix = (int)((x - a_bb.Min.x) / cell);
                            const int iy = (int)((y - a_bb.Min.y) / cell);
                            dl->AddRectFilled({ x, y }, { ImMin(x + cell, a_bb.Max.x), ImMin(y + cell, a_bb.Max.y) }, (ix + iy) % 2 ? c0 : c1);
                        }
                    const ImU32 rgb_col = col(ImVec4(rgb.x, rgb.y, rgb.z, 1.f), popup_anim);
                    const ImU32 trans_col = ImGui::GetColorU32(ImVec4(rgb.x, rgb.y, rgb.z, 0.f));
                    dl->AddRectFilledMultiColor(a_bb.Min, a_bb.Max, trans_col, rgb_col, rgb_col, trans_col);
                    dl->PopClipRect();

                    const float gw = s(5.f), gh = s(18.f);
                    const float cx = ImLerp(a_bb.Min.x + gh * 0.5f, a_bb.Max.x - gh * 0.5f, st.pick_a);
                    dl->AddRectFilled({ cx - gw * 0.5f, a_bb.GetCenter().y - gh * 0.5f },
                                      { cx + gw * 0.5f, a_bb.GetCenter().y + gh * 0.5f }, col(ImVec4(1, 1, 1, 1), popup_anim), s(3.f));
                }

                // Sync HSV back into the stored RGBA
                ImGui::ColorConvertHSVtoRGB(st.pick_h, st.pick_s, st.pick_v, value.x, value.y, value.z);
                value.w = st.pick_a;
                if (st.open_popup == 11) colors::sync_accent_hover();
            }
            // Triggerbot Cog popout contents (Keybind + Mode toggle/hold)
            else if (st.open_popup == 6)
            {
                // Row 1: Keybind listener
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y }, { popup_bb.Max.x, start_y + s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::InvisibleButton("##trigger_keybind_btn", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.binding_trigger_key = !st.binding_trigger_key;

                    if (hovered && !st.binding_trigger_key)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    // Keybind text label
                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, col(colors::text, popup_anim), "Keybind");

                    // Keybind value text (blinks/glows yellow when binding)
                    char val_buf[32];
                    if (st.binding_trigger_key)
                    {
                        sprintf_s(val_buf, sizeof(val_buf), "[Press Key]");
                    }
                    else
                    {
                        sprintf_s(val_buf, sizeof(val_buf), "[%s]", get_key_name(st.trigger_keybind));
                    }
                    const ImVec2 val_sz = bone_f->CalcTextSizeA(bone_px, FLT_MAX, 0.f, val_buf);
                    
                    ImU32 val_col = st.binding_trigger_key ? col(colors::accent, (0.5f + 0.5f * sinf((float)ImGui::GetTime() * 12.f)) * popup_anim) : col(colors::text_secondary, popup_anim);
                    dl->AddText(bone_f, bone_px, { row_bb.Max.x - s(15.f) - val_sz.x, text_y }, val_col, val_buf);
                }

                // Row 2 & 3: Mode hold / toggle
                const char* modes[] = { "Hold", "Toggle" };
                for (int i = 0; i < 2; ++i)
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y + (i + 1) * s(32.f) }, { popup_bb.Max.x, start_y + (i + 2) * s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::PushID(i + 800);
                    ImGui::InvisibleButton("##trigger_mode_row", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.trigger_mode = i;
                    ImGui::PopID();

                    const bool selected = (st.trigger_mode == i);

                    if (hovered && !selected)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    if (selected)
                    {
                        // Selected-effect color: brighter accent tint fill + soft accent outline (no vertical bar)
                        const ImVec2 fill_min = row_bb.Min + ImVec2(s(4.f), s(2.f));
                        const ImVec2 fill_max = row_bb.Max - ImVec2(s(4.f), s(2.f));
                        dl->AddRectFilled(fill_min, fill_max, col(colors::accent, 0.22f * popup_anim), s(5.f));
                    }

                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;
                    
                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y }, 
                                col(selected ? colors::accent : colors::text, popup_anim), modes[i]);

                    if (selected)
                    {
                        ImFont* check_f = fonts::icon(12.f);
                        const float check_px = fonts::design_to_pixel(12.f);
                        const ImVec2 check_sz = check_f->CalcTextSizeA(check_px, FLT_MAX, 0.f, icons::check);
                        
                        dl->AddText(check_f, check_px, { row_bb.Max.x - s(10.f) - check_sz.x, row_bb.Min.y + (row_bb.GetHeight() - check_sz.y) * 0.5f },
                                    col(colors::accent, popup_anim), icons::check);
                    }
                }
            }
            // Box Type selector popout (Corner, Full, 3D)
            else if (st.open_popup == 7)
            {
                const char* box_options[] = { "Corner", "Full", "3D" };
                for (int i = 0; i < 3; ++i)
                {
                    const ImRect row_bb({ popup_bb.Min.x, start_y + i * s(32.f) }, { popup_bb.Max.x, start_y + (i + 1) * s(32.f) });
                    const bool hovered = row_bb.Contains(ImGui::GetIO().MousePos);

                    ImGui::SetCursorScreenPos(row_bb.Min);
                    ImGui::PushID(i + 900);
                    ImGui::InvisibleButton("##box_type_row", row_bb.GetSize());
                    if (ImGui::IsItemClicked())
                        st.esp_box_type = i;
                    ImGui::PopID();

                    const bool selected = (st.esp_box_type == i);

                    if (hovered && !selected)
                    {
                        dl->AddRectFilled(row_bb.Min, row_bb.Max, col(colors::widget_hover, 0.4f * popup_anim), s(4.f));
                    }

                    if (selected)
                    {
                        const ImVec2 fill_min = row_bb.Min + ImVec2(s(4.f), s(2.f));
                        const ImVec2 fill_max = row_bb.Max - ImVec2(s(4.f), s(2.f));
                        dl->AddRectFilled(fill_min, fill_max, col(colors::accent, 0.22f * popup_anim), s(5.f));
                    }

                    ImFont* bone_f = fonts::inter(13.f);
                    const float bone_px = fonts::design_to_pixel(13.f);
                    const float text_y = row_bb.Min.y + (row_bb.GetHeight() - bone_px) * 0.5f;

                    dl->AddText(bone_f, bone_px, { row_bb.Min.x + s(15.f), text_y },
                                col(selected ? colors::accent : colors::text, popup_anim), box_options[i]);

                    if (selected)
                    {
                        ImFont* check_f = fonts::icon(12.f);
                        const float check_px = fonts::design_to_pixel(12.f);
                        const ImVec2 check_sz = check_f->CalcTextSizeA(check_px, FLT_MAX, 0.f, icons::check);

                        dl->AddText(check_f, check_px, { row_bb.Max.x - s(10.f) - check_sz.x, row_bb.Min.y + (row_bb.GetHeight() - check_sz.y) * 0.5f },
                                    col(colors::accent, popup_anim), icons::check);
                    }
                }
            }
        }
    }
}

namespace main_page
{
    void draw(const ImRect& panel, state& st, bool& blocks_window_drag)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        bool sb_block = false;
        draw_sidebar(dl, panel, st, sb_block);
        draw_app_content(dl, panel, st);
        blocks_window_drag = blocks_window_drag || sb_block;

        // Sync UI toggles into g_Options so esp_overlay::frame() renders them
        esp_sync(st);
    }
}
