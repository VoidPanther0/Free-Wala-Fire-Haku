#include <imgui.h>
#include <d3d11.h>
#include <cmath>
#include <algorithm>
#include <Cheat/Options.hpp>
#include <Cheat/Features/Visuals/PlayerEsp.hpp>
#include <Cheat/FreeFireSDK/Context.hpp>
#include <FrameWork/Math/Entity.hpp>
#include <FrameWork/Math/Vectors/Vector2.hpp>
#include <FrameWork/Math/W2S.hpp>
#include <FrameWork/Render/Assets.hpp>
#include <mutex>

using std::min;
using std::max;

bool IsTeam = false;

namespace font
{
    inline ImFont* WeaponsIco = nullptr;

    inline ImFont* icomoon = nullptr;
    inline ImFont* lexend_bold = nullptr;
    inline ImFont* lexend_regular = nullptr;
    inline ImFont* lexend_general_bold = nullptr;

    inline ImFont* icomoon_widget = nullptr;
    inline ImFont* icomoon_widget2 = nullptr;

}

ImColor GetAnimatedRGBColor(float speedMultiplier = 1.0f) {
    float time = ImGui::GetTime() * speedMultiplier;
    float r = (sinf(time + 0.0f) * 0.5f) + 0.5f;
    float g = (sinf(time + 2.0f) * 0.5f) + 0.5f;
    float b = (sinf(time + 4.0f) * 0.5f) + 0.5f;
    return ImColor(r, g, b, 1.0f);
}

void AddGlowEffect(ImDrawList* draw, ImVec2 start, ImVec2 end, ImColor color, float thickness = 1.0f) {
    if (!g_Options.Visuals.ESP.Players.Glow) return;
    
    // Static glow effect - no animation
    float alpha = 0.2f; // Fixed alpha instead of pulsing
    ImColor glowColor = ImColor(color.Value.x, color.Value.y, color.Value.z, alpha);
    
    // Create static glow with multiple layers
    for (int i = 2; i > 0; i--) {
        float layerAlpha = alpha * (0.5f / i); // Static fade
        ImColor layerColor = ImColor(color.Value.x, color.Value.y, color.Value.z, layerAlpha);
        draw->AddLine(start, end, layerColor, thickness + (i * 1.5f));
    }
}

void AddGlowRect(ImDrawList* draw, ImVec2 min, ImVec2 max, ImColor color, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f) {
    // Disabled expanding glow rect animation as requested
    return;
}

void AddGlowText(ImDrawList* draw, ImVec2 pos, ImColor color, const std::string& text) {
    if (!g_Options.Visuals.ESP.Players.Glow) return;
    
    // Static glow effect - no animation
    float alpha = 0.25f; // Fixed alpha instead of pulsing
    ImColor glowColor = ImColor(color.Value.x, color.Value.y, color.Value.z, alpha);
    
    // Create static glow with multiple layers
    for (int i = 2; i > 0; i--) {
        float layerAlpha = alpha * (0.6f / i); // Static fade
        ImColor layerColor = ImColor(color.Value.x, color.Value.y, color.Value.z, layerAlpha);
        for (int dx = -i; dx <= i; dx++) {
            for (int dy = -i; dy <= i; dy++) {
                if (dx != 0 || dy != 0) {
                    draw->AddText(ImVec2(pos.x + dx, pos.y + dy), layerColor, text.c_str());
                }
            }
        }
    }
}

static ID3D11ShaderResourceView* IconWeapon(int index) {
    switch (index) {
        using namespace Assets::WeaponIcons;
    case 0: return AK47;
    case 1: return FIST;
    case 2: return M4A1;
    case 3: return USP;
    case 4: return AWM;
    case 5: return M1014;
    case 6: return AK47;
    case 7: return UMP;
    case 8: return MP5;
    case 9: return DESERT_EAGLE;
    case 10: return G18;
    case 11: return M14;
    case 12: return SCAR;
    case 13: return VSS;
    case 14: return GROZA;
    case 15: return MP40;
    case 16: return PAN;
    case 17: return PARANG;
    case 18: return SKS;
    case 19: return M249;
    case 20: return M1873;
    case 21: return KAR98K;
    case 22: return CROSSBOW;
    case 23: return M79;
    case 24: return FAMAS;
    case 25: return M500;
    case 26: return SVD;
    case 27: return BAT;
    case 28: return XM8;
    case 29: return SPAS12;
    case 30: return M60;
    case 32: return P90;
    case 33: return AN94;
    case 34: return KATANA;
    case 35: return CG15;
    case 36: return RGS50;
    case 37: return GATLING;
    case 39: return PLASMA;
    case 41: return M1887;
    case 43: return THOMPSON;
    case 45: return M82B;
    case 46: return AUG;
    case 47: return PARAFAL;
    case 48: return WOODPECKER;
    case 49: return VECTOR;
    case 50: return MAG_7;
    case 51: return SCYTHE;
    case 54: return KORD;
    case 55: return M1917;
    case 56: return USP;
    case 57: return KINGFISHER;
    case 58: return MINI_UZI;
    case 60: return MP5;
    case 61: return M60;
    case 62: return VSS;
    case 63: return M14;
    case 64: return KAR98K;
    case 65: return AWM;
    case 67: return FAMAS;
    case 70: return GROZA;
    case 71: return M249;
    case 72: return SVD;
    case 73: return G36;
    case 74: return G36;
    case 75: return M24;
    case 78: return HEAL_SNIPER;
    case 80: return M4A1;
    case 81: return M4A1;
    case 82: return M4A1;
    case 86: return CHARGE_BUSTER;
    case 88: return MAC10;
    case 89: return AC80;
    case 93: return HEAL_PISTOL;
    case 98: return CG15;
    case 99: return SHIELD_GUN;
    case 100: return FLAMETHROWER;
    case 119: return M1887;
    case 120: return MP5;
    case 121: return MP5;
    case 122: return M60;
    case 123: return M60;
    case 124: return VSS;
    case 125: return VSS;
    case 126: return M14;
    case 127: return M14;
    case 128: return KAR98K;
    case 129: return KAR98K;
    case 130: return FAMAS;
    case 131: return FAMAS;
    case 149: return FF_KNIFE;
    case 150: return BIZON;
    case 178: return SCAR;
    case 179: return SCAR;
    case 180: return SCAR;
    case 181: return TROGON;
    case 184: return M1014;
    case 185: return M1014;
    case 186: return M1014;
    case 193: return AUG;
    case 194: return AUG;
    case 195: return AUG;
    case 197: return VSK94;
    case 21001: return HEAL_PISTOL;
    case 21002: return M590;
    case 228: return MAC10;
    case 229: return MAC10;
    case 230: return MAC10;
    case 601: return GRENADE;
    case 602: return FLAMETHROWER;
    case 603: return SMOKE_GRENADE;
    case 608: return FLASH_FREEZE;
    case 617: return GLOO_MELTER;
    case 1201: return GLOO_WALL;
    default: return nullptr;
    }
}

void DrawHealthBarVertical(ImDrawList* vList, float x, float y, float w, float h, float healthPercentage, bool isKnocked, float radius = 0.0f, float glowStrength = 6.0f, float border = 0.5f)
{
    healthPercentage = std::clamp(healthPercentage, 0.0f, 1.0f);

    float filledHeight = h * healthPercentage;
    float emptyHeight = h - filledHeight;

    ImVec2 barTopLeft = ImVec2(x, y);
    ImVec2 barBottomRight = ImVec2(x + w, y + h);

    ImVec2 fillTopLeft = ImVec2(x, y + emptyHeight);
    ImVec2 fillBottomRight = ImVec2(x + w, y + h);

    ImColor topColor, bottomColor;

    if (isKnocked) {
        ImVec4 knockCol = g_Options.Visuals.ESP.Players.KnockedEnemiesColor;
        topColor = bottomColor = ImColor(knockCol);
    }
    else if (healthPercentage > 0.75f) {
        topColor = ImColor(34, 112, 18, 255);
        bottomColor = ImColor(51, 160, 27, 255);
    }
    else if (healthPercentage > 0.3f) {
        topColor = ImColor(130, 110, 0, 255);
        bottomColor = ImColor(200, 180, 0, 255);
    }
    else {
        topColor = ImColor(120, 15, 10, 255);
        bottomColor = ImColor(191, 20, 12, 255);
    }

    vList->AddRectFilled(barTopLeft, barBottomRight, IM_COL32(0, 0, 0, 180), radius);

    vList->AddRectFilledMultiColor(
        fillTopLeft, fillBottomRight,
        topColor, topColor, bottomColor, bottomColor
    );
   
    float glowSize = 4.0f; 
    for (int i = 1; i <= 2; ++i) {
        float offset = glowSize * i;
        ImU32 glowCol = ImColor(
            bottomColor.Value.x,
            bottomColor.Value.y,
            bottomColor.Value.z,
            0.05f / i  
        );

        vList->AddRectFilled(
            ImVec2(fillTopLeft.x - offset, fillTopLeft.y - offset),
            ImVec2(fillBottomRight.x + offset, fillBottomRight.y + offset),
            glowCol,
            radius
        );
    }
    vList->AddRect(barTopLeft, barBottomRight, IM_COL32(0, 0, 0, 255), radius, 0, border);
}

inline bool IsOnScreen(const Vector2& screenPos, int screenWidth, int screenHeight) {
    return screenPos.X >= 0 && screenPos.Y >= 0 && screenPos.X <= screenWidth && screenPos.Y <= screenHeight;
}
namespace Cheat {

    uint32_t ColorToUint32(const ImVec4& color) {
        return ImColor(color.x * 255, color.y * 255, color.z * 255, color.w * 255);
    }

    uint32_t LerpColor(uint32_t color1, uint32_t color2, float t)
    {
        uint8_t a1 = (color1 >> 24) & 0xFF;
        uint8_t r1 = (color1 >> 16) & 0xFF;
        uint8_t g1 = (color1 >> 8) & 0xFF;
        uint8_t b1 = color1 & 0xFF;

        uint8_t a2 = (color2 >> 24) & 0xFF;
        uint8_t r2 = (color2 >> 16) & 0xFF;
        uint8_t g2 = (color2 >> 8) & 0xFF;
        uint8_t b2 = color2 & 0xFF;

        uint8_t a = static_cast<uint8_t>(a1 + (a2 - a1) * t);
        uint8_t r = static_cast<uint8_t>(r1 + (r2 - r1) * t);
        uint8_t g = static_cast<uint8_t>(g1 + (g2 - g1) * t);
        uint8_t b = static_cast<uint8_t>(b1 + (b2 - b1) * t);

        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    void RenderRadar(float radarX, float radarY, float radarSize, float range)
    {
        auto draw = ImGui::GetForegroundDrawList();
        ImVec2 radarCenter = ImVec2(radarX + radarSize / 2.0f, radarY + radarSize / 2.0f);
        float halfSize = radarSize / 2.0f;

        draw->AddRectFilled(ImVec2(radarX, radarY), ImVec2(radarX + radarSize, radarY + radarSize), IM_COL32(0, 0, 0, 90), 6.0f);
        draw->AddRect(ImVec2(radarX, radarY), ImVec2(radarX + radarSize, radarY + radarSize), IM_COL32(255, 255, 255, 40), 6.0f);

        Vector3 localPos = Context::LocalMainCamera;
        float yaw = Context::ViewYaw;

        for (auto& [id, entity] : Context::Entities)
        {
            if (!entity.IsKnown || entity.IsDead || entity.Health <= 0)
                continue;

            Vector3 delta = entity.Root - localPos;
            float dist = sqrtf(delta.X * delta.X + delta.Z * delta.Z);
            if (dist > range) continue;

            float angle = atan2f(delta.X, delta.Z) - yaw;
            float radius = (dist / range) * halfSize;

            float x = radarCenter.x + radius * sinf(angle);
            float y = radarCenter.y - radius * cosf(angle);

            ImColor dotColor = IM_COL32(255, 0, 0, 255);
            if (entity.IsTeam == Bool3::True)
                dotColor = IM_COL32(0, 255, 0, 255);
            else if (entity.IsTeam == Bool3::Unknown)
                dotColor = IM_COL32(255, 255, 0, 255);

            float dotSize = entity.IsKnocked ? 2.0f : 3.5f;
            draw->AddCircleFilled(ImVec2(x, y), dotSize, dotColor);
        }
        draw->AddCircleFilled(radarCenter, 3.5f, IM_COL32(0, 255, 0, 255));
    }

    void RenderEspIndicator()
    {
        auto draw = ImGui::GetForegroundDrawList();
        ImVec2 screenCenter = ImVec2(Context::WindowWidth / 2.f, Context::WindowHeight / 2.f);
        float radius = (float)(min(Context::WindowWidth, Context::WindowHeight)) / 2.0f - 50.0f;

        for (auto& [id, entity] : Context::Entities)
        {
            if (!entity.IsKnown || entity.IsDead || entity.Health <= 0)
                continue;

            if (entity.IsTeam == Bool3::True)
                continue;

            Vector2 screenPos = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
            if (screenPos.X >= 0 && screenPos.Y >= 0 &&
                screenPos.X <= Context::WindowWidth &&
                screenPos.Y <= Context::WindowHeight)
                continue;

            Vector3 dir = entity.Head - Context::LocalMainCamera;

            float angle = atan2f(dir.X, dir.Z) - Context::ViewYaw;

            ImVec2 arrowPos = ImVec2(
                screenCenter.x + cosf(angle) * radius,
                screenCenter.y + sinf(angle) * radius
            );

            float size = 14.0f;
            ImVec2 p1 = ImVec2(arrowPos.x + cosf(angle) * size, arrowPos.y + sinf(angle) * size);
            ImVec2 p2 = ImVec2(arrowPos.x + cosf(angle + 2.5f) * size, arrowPos.y + sinf(angle + 2.5f) * size);
            ImVec2 p3 = ImVec2(arrowPos.x + cosf(angle - 2.5f) * size, arrowPos.y + sinf(angle - 2.5f) * size);

            draw->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 0, 0, 255));
        }
    }
   
    void ESP::Players() {
        auto draw = ImGui::GetBackgroundDrawList();
        auto drawForeground = ImGui::GetForegroundDrawList();
        bool brandingDrawn = false;

        auto DrawOutlinedText = [drawForeground](ImDrawList* draw, ImVec2 pos, ImColor color, const std::string& text) {
            ImU32 outline = IM_COL32(0, 0, 0, 255);
            for (int dx = -1; dx <= 1; ++dx)
                for (int dy = -1; dy <= 1; ++dy)
                    if (dx != 0 || dy != 0)
                        draw->AddText(ImVec2(pos.x + dx, pos.y + dy), outline, text.c_str());
            draw->AddText(pos, color, text.c_str());
            
            // Remove glow effect from text - no more glow on names
            // AddGlowText(drawForeground, pos, color, text);
            };

        auto DrawOutlinedTextWithFont = [](ImDrawList* draw, ImFont* font, float fontSize, ImVec2 pos, ImColor color, const std::string& text) {
            ImFont* safeFont = font ? font : ImGui::GetFont();
            float safeFontSize = fontSize > 0.0f ? fontSize : ImGui::GetFontSize();
            ImU32 outline = IM_COL32(0, 0, 0, 255);

            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (dx != 0 || dy != 0)
                    {
                        draw->AddText(safeFont, safeFontSize, ImVec2(pos.x + dx, pos.y + dy), outline, text.c_str());
                    }
                }
            }

            draw->AddText(safeFont, safeFontSize, pos, color, text.c_str());
        };

        for (auto& [entityID, entity] : Context::Entities) {
            if (!entity.IsKnown || entity.IsDead) continue;

            ImColor rainbow = GetAnimatedRGBColor(1.0f);
            if (g_Options.Visuals.ESP.Players.IgnoreTrainingBots)
            {
                if (entity.Name.empty() || entity.Name.find("Training Doll") != std::string::npos)
                    continue;
            }

            float dist = Vector3::Distance(Context::LocalMainCamera, entity.Head);
            if (dist > g_Options.Visuals.ESP.Players.RenderDistance) continue;

            Vector2 headPos = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
            Vector2 rootPos = W2S::WorldToScreen(Context::ViewMatrix, entity.Root, Context::WindowWidth, Context::WindowHeight);

            if (headPos.X < 0 || rootPos.X < 0 || rootPos.X < 0) continue;

            float rawBoxHeight = fabsf(headPos.Y - rootPos.Y);
            float distSafe = (dist > 0.1f) ? dist : 1.0f;
            float distanceBoxHeight = (Context::WindowHeight * 1.75f) / distSafe;
            float boxHeight = (rawBoxHeight > distanceBoxHeight * 1.8f || rawBoxHeight < distanceBoxHeight * 0.4f) ? distanceBoxHeight : rawBoxHeight;
            float boxWidth = boxHeight * 0.65f;
            float x = headPos.X - (boxWidth / 2), y = headPos.Y - 11;
            float w = boxWidth, h = boxHeight + 17;

            ImColor currecntlinecolor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.SnapLinesColor)));

            ImColor currentBoxColor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.BoxColor)));

            ImColor currentSkeletonColor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.SkeletonColor)));

            ImColor currentNameColor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.NameColor)));

            ImColor currentDistanceColor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.DistColor)));

            ImColor currenthealthcolor = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.DistColor)));

            ImColor weaponcolorxd = g_Options.Visuals.ESP.Players.RainbowESP
                ? rainbow
                : (entity.IsKnocked ? ImColor((g_Options.Visuals.ESP.Players.KnockedEnemiesColor)) : ImColor((g_Options.Visuals.ESP.Players.WeaponColor)));

            

            if (g_Options.Visuals.ESP.Players.Lines) {
                ImVec2 startPoint;

                switch (g_Options.Visuals.ESP.Players.EspLines) {
                case 0: // Top
                    startPoint = ImVec2(Context::WindowWidth / 2, 60);
                    break;
                case 1: // Center  
                    startPoint = ImVec2(Context::WindowWidth / 2, Context::WindowHeight / 2);
                    break;
                case 2: // Bottom
                    startPoint = ImVec2(Context::WindowWidth / 2, Context::WindowHeight);
                    break;
                }

                // Line draw karein
                draw->AddLine(startPoint, ImVec2(headPos.X, headPos.Y - 11), currecntlinecolor, 1.0f);
                
                // Add glow effect to lines (reduced intensity)
                AddGlowEffect(draw, startPoint, ImVec2(headPos.X, headPos.Y - 11), currecntlinecolor, 1.0f);
        
                if (g_Options.Visuals.ESP.Players.ShinyEffects)
                {
                 
                }

                switch (g_Options.Visuals.ESP.Players.TopPosStyles)
                {
                case 0:
                    break;
                case 1:
                   
                    break;
                }

            }

            if (g_Options.Visuals.ESP.Players.SnapLines)
            {
                if (!g_Options.Visuals.ESP.Players.SnapLines)
                    return;

                Entity* closest = nullptr;
                float closestDist = FLT_MAX;

                for (auto& [id, entity] : Context::Entities)
                {
                    if (!entity.IsKnown || entity.IsDead)
                        continue;

                    if (g_Options.LegitBot.AimBot.IgnoreDowned && entity.IsKnocked)
                        continue;

                    float dist = Vector3::Distance(Context::LocalMainCamera, entity.Head);
                    if (dist < closestDist && dist < 20.0f)
                    {
                        Vector2 screenPos = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                        if (screenPos.X >= 0 && screenPos.Y >= 0)
                        {
                            closestDist = dist;
                            closest = &entity;
                        }
                    }
                }

                if (closest)
                {
                    Vector2 screenPos = W2S::WorldToScreen(Context::ViewMatrix, closest->Head, Context::WindowWidth, Context::WindowHeight);
                    if (screenPos.X >= 0 && screenPos.Y >= 0)
                    {
                        ImVec2 center = ImVec2(Context::WindowWidth / 2.0f, Context::WindowHeight / 2.0f);
                        ImColor lineColor = closest->IsKnocked ? ImColor(192, 0, 0, 255) : ImColor((g_Options.Visuals.ESP.Players.OriginLineColor));
                        ImGui::GetBackgroundDrawList()->AddLine(center, ImVec2(screenPos.X, screenPos.Y), lineColor, 1.0f);
                    }
                }
            }

            if (g_Options.Visuals.ESP.Players.Box) {
                auto vList = ImGui::GetForegroundDrawList();

                switch (g_Options.Visuals.ESP.Players.players_box) {
                case 0:
                {
                    ImVec2 boxStart = ImVec2(x, y);
                    ImVec2 boxEnd = ImVec2(x + w, y + h);

                    vList->AddRect(boxStart, boxEnd, currentBoxColor, 1.5f, 0, 1.0f);
                    
                    // Add glow effect to boxes
                    AddGlowRect(vList, boxStart, boxEnd, currentBoxColor, 1.5f, 0, 1.0f);


                    if (g_Options.Visuals.ESP.Players.ShinyEffects) {
                      
                    }
                    break;
                }

                case 1:
                {
                    float XBox = headPos.X - ((fabsf(headPos.Y - rootPos.Y) * 0.62f) / 2.0f);
                    float YBox = headPos.Y;
                    float WBox = fabsf(headPos.Y - rootPos.Y) * 0.60f;
                    float HBox = fabsf(headPos.Y - rootPos.Y);
                    float thickness = 1.0f;

                    float lineW = WBox / 3;
                    float lineH = HBox / 3;


                    vList->AddLine(ImVec2(XBox, YBox), ImVec2(XBox, YBox + lineH), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox, YBox), ImVec2(XBox + lineW, YBox), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox + WBox - lineW, YBox), ImVec2(XBox + WBox, YBox), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox + WBox, YBox), ImVec2(XBox + WBox, YBox + lineH), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox, YBox + HBox - lineH), ImVec2(XBox, YBox + HBox), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox, YBox + HBox), ImVec2(XBox + lineW, YBox + HBox), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox + WBox - lineW, YBox + HBox), ImVec2(XBox + WBox, YBox + HBox), currentBoxColor, thickness);
                    vList->AddLine(ImVec2(XBox + WBox, YBox + HBox - lineH), ImVec2(XBox + WBox, YBox + HBox), currentBoxColor, thickness);
                    
                    if (g_Options.Visuals.ESP.Players.ShinyEffects) {
                        ImVec2 cornerLines[8][2] = {
                            { ImVec2(XBox, YBox), ImVec2(XBox, YBox + lineH) },
                            { ImVec2(XBox, YBox), ImVec2(XBox + lineW, YBox) },
                            { ImVec2(XBox + WBox - lineW, YBox), ImVec2(XBox + WBox, YBox) },
                            { ImVec2(XBox + WBox, YBox), ImVec2(XBox + WBox, YBox + lineH) },
                            { ImVec2(XBox, YBox + HBox - lineH), ImVec2(XBox, YBox + HBox) },
                            { ImVec2(XBox, YBox + HBox), ImVec2(XBox + lineW, YBox + HBox) },
                            { ImVec2(XBox + WBox - lineW, YBox + HBox), ImVec2(XBox + WBox, YBox + HBox) },
                            { ImVec2(XBox + WBox, YBox + HBox - lineH), ImVec2(XBox + WBox, YBox + HBox) }
                        };

                        for (int i = 0; i < 8; ++i) {
                        
                        }
                    }
                    break;
                }
                }
            }

            if (g_Options.Visuals.ESP.Players.fillBox) {

                ImVec4 color = (g_Options.Visuals.ESP.Players.fillBoxColor);

                ImU32 fillColor = IM_COL32(
                    color.x * 255,
                    color.y * 255,
                    color.z * 255,
                    50
                );


                float fx = x;
                float fy = y;
                float fw = w;
                float fh = h;


                if (g_Options.Visuals.ESP.Players.players_box == 1) {
                    fx = headPos.X - ((fabsf(headPos.Y - rootPos.Y) * 0.62f) / 2.0f);
                    fy = headPos.Y;
                    fw = fabsf(headPos.Y - rootPos.Y) * 0.60f;
                    fh = fabsf(headPos.Y - rootPos.Y);
                }

                draw->AddRectFilled(
                    ImVec2(fx, fy),
                    ImVec2(fx + fw, fy + fh),
                    fillColor,
                    1.5f
                );
            }

            if (g_Options.Visuals.ESP.Players.HealthBar) {
                float barX = 0, barY = 0;
                float barWidth = 0, barHeight = 0;

                float boxX = x, boxY = y;
                float boxW = w, boxH = h;

                if (g_Options.Visuals.ESP.Players.players_box == 1) {
                    boxX = headPos.X - ((fabsf(headPos.Y - rootPos.Y) * 0.62f) / 2.0f);
                    boxY = headPos.Y;
                    boxW = fabsf(headPos.Y - rootPos.Y) * 0.60f;
                    boxH = fabsf(headPos.Y - rootPos.Y);
                }

                switch (g_Options.Visuals.ESP.Players.HealthBarPosition) {
                default: // Default = Right
                case 2:
                    barWidth = 4.0f;
                    barHeight = boxH;
                    barX = boxX + boxW + 4.0f;
                    barY = boxY;
                    break;
                case 3: // Left
                    barWidth = 4.0f;
                    barHeight = boxH;
                    barX = boxX - barWidth - 4.0f;
                    barY = boxY;
                    break;
                case 0: // Top
                    barWidth = boxW;
                    barHeight = 4.0f;
                    barX = boxX;
                    barY = boxY - barHeight - 4.0f;
                    break;
                case 1: // Bottom
                    barWidth = boxW;
                    barHeight = 4.0f;
                    barX = boxX;
                    barY = boxY + boxH + 4.0f;
                    break;
                }

                float healthPerc = std::clamp(entity.Health / 200.0f, 0.0f, 1.0f);
                DrawHealthBarVertical(drawForeground, barX, barY, barWidth, barHeight, healthPerc, entity.IsKnocked);
            }

            if (g_Options.Visuals.ESP.Players.WeaponInfo)
            {
                auto* draw = ImGui::GetForegroundDrawList();

                const float healthBarHeight = 6.0f;
                const float healthBarGap = 1.0f;
                const float iconGapBelowHealth = 2.0f;
                const float iconHeight = 14.0f;
                const float iconWidth = 36.0f;
                const float textGapBelowIcon = 2.0f;

                const float upwardOffset = 4.0f;

                float baseY = y + h + healthBarGap + healthBarHeight + iconGapBelowHealth - upwardOffset;

                if (g_Options.Visuals.ESP.Players.HealthBar &&
                    g_Options.Visuals.ESP.Players.HealthBarPosition == 1)
                {
                    baseY += 10.0f;
                }

                if (g_Options.Visuals.ESP.Players.WeaponIcon)
                {
                    ID3D11ShaderResourceView* weaponIcon = IconWeapon(entity.WeaponIndex);
                    if (weaponIcon)
                    {
                        ImVec2 iconSize(iconWidth, iconHeight);
                        ImVec2 iconPos(headPos.X - iconSize.x / 2.0f, baseY);

                        ImU32 outline = IM_COL32(0, 0, 0, 180);

                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            for (int dy = -1; dy <= 1; ++dy)
                            {
                                if (dx != 0 || dy != 0)
                                {
                                    draw->AddImage(
                                        (ImTextureID)weaponIcon,
                                        ImVec2(iconPos.x + dx, iconPos.y + dy),
                                        ImVec2(iconPos.x + iconSize.x + dx, iconPos.y + iconSize.y + dy),
                                        ImVec2(0, 0), ImVec2(1, 1),
                                        outline
                                    );
                                }
                            }
                        }

                        draw->AddImage(
                            (ImTextureID)weaponIcon,
                            iconPos,
                            ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y),
                            ImVec2(0, 0), ImVec2(1, 1),
                            weaponcolorxd
                        );

                        baseY += iconSize.y + textGapBelowIcon;
                    }
                }


                if (g_Options.Visuals.ESP.Players.WeaponName)
                {
                    std::string name = entity.WeaponName.empty() ? "AK47" : entity.WeaponName;
                    ImFont* weaponFont = Assets::InterExtraBold ? Assets::InterExtraBold : ImGui::GetFont();
                    float weaponFontSize = weaponFont ? weaponFont->FontSize : ImGui::GetFontSize();
                    ImVec2 textSize = weaponFont->CalcTextSizeA(weaponFontSize, FLT_MAX, 0.0f, name.c_str());
                    ImVec2 textPos(headPos.X - textSize.x / 2.0f, baseY);

                    DrawOutlinedTextWithFont(draw, weaponFont, weaponFontSize, textPos, weaponcolorxd, name);
                }
            }

            if (g_Options.Visuals.ESP.Players.Name || g_Options.Visuals.ESP.Players.Distance || g_Options.Visuals.ESP.Players.Uid)
            {
                std::string distText = std::to_string((int)dist) + " m";
                std::string nameText = entity.Name;
                std::string uidText = "UID: " + std::to_string(entity.UserID);

                float spacing = 2.0f;
                float verticalOffset = 20.0f; // Increased from 12.0f to move names up higher

                ImFont* nameFont = Assets::InterExtraBold; // Use bold font for names
                ImFont* distFont = Assets::InterSemiBold;   // Use slightly smaller font for distance
                ImFont* uidFont = Assets::InterExtraBold; // Use same font as player name for UID
                ImGui::PushFont(nameFont); // Push bold font for name text

                ImVec2 distSize = ImGui::CalcTextSize(distText.c_str());
                ImVec2 nameSize = ImGui::CalcTextSize(nameText.c_str());
                ImVec2 uidSize = ImGui::CalcTextSize(uidText.c_str());

                ImVec2 distPos, namePos, uidPos;

                float offsetTop = 0.0f;

                if (g_Options.Visuals.ESP.Players.HealthBar &&
                    g_Options.Visuals.ESP.Players.HealthBarPosition == 0)
                {
                    offsetTop = 15.0f; // Increased offset
                }

                if (g_Options.Visuals.ESP.Players.Name && g_Options.Visuals.ESP.Players.Distance && g_Options.Visuals.ESP.Players.Uid)
                {
                    namePos = ImVec2(
                        headPos.X - nameSize.x / 2.0f, // Center the name better
                        headPos.Y - nameSize.y - distSize.y - uidSize.y - spacing * 6.0f - verticalOffset - offsetTop - 10.0f // Move up by extra 10.0f
                    );

                    distPos = ImVec2(
                        headPos.X - distSize.x / 2.0f,
                        namePos.y + nameSize.y + spacing
                    );

                    uidPos = ImVec2(
                        headPos.X - uidSize.x / 2.0f,
                        distPos.y + distSize.y + spacing
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Name && g_Options.Visuals.ESP.Players.Distance)
                {
                    namePos = ImVec2(
                        headPos.X - nameSize.x / 2.0f, // Center the name better
                        headPos.Y - nameSize.y - distSize.y - spacing * 4.0f - verticalOffset - offsetTop - 10.0f // Move up by extra 10.0f
                    );

                    distPos = ImVec2(
                        headPos.X - distSize.x / 2.0f,
                        namePos.y + nameSize.y + spacing
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Name && g_Options.Visuals.ESP.Players.Uid)
                {
                    namePos = ImVec2(
                        headPos.X - nameSize.x / 2.0f, // Center the name better
                        headPos.Y - nameSize.y - uidSize.y - spacing * 4.0f - verticalOffset - offsetTop - 10.0f // Move up by extra 10.0f
                    );

                    uidPos = ImVec2(
                        headPos.X - uidSize.x / 2.0f,
                        namePos.y + nameSize.y + spacing
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Distance && g_Options.Visuals.ESP.Players.Uid)
                {
                    distPos = ImVec2(
                        headPos.X - distSize.x / 2.0f,
                        headPos.Y - distSize.y - uidSize.y - spacing * 4.0f - verticalOffset - offsetTop
                    );

                    uidPos = ImVec2(
                        headPos.X - uidSize.x / 2.0f,
                        distPos.y + distSize.y + spacing
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Name)
                {
                    namePos = ImVec2(
                        headPos.X - nameSize.x / 2.0f, // Center the name better
                        headPos.Y - nameSize.y - verticalOffset - offsetTop - 10.0f // Move up by extra 10.0f
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Distance)
                {
                    distPos = ImVec2(
                        headPos.X - distSize.x / 2.0f,
                        headPos.Y - distSize.y - verticalOffset - offsetTop
                    );
                }
                else if (g_Options.Visuals.ESP.Players.Uid)
                {
                    uidPos = ImVec2(
                        headPos.X - uidSize.x / 2.0f,
                        headPos.Y - uidSize.y - verticalOffset - offsetTop
                    );
                }

                if (g_Options.Visuals.ESP.Players.Name)
                    DrawOutlinedText(drawForeground, namePos, currentNameColor, nameText);

                if (g_Options.Visuals.ESP.Players.Distance) {
                    ImGui::PopFont(); // Pop name font
                    ImGui::PushFont(distFont); // Push distance font
                    DrawOutlinedText(drawForeground, distPos, currentDistanceColor, distText);
                }

                // Draw UID with different color
                if (g_Options.Visuals.ESP.Players.Uid) {
                    ImGui::PopFont(); // Pop distance font if pushed
                    ImGui::PushFont(uidFont); // Push UID font
                    ImColor uidColor = IM_COL32(255, 255, 255, 255); // White color for UID
                    DrawOutlinedText(drawForeground, uidPos, uidColor, uidText);
                }

                ImGui::PopFont(); // Pop final font
            }

            if (g_Options.Visuals.ESP.Players.Skeleton) {
                auto draw = ImGui::GetBackgroundDrawList();
                auto DrawBone = [&](const Vector3& from, const Vector3& to) {
                    Vector2 screenFrom = W2S::WorldToScreen(Context::ViewMatrix, from, Context::WindowWidth, Context::WindowHeight);
                    Vector2 screenTo = W2S::WorldToScreen(Context::ViewMatrix, to, Context::WindowWidth, Context::WindowHeight);
                    if (screenFrom.X >= 0 && screenTo.X >= 0) {
                        draw->AddLine(ImVec2(screenFrom.X, screenFrom.Y), ImVec2(screenTo.X, screenTo.Y), currentSkeletonColor, 1.0f);
                    }

                    if (g_Options.Visuals.ESP.Players.ShinyEffects) {
                       
                    }
                    };
                DrawBone(entity.Head, entity.Neck);
                DrawBone(entity.Neck, entity.Hip);
                DrawBone(entity.Neck, entity.ShoulderLeft);
                DrawBone(entity.ShoulderLeft, entity.ElbowLeft);
                DrawBone(entity.ElbowLeft, entity.WristLeft);
                DrawBone(entity.Neck, entity.ShoulderRight);
                DrawBone(entity.ShoulderRight, entity.ElbowRight);
                DrawBone(entity.ElbowRight, entity.WristRight);
                DrawBone(entity.Hip, entity.HipCenter);
                DrawBone(entity.HipCenter, entity.AnkleLeft);
                DrawBone(entity.HipCenter, entity.AnkleRight);
                DrawBone(entity.AnkleLeft, entity.ToeLeft);
                DrawBone(entity.AnkleRight, entity.ToeRight);
            }
        }
    }
}
