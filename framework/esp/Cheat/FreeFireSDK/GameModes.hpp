#pragma once

#include <cstdint>
#include <string>

class GameModes {
public:
    static uint32_t GetGameMode(uintptr_t localPlayer);
    static uint32_t GetMatchMode(uintptr_t localPlayer);
    static uint32_t GetGroupMode(uintptr_t localPlayer);
    static uint32_t GetMapID(uintptr_t localPlayer);
    static uint32_t GetRoomType(uintptr_t localPlayer);
    static std::string GetWorkshopMapName(uintptr_t localPlayer);
    static std::string GetGameModeName(uintptr_t localPlayer);

    static bool IsInTraining(uintptr_t localPlayer);
    static bool IsInCraftland(uintptr_t localPlayer);
    static bool IsInLoneWolf(uintptr_t localPlayer);
    static bool IsInBR(uintptr_t localPlayer);
    static bool IsInCS(uintptr_t localPlayer);
    static bool IsSolo(uintptr_t localPlayer);
    static bool IsDuo(uintptr_t localPlayer);
    static bool IsSquad(uintptr_t localPlayer);
};
