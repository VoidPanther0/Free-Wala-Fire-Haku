#include "GameModes.hpp"

#include <Cheat\Offsets\Offsets.hpp>
#include <FrameWork\Utilities\Memory\Memory.hpp>

namespace {
    uint32_t GetProfile(uintptr_t localPlayer)
    {
        if (!localPlayer)
            return 0;

        return FrameWork::Mem.ReadS<uint32_t>(
            localPlayer + Offsets::Player_BaseProfileInfo);
    }

    uint32_t ReadProfileUInt32(uintptr_t localPlayer, uintptr_t offset)
    {
        const uint32_t profile = GetProfile(localPlayer);
        return profile ? FrameWork::Mem.ReadS<uint32_t>(profile + offset) : 0;
    }
}

uint32_t GameModes::GetGameMode(uintptr_t localPlayer)
{
    return ReadProfileUInt32(localPlayer, Offsets::Profile_GameMode);
}

uint32_t GameModes::GetMatchMode(uintptr_t localPlayer)
{
    return ReadProfileUInt32(localPlayer, Offsets::Profile_MatchMode);
}

uint32_t GameModes::GetGroupMode(uintptr_t localPlayer)
{
    return ReadProfileUInt32(localPlayer, Offsets::Profile_GroupMode);
}

uint32_t GameModes::GetMapID(uintptr_t localPlayer)
{
    return ReadProfileUInt32(localPlayer, Offsets::Profile_MapID);
}

uint32_t GameModes::GetRoomType(uintptr_t localPlayer)
{
    return ReadProfileUInt32(localPlayer, Offsets::Profile_RoomType);
}

std::string GameModes::GetWorkshopMapName(uintptr_t localPlayer)
{
    const uint32_t profile = GetProfile(localPlayer);
    if (!profile)
        return {};

    const uint32_t stringObject =
        FrameWork::Mem.ReadS<uint32_t>(profile + Offsets::Profile_WorkshopMapName);
    return stringObject
        ? FrameWork::Mem.String(stringObject + Offsets::ManagedStringText, 256, true)
        : std::string{};
}

std::string GameModes::GetGameModeName(uintptr_t localPlayer)
{
    switch (GetGameMode(localPlayer)) {
    case 0: return "BR";
    case 1: return "CS";
    case 2: return "Lone Wolf";
    case 3: return "Craftland";
    case 4: return "PVE";
    case 5: return "Training";
    case 6: return "Workshop";
    case 7: return "Custom Room";
    case 8: return "Ranked";
    case 9: return "Casual";
    default: return "Unknown";
    }
}

bool GameModes::IsInTraining(uintptr_t localPlayer) { return GetGameMode(localPlayer) == 5; }
bool GameModes::IsInCraftland(uintptr_t localPlayer) { return GetGameMode(localPlayer) == 3; }
bool GameModes::IsInLoneWolf(uintptr_t localPlayer) { return GetGameMode(localPlayer) == 2; }
bool GameModes::IsInBR(uintptr_t localPlayer) { return GetGameMode(localPlayer) == 0; }
bool GameModes::IsInCS(uintptr_t localPlayer) { return GetGameMode(localPlayer) == 1; }
bool GameModes::IsSolo(uintptr_t localPlayer) { return GetGroupMode(localPlayer) == 0; }
bool GameModes::IsDuo(uintptr_t localPlayer) { return GetGroupMode(localPlayer) == 1; }
bool GameModes::IsSquad(uintptr_t localPlayer) { return GetGroupMode(localPlayer) == 2; }
