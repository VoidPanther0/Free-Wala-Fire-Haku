#include "login_page.h"

#include "../helpers/fonts.h"
#include "../helpers/icons.h"
#include "../theme/colors.h"
#include "../theme/layout.h"
#include "../theme/ui_scale.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <string>

namespace
{
    using ui_scale::s;

    ImU32 col(const ImVec4& c, float a = 1.f)
    {
        return ImGui::GetColorU32(ImVec4(c.x, c.y, c.z, c.w * a));
    }

    ImVec4 lerp_color(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
    }
}

namespace login_page
{
    void draw(const ImRect& panel, state& st, bool& blocks_window_drag)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float dt = ImGui::GetIO().DeltaTime;
        const float alpha = st.page_alpha;
        if (alpha <= 0.001f)
            return;

        const float round_inner = s(8.f);

        // Content padding inside the panel (NO SIDEBAR TABS AT ALL)
        const float pad_x = s(36.f);
        const float content_w = panel.GetWidth() - pad_x * 2.f;
        const float start_x = panel.Min.x + pad_x;

        float cur_y = panel.Min.y + s(44.f);

        // --- Top Power / Exit Button ---
        {
            const float ex_size = s(34.f);
            const ImRect exit_bb({ panel.Max.x - pad_x - ex_size, panel.Min.y + s(38.f) },
                                 { panel.Max.x - pad_x, panel.Min.y + s(38.f) + ex_size });

            ImGui::PushID("##login_exit_btn");
            ImGui::SetCursorScreenPos(exit_bb.Min);
            ImGui::InvisibleButton("##login_exit", exit_bb.GetSize());
            const bool ex_hover = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                st.exit_pressed = true;
            ImGui::PopID();

            static float ex_anim = 0.f;
            ex_anim += ((ex_hover ? 1.f : 0.f) - ex_anim) * dt * 14.f;

            ImVec4 bg_col = lerp_color(colors::topbar, colors::danger, ex_anim);
            dl->AddRectFilled(exit_bb.Min, exit_bb.Max, col(bg_col, alpha), s(6.f));

            ImFont* ex_f = fonts::icon(14.f);
            const float ex_px = fonts::design_to_pixel(14.f);
            const ImVec2 ex_sz = ex_f->CalcTextSizeA(ex_px, FLT_MAX, 0.f, icons::power);
            dl->AddText(ex_f, ex_px,
                        { exit_bb.GetCenter().x - ex_sz.x * 0.5f, exit_bb.GetCenter().y - ex_sz.y * 0.5f - s(0.5f) },
                        col(colors::text, alpha), icons::power);
        }

        // --- Brand Header (Glowing Diamond Emblem + App Title) ---
        {
            const ImVec2 emblem_center = { panel.GetCenter().x, cur_y + s(18.f) };
            const float r = s(16.f);

            // Ambient yellow pulse glow
            static float glow_t = 0.f;
            glow_t += dt * 3.f;
            const float pulse = (sinf(glow_t) * 0.5f + 0.5f) * 0.35f + 0.65f;

            dl->AddCircleFilled(emblem_center, r * 1.5f, col(colors::accent, 0.15f * pulse * alpha));

            // Diamond Points
            const ImVec2 pts[4] = {
                { emblem_center.x, emblem_center.y - r },
                { emblem_center.x + r * 0.95f, emblem_center.y },
                { emblem_center.x, emblem_center.y + r },
                { emblem_center.x - r * 0.95f, emblem_center.y }
            };
            dl->AddQuadFilled(pts[0], pts[1], pts[2], pts[3], col(colors::accent, alpha));

            // Inner dark core notch
            const float ir = r * 0.42f;
            const ImVec2 ipts[4] = {
                { emblem_center.x, emblem_center.y - ir },
                { emblem_center.x + ir * 0.95f, emblem_center.y },
                { emblem_center.x, emblem_center.y + ir },
                { emblem_center.x - ir * 0.95f, emblem_center.y }
            };
            dl->AddQuadFilled(ipts[0], ipts[1], ipts[2], ipts[3], col(colors::bg, alpha));

            cur_y += s(44.f);

            // Title "SACHIN"
            ImFont* title_f = fonts::logo(24.f);
            const float title_px = fonts::design_to_pixel(24.f);
            const char* title_str = "SACHIN";
            const ImVec2 title_sz = title_f->CalcTextSizeA(title_px, FLT_MAX, 0.f, title_str);
            dl->AddText(title_f, title_px,
                        { panel.GetCenter().x - title_sz.x * 0.5f, cur_y },
                        col(colors::text, alpha), title_str);

            cur_y += title_sz.y + s(5.f);

            // Subtitle
            ImFont* sub_f = fonts::inter(12.f);
            const float sub_px = fonts::design_to_pixel(12.f);
            const char* sub_str = "Sign in to access your dashboard";
            const ImVec2 sub_sz = sub_f->CalcTextSizeA(sub_px, FLT_MAX, 0.f, sub_str);
            dl->AddText(sub_f, sub_px,
                        { panel.GetCenter().x - sub_sz.x * 0.5f, cur_y },
                        col(colors::text_secondary, alpha), sub_str);

            cur_y += sub_sz.y + s(28.f);
        }

        // --- Divider Line ---
        dl->AddLine({ start_x, cur_y }, { start_x + content_w, cur_y }, col(colors::divider, alpha), s(1.0f));
        cur_y += s(22.f);

        // --- Helper for input boxes ---
        auto draw_form_input = [&](const char* label_text, const char* icon_glyph, char* buffer, size_t buf_sz,
                                   bool is_pwd, bool& show_pwd, const char* placeholder, const char* input_id)
        {
            // Label
            ImFont* lf = fonts::inter(12.f, true);
            const float l_px = fonts::design_to_pixel(12.f);
            dl->AddText(lf, l_px, { start_x, cur_y }, col(colors::text, alpha), label_text);
            cur_y += l_px + s(7.f);

            const float box_h = s(44.f);
            const ImRect box_bb({ start_x, cur_y }, { start_x + content_w, cur_y + box_h });

            const ImGuiID id = ImGui::GetID(input_id);
            const bool focused = (ImGui::GetActiveID() == id);

            // Box background (#18181C)
            dl->AddRectFilled(box_bb.Min, box_bb.Max, col(colors::topbar, alpha), round_inner);
            if (focused)
                dl->AddRect(box_bb.Min, box_bb.Max, col(colors::accent, 0.85f * alpha), round_inner, 0, s(1.2f));
            else
                dl->AddRect(box_bb.Min, box_bb.Max, col(colors::divider, alpha), round_inner, 0, s(1.0f));

            // Leading icon
            ImFont* ic_f = fonts::icon(14.f);
            const float ic_px = fonts::design_to_pixel(14.f);
            const ImVec2 ic_sz = ic_f->CalcTextSizeA(ic_px, FLT_MAX, 0.f, icon_glyph);
            dl->AddText(ic_f, ic_px,
                        { box_bb.Min.x + s(14.f), box_bb.GetCenter().y - ic_sz.y * 0.5f },
                        col(focused ? colors::accent : colors::text_secondary, alpha), icon_glyph);

            // Trailing eye button for password
            float reserve_r = s(14.f);
            if (is_pwd)
            {
                reserve_r = s(38.f);
                const ImRect eye_bb({ box_bb.Max.x - s(36.f), box_bb.Min.y + s(6.f) },
                                    { box_bb.Max.x - s(6.f), box_bb.Max.y - s(6.f) });

                ImGui::PushID((std::string(input_id) + "_eye").c_str());
                ImGui::SetCursorScreenPos(eye_bb.Min);
                ImGui::InvisibleButton("##eye_btn", eye_bb.GetSize());
                const bool eye_hov = ImGui::IsItemHovered();
                if (ImGui::IsItemClicked())
                    show_pwd = !show_pwd;
                ImGui::PopID();

                const char* eye_glyph = show_pwd ? icons::eye_off : icons::eye;
                ImFont* eye_f = fonts::icon(13.f);
                const float eye_px = fonts::design_to_pixel(13.f);
                const ImVec2 eye_sz = eye_f->CalcTextSizeA(eye_px, FLT_MAX, 0.f, eye_glyph);
                dl->AddText(eye_f, eye_px,
                            { eye_bb.GetCenter().x - eye_sz.x * 0.5f, eye_bb.GetCenter().y - eye_sz.y * 0.5f },
                            col(eye_hov ? colors::accent : colors::text_secondary, alpha), eye_glyph);
            }

            // Placeholder
            if (buffer[0] == '\0' && !focused)
            {
                ImFont* ph_f = fonts::inter(12.5f);
                const float ph_px = fonts::design_to_pixel(12.5f);
                dl->AddText(ph_f, ph_px,
                            { box_bb.Min.x + s(40.f), box_bb.GetCenter().y - ph_px * 0.5f },
                            col(colors::text_muted, 0.7f * alpha), placeholder);
            }

            // InputText
            ImGuiInputTextFlags flags = (is_pwd && !show_pwd) ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None;

            ImGui::PushFont(fonts::inter(13.f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colors::text.x, colors::text.y, colors::text.z, alpha));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s(40.f), s(11.f)));

            ImGui::SetCursorScreenPos(box_bb.Min);
            ImGui::PushItemWidth(content_w - reserve_r);
            ImGui::InputText(input_id, buffer, (int)buf_sz, flags);
            ImGui::PopItemWidth();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            ImGui::PopFont();

            cur_y += box_h + s(14.f);
        };

        // 1. Username Field
        bool dummy_eye = false;
        draw_form_input("Username", icons::user_secret, st.username, sizeof(st.username),
                        false, dummy_eye, "Enter your username", "##form_usr");

        // 2. Password Field
        draw_form_input("Password", icons::lock_5, st.password, sizeof(st.password),
                        true, st.show_password, "••••••••••••", "##form_pwd");

        // --- Remember Me Checkbox Row ---
        {
            const float cb_sz = s(18.f);
            const ImRect cb_bb({ start_x, cur_y }, { start_x + cb_sz, cur_y + cb_sz });

            ImGui::PushID("##form_rmb_btn");
            ImGui::SetCursorScreenPos({ start_x, cur_y });
            ImGui::InvisibleButton("##rmb", { content_w, cb_sz });
            if (ImGui::IsItemClicked())
                st.remember_me = !st.remember_me;
            const bool cb_hov = ImGui::IsItemHovered();
            ImGui::PopID();

            dl->AddRectFilled(cb_bb.Min, cb_bb.Max,
                              col(st.remember_me ? colors::accent : colors::control, alpha), s(4.f));

            if (st.remember_me)
            {
                ImFont* chk_f = fonts::icon(11.f);
                const float chk_px = fonts::design_to_pixel(11.f);
                const ImVec2 chk_sz = chk_f->CalcTextSizeA(chk_px, FLT_MAX, 0.f, icons::check);
                dl->AddText(chk_f, chk_px,
                            { cb_bb.GetCenter().x - chk_sz.x * 0.5f, cb_bb.GetCenter().y - chk_sz.y * 0.5f - s(0.5f) },
                            col(colors::bg, alpha), icons::check);
            }

            ImFont* rmb_f = fonts::inter(12.f);
            const float rmb_px = fonts::design_to_pixel(12.f);
            dl->AddText(rmb_f, rmb_px,
                        { cb_bb.Max.x + s(10.f), cb_bb.GetCenter().y - rmb_px * 0.5f },
                        col(cb_hov ? colors::text : colors::text_secondary, alpha), "Remember session credentials");

            cur_y += cb_sz + s(24.f);
        }

        // --- Error Message (if any) ---
        if (st.status_text[0] != '\0')
        {
            ImFont* err_f = fonts::inter(11.5f, true);
            const float err_px = fonts::design_to_pixel(11.5f);
            const ImVec2 err_sz = err_f->CalcTextSizeA(err_px, FLT_MAX, 0.f, st.status_text);
            dl->AddText(err_f, err_px,
                        { panel.GetCenter().x - err_sz.x * 0.5f, cur_y - s(6.f) },
                        col(colors::danger, alpha), st.status_text);
            cur_y += err_sz.y + s(6.f);
        }

        // --- 3. LOGIN / CONTINUE BUTTON ---
        {
            const float btn_h = s(46.f);
            const ImRect btn_bb({ start_x, cur_y }, { start_x + content_w, cur_y + btn_h });

            ImGui::PushID("##form_login_btn");
            ImGui::SetCursorScreenPos(btn_bb.Min);
            ImGui::InvisibleButton("##login_act", btn_bb.GetSize());
            const bool hov = ImGui::IsItemHovered();
            const bool act = ImGui::IsItemActive();
            if (ImGui::IsItemClicked())
            {
                if (st.username[0] == '\0')
                {
                    strncpy_s(st.status_text, "Please enter your username", sizeof(st.status_text));
                }
                else
                {
                    st.status_text[0] = '\0';
                    st.is_logged_in = true;
                }
            }
            ImGui::PopID();

            static float btn_anim = 0.f;
            btn_anim += ((hov ? 1.f : 0.f) - btn_anim) * dt * 14.f;

            ImVec4 btn_col = lerp_color(colors::accent, colors::accent_hover, btn_anim);
            if (act)
                btn_col = lerp_color(btn_col, ImVec4(0.85f, 0.65f, 0.0f, 1.f), 0.5f);

            dl->AddRectFilled(btn_bb.Min, btn_bb.Max, col(btn_col, alpha), round_inner);

            // Subtle glow on hover
            if (btn_anim > 0.01f)
            {
                dl->AddRect(btn_bb.Min - ImVec2(s(1.f), s(1.f)),
                            btn_bb.Max + ImVec2(s(1.f), s(1.f)),
                            col(colors::accent_hover, btn_anim * 0.45f * alpha),
                            round_inner + s(1.f), 0, s(1.5f));
            }

            // Centered button label with arrow
            ImFont* btn_f = fonts::inter(13.5f, true);
            const float btn_px = fonts::design_to_pixel(13.5f);
            const char* btn_txt = "CONTINUE TO DASHBOARD";
            const ImVec2 txt_sz = btn_f->CalcTextSizeA(btn_px, FLT_MAX, 0.f, btn_txt);

            ImFont* arr_f = fonts::icon(12.f);
            const float arr_px = fonts::design_to_pixel(12.f);
            const ImVec2 arr_sz = arr_f->CalcTextSizeA(arr_px, FLT_MAX, 0.f, icons::chevron_right);

            const float total_w = txt_sz.x + arr_sz.x + s(8.f);
            const ImVec2 txt_pos = {
                btn_bb.GetCenter().x - total_w * 0.5f,
                btn_bb.GetCenter().y - txt_sz.y * 0.5f
            };
            dl->AddText(btn_f, btn_px, txt_pos, col(colors::bg, alpha), btn_txt);

            const float arr_x = txt_pos.x + txt_sz.x + s(8.f) + btn_anim * s(3.f);
            dl->AddText(arr_f, arr_px,
                        { arr_x, btn_bb.GetCenter().y - arr_sz.y * 0.5f },
                        col(colors::bg, alpha), icons::chevron_right);

            cur_y += btn_h + s(20.f);
        }

        // --- Footer ---
        {
            ImFont* ft_f = fonts::inter(11.f);
            const float ft_px = fonts::design_to_pixel(11.f);
            const char* ft_text = "Sachin Security • Hardware Authorized";
            const ImVec2 ft_sz = ft_f->CalcTextSizeA(ft_px, FLT_MAX, 0.f, ft_text);
            dl->AddText(ft_f, ft_px,
                        { panel.GetCenter().x - ft_sz.x * 0.5f, panel.Max.y - s(24.f) },
                        col(colors::text_muted, 0.6f * alpha), ft_text);
        }
    }
}
