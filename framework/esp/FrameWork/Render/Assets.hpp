#pragma once
// Stub for Grej Labs' Assets system.
// PlayerEsp.cpp references Assets::WeaponIcons and Assets::InterExtraBold.
// Silk Kernel loads its own fonts/textures via silkui; these return nullptr so
// weapon-icon and font-override code paths fall back gracefully.

#include <d3d11.h>
#include <imgui.h>

namespace Assets {
    // Font pointers — nullptr means fall back to ImGui default font
    inline ImFont* InterExtraBold = nullptr;
    inline ImFont* icomoon        = nullptr;
    inline ImFont* InterSemiBold  = nullptr;
    inline ImFont* lexend_bold   = nullptr;
    inline ImFont* lexend_regular = nullptr;

    namespace WeaponIcons {
        // All weapon icon textures — nullptr disables weapon icon drawing
        inline ID3D11ShaderResourceView* AK47          = nullptr;
        inline ID3D11ShaderResourceView* FIST          = nullptr;
        inline ID3D11ShaderResourceView* M4A1          = nullptr;
        inline ID3D11ShaderResourceView* USP           = nullptr;
        inline ID3D11ShaderResourceView* AWM           = nullptr;
        inline ID3D11ShaderResourceView* M1014         = nullptr;
        inline ID3D11ShaderResourceView* UMP           = nullptr;
        inline ID3D11ShaderResourceView* MP5           = nullptr;
        inline ID3D11ShaderResourceView* DESERT_EAGLE  = nullptr;
        inline ID3D11ShaderResourceView* G18           = nullptr;
        inline ID3D11ShaderResourceView* M14           = nullptr;
        inline ID3D11ShaderResourceView* SCAR          = nullptr;
        inline ID3D11ShaderResourceView* VSS           = nullptr;
        inline ID3D11ShaderResourceView* GROZA         = nullptr;
        inline ID3D11ShaderResourceView* MP40          = nullptr;
        inline ID3D11ShaderResourceView* PAN           = nullptr;
        inline ID3D11ShaderResourceView* PARANG        = nullptr;
        inline ID3D11ShaderResourceView* SKS           = nullptr;
        inline ID3D11ShaderResourceView* M249          = nullptr;
        inline ID3D11ShaderResourceView* M1873         = nullptr;
        inline ID3D11ShaderResourceView* KAR98K        = nullptr;
        inline ID3D11ShaderResourceView* CROSSBOW      = nullptr;
        inline ID3D11ShaderResourceView* M79           = nullptr;
        inline ID3D11ShaderResourceView* FAMAS         = nullptr;
        inline ID3D11ShaderResourceView* M500          = nullptr;
        inline ID3D11ShaderResourceView* SVD           = nullptr;
        inline ID3D11ShaderResourceView* BAT           = nullptr;
        inline ID3D11ShaderResourceView* XM8           = nullptr;
        inline ID3D11ShaderResourceView* SPAS12        = nullptr;
        inline ID3D11ShaderResourceView* M60           = nullptr;
        inline ID3D11ShaderResourceView* P90           = nullptr;
        inline ID3D11ShaderResourceView* AN94          = nullptr;
        inline ID3D11ShaderResourceView* KATANA        = nullptr;
        inline ID3D11ShaderResourceView* CG15          = nullptr;
        inline ID3D11ShaderResourceView* RGS50         = nullptr;
        inline ID3D11ShaderResourceView* GATLING       = nullptr;
        inline ID3D11ShaderResourceView* HOOK_GUN      = nullptr;
        inline ID3D11ShaderResourceView* PLASMA        = nullptr;
        inline ID3D11ShaderResourceView* M1887         = nullptr;
        inline ID3D11ShaderResourceView* THOMPSON      = nullptr;
        inline ID3D11ShaderResourceView* M82B          = nullptr;
        inline ID3D11ShaderResourceView* AUG           = nullptr;
        inline ID3D11ShaderResourceView* PARAFAL       = nullptr;
        inline ID3D11ShaderResourceView* WOODPECKER    = nullptr;
        inline ID3D11ShaderResourceView* VECTOR        = nullptr;
        inline ID3D11ShaderResourceView* MAG7          = nullptr;
        inline ID3D11ShaderResourceView* SCYTHE        = nullptr;
        inline ID3D11ShaderResourceView* KORD          = nullptr;
        inline ID3D11ShaderResourceView* M1917         = nullptr;
        inline ID3D11ShaderResourceView* USP2          = nullptr;
        inline ID3D11ShaderResourceView* KINGFISHER    = nullptr;
        inline ID3D11ShaderResourceView* MINI_UZI      = nullptr;
        inline ID3D11ShaderResourceView* MP5_I         = nullptr;
        inline ID3D11ShaderResourceView* M60_I         = nullptr;
        inline ID3D11ShaderResourceView* VSS_I         = nullptr;
        inline ID3D11ShaderResourceView* BIZON         = nullptr;
        inline ID3D11ShaderResourceView* TROGON        = nullptr;
        inline ID3D11ShaderResourceView* VSK94         = nullptr;
        inline ID3D11ShaderResourceView* MAC10         = nullptr;
        inline ID3D11ShaderResourceView* AC80          = nullptr;
        inline ID3D11ShaderResourceView* G36           = nullptr;
        inline ID3D11ShaderResourceView* M24           = nullptr;
        inline ID3D11ShaderResourceView* M590          = nullptr;
        inline ID3D11ShaderResourceView* CHARGE_BUSTER = nullptr;
        inline ID3D11ShaderResourceView* MAG_7         = nullptr;
        inline ID3D11ShaderResourceView* HEAL_SNIPER   = nullptr;
        inline ID3D11ShaderResourceView* HEAL_PISTOL   = nullptr;
        inline ID3D11ShaderResourceView* SHIELD_GUN    = nullptr;
        inline ID3D11ShaderResourceView* FLAMETHROWER  = nullptr;
        inline ID3D11ShaderResourceView* FF_KNIFE      = nullptr;
        inline ID3D11ShaderResourceView* GRENADE       = nullptr;
        inline ID3D11ShaderResourceView* SMOKE_GRENADE = nullptr;
        inline ID3D11ShaderResourceView* FLASH_FREEZE  = nullptr;
        inline ID3D11ShaderResourceView* GLOO_MELTER   = nullptr;
        inline ID3D11ShaderResourceView* GLOO_WALL     = nullptr;
    }
}
