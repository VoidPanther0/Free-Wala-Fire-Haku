#pragma once

#include <iostream>
#include <unordered_map>
#include <thread>
#include <string>
#include <FrameWork/Utilities/Memory/Memory.hpp>
#include <FrameWork/Math/Entity.hpp>
#include <FrameWork/Math/Transform.hpp>
#include <Cheat/Offsets/Offsets.hpp>
#include <Cheat/FreeFireSDK/Context.hpp>
#include <Cheat/Options.hpp>

inline float defaultRecoil = -1.0f;



namespace FrameWork {
    class Data {
    public:
        static void Work();
        static void SilentAimReworkedThread();
        static void SilentLiteThread();
        static void ForceAimThread();
        static void EnemyPullThread();
        static void FlyHackInternalThread();
        static void NoGravityFlyThread();
        static void MapTeleportThread();
        static void AimLockThread();
        static void SmoothAimbotThread();
        static void SpoofNameThread();
        static void ClimbUpThread();
        static void ShakeKillThread();
        static void TeleportBaseThread();
        static void UndergroundThread();
        static void DownPlayerThread();
		static void SpeedHackThread();
		static void VisionHackThread();
		static void InfiniteAmmoThread();
		static void BackJumpThread();

    private:
        static bool EntityData(uint32_t entity, Entity& player, Vector3& mainPos);
        static void HandleRecoil(uint32_t localPlayer);
        static void ReadAndStorePlayerName(uintptr_t entityAddr, Entity& entity);
        static void Reset();;
    };
}
