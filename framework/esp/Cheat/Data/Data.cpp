#include "Data.hpp"
#include <thread>
#include <chrono>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <random>
#include <optional>
#include <vector>
#include <cfloat>
#include <FrameWork/Math/W2S.hpp>
#include <FrameWork/Math/Vectors/Vector2.hpp>
#include <FrameWork/Math/Quaternion.hpp>
#include <FrameWork/Render/Overlay.hpp>

namespace {
    void ApplyTimerSpeed(uint32_t currentGame, float value)
    {
        if (currentGame == 0) return;
        uint32_t timerObj = 0;
        if (!FrameWork::Mem.Read<uint32_t>(currentGame + Offsets::GameTimer, timerObj) || timerObj == 0) return;
        FrameWork::Mem.Write<float>(timerObj + Offsets::FixedDeltaTime, value);
    }

    namespace DownPlayerState {
        std::unordered_map<uint32_t, float> originalY;
        bool isFiring = false;
        bool forceRefresh = true;

        void RefreshOriginalY()
        {
            if (!isFiring) {
                originalY.clear();
                forceRefresh = true;
            }
        }

        bool ResolveRootMatrix(uint32_t entityAddress, uint32_t& matrixPtr)
        {
            uint32_t rootBone = 0;
            uint32_t transform = 0;
            matrixPtr = 0;

            if (!Mem.Read(entityAddress + Offsets::Bones::Root, rootBone) || rootBone == 0)
                return false;
            if (!Mem.Read(rootBone + 0x8, transform) || transform == 0)
                return false;
            if (!Mem.Read(transform + 0x8, transform) || transform == 0)
                return false;
            if (!Mem.Read(transform + 0x20, matrixPtr) || matrixPtr == 0)
                return false;

            return true;
        }

        Entity* FindBestTarget()
        {
            if (Context::Entities.empty())
                return nullptr;

            Entity* best = nullptr;
            float bestDistance = FLT_MAX;
            const Vector2 center(Context::WindowWidth * 0.5f, Context::WindowHeight * 0.5f);

            for (auto& [id, entity] : Context::Entities) {
                if (entity.Address == 0 || entity.Address == Context::LocalPlayerAddress ||
                    !entity.IsKnown || entity.IsDead || entity.IsKnocked)
                    continue;

                const Vector2 head2D = W2S::WorldToScreen(
                    Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                if (head2D.X < 1.0f || head2D.Y < 1.0f)
                    continue;

                if (Vector3::Distance(Context::LocalMainCamera, entity.Head) > 200.0f)
                    continue;

                const float screenDistance = Vector2::Distance(center, head2D);
                if (screenDistance > 500.0f || screenDistance >= bestDistance)
                    continue;

                bestDistance = screenDistance;
                best = &entity;
            }

            return best;
        }

        void AimAtTarget(Entity* target)
        {
            if (!target)
                return;

            uint32_t matrixPtr = 0;
            if (!ResolveRootMatrix(target->Address, matrixPtr))
                return;

            Vector3 enemyPosition;
            if (!Mem.Read(matrixPtr + 0x60, enemyPosition))
                return;

            const auto key = target->Address;
            if (forceRefresh || originalY.find(key) == originalY.end())
                originalY[key] = enemyPosition.Y;

            const float desiredY = originalY[key] +
                g_Options.Misc.Exploits.LocalPlayer.DownValue;
            if (std::abs(enemyPosition.Y - desiredY) > 0.2f) {
                enemyPosition.Y = desiredY;
                Mem.Write(matrixPtr + 0x60, enemyPosition);
            }
        }
    }
}

namespace FrameWork {

    uint32_t previouscount = 0;
    void Data::Work() {
        ::Context::HasMatrix = false;
        static uint32_t savedLocalPlayer = 0;
        try {

            if (Offsets::Il2Cpp == 0) {
                return;
            }

            uint32_t baseGameFacade = Mem.ReadS<uint32_t>(Offsets::Il2Cpp + 0 + Offsets::InitBase);
            if (!baseGameFacade)
            {
                Reset();
                return;
            }

            uint32_t gameFacade = Mem.ReadS<uint32_t>(baseGameFacade);
            if (!gameFacade) {
                Reset();
                return;
            }

            uint32_t staticGameFacade = Mem.ReadS<uint32_t>(gameFacade + Offsets::StaticClass);
            if (!staticGameFacade) {
                Reset();
                return;
            }

            uint32_t currentGame = Mem.ReadS<uint32_t>(staticGameFacade);
            if (!currentGame) {
                Reset();
                return;
            }

            // ==================== SPEED HACK IMPLEMENTATION ====================
            static bool lastSpeedState = false;
            bool speedActive = false;

            if (g_Options.LegitBot.AimBot.enablespeed || g_Options.Misc.Exploits.LocalPlayer.speed) {
                if (g_Options.LegitBot.AimBot.speedkey != 0) {
                    speedActive = (GetAsyncKeyState(g_Options.LegitBot.AimBot.speedkey) & 0x8000) != 0;
                }
                else {
                    speedActive = true;
                }
            }

            if (speedActive) {
                ApplyTimerSpeed(currentGame, 0.050000f);
                lastSpeedState = true;
            }
            else if (lastSpeedState) {
                ApplyTimerSpeed(currentGame, 0.033000f);
                lastSpeedState = false;
            }

            uint32_t currentMatch = Mem.ReadS<uint32_t>(currentGame + Offsets::CurrentMatch);
            if (!currentMatch) {
                Reset();
                return;
            }

            uint32_t matchStatus = Mem.ReadS<uint32_t>(currentMatch + Offsets::MatchStatus);
            /*  if (!matchStatus) {
                  Reset();
                  return;
              }

              if (matchStatus != 1) {
                  Reset();
                  return;
              }*/

              // defaultRecoil = -1.0f;

            uint32_t localPlayer = 0;
            if (Mem.Read<uint32_t>(currentMatch + Offsets::LocalPlayer, localPlayer) && localPlayer != 0) {
                savedLocalPlayer = localPlayer;
            }
            else if (savedLocalPlayer != 0) {
                localPlayer = savedLocalPlayer;
            }
            else {
                localPlayer = 0;
                Reset();
                return;
            }

            Context::LocalPlayerAddress = localPlayer;
            Context::CurrentMatch = currentMatch;
            Context::MatchStatus = static_cast<int>(matchStatus);
            Context::LocalPlayerInMatch = matchStatus == InMatch;
            ::MatchStatus = static_cast<MatchStates>(matchStatus);

            const uint32_t profileInfo =
                Mem.ReadS<uint32_t>(localPlayer + Offsets::Player_BaseProfileInfo);
            Context::LocalProfileValid = profileInfo != 0;
            if (profileInfo != 0)
            {
                const auto readProfileString = [profileInfo](uintptr_t field) {
                    const uint32_t stringObject = Mem.ReadS<uint32_t>(profileInfo + field);
                    return stringObject
                        ? Mem.String(stringObject + Offsets::ManagedStringText, 256, true)
                        : std::string{};
                    };

                Context::LocalUserID =
                    Mem.ReadS<uint64_t>(profileInfo + Offsets::Player_AccountID);
                Context::LocalClanID =
                    Mem.ReadS<uint64_t>(profileInfo + Offsets::Profile_ClanID);
                Context::LocalGroupID =
                    Mem.ReadS<uint64_t>(profileInfo + Offsets::Profile_GroupID);
                Context::LocalRoomID =
                    Mem.ReadS<uint64_t>(profileInfo + Offsets::Profile_RoomID);
                Context::LocalRegionID =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_Region);
                Context::LocalRegion = readProfileString(Offsets::Profile_Region);
                if (Context::LocalRegion.empty() || Context::LocalRegion == "Unknown") {
                    Context::LocalRegion = GetRegionName(Context::LocalRegionID);
                }
                Context::LocalNickName = readProfileString(Offsets::Profile_NickName);
                Context::LocalClanName = readProfileString(Offsets::Profile_ClanName);
                Context::LocalWorkshopMapName =
                    readProfileString(Offsets::Profile_WorkshopMapName);
                Context::LocalLevel =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_Level);
                Context::LocalGroupMode =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_GroupMode);
                Context::LocalGroupMemberCount =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_GroupMemberCnt);
                Context::LocalGameMode =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_GameMode);
                Context::LocalMatchMode =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_MatchMode);
                Context::LocalMapID =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_MapID);
                Context::LocalHasPveTalent =
                    Mem.ReadS<bool>(profileInfo + Offsets::Profile_HasPveTalent);
                Context::LocalRoomType =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_RoomType);
                Context::LocalRoomMember =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_RoomMember);
                Context::LocalRoomMaxMember =
                    Mem.ReadS<uint32_t>(profileInfo + Offsets::Profile_RoomMaxMember);
                Context::LocalBRRank =
                    Mem.ReadS<int>(profileInfo + Offsets::Player_BRRank);
                Context::LocalBRRankPoint =
                    Mem.ReadS<int>(profileInfo + Offsets::Player_BRRankPoint);
                Context::LocalCSRank =
                    Mem.ReadS<int>(profileInfo + Offsets::Player_CSRank);
                Context::LocalCSRankPoint =
                    Mem.ReadS<int>(profileInfo + Offsets::Player_CSRankPoint);
            }

            if (g_Options.Misc.Exploits.LocalPlayer.norecoil) {
                uint32_t weapon = Mem.ReadS<uint32_t>(localPlayer + Offsets::Weapon);
                if (weapon != 0) {
                    uint32_t weaponData = Mem.ReadS<uint32_t>(weapon + Offsets::WeaponData);
                    if (weaponData != 0) {
                        float currentRecoil = Mem.ReadS<float>(weaponData + Offsets::WeaponRecoil);
                        if (currentRecoil != 0.0f) {
                            // Recoil ko zero karein
                            Mem.Write<float>(weaponData + Offsets::WeaponRecoil, 0.0f);
                        }
                    }
                }
            }

            // ==================== NO RELOAD IMPLEMENTATION ====================
            if (g_Options.Misc.Exploits.LocalPlayer.Noreload) {
                uint32_t localPlayerAttributes = Mem.ReadS<uint32_t>(localPlayer + Offsets::LocalPlayerAttributes);
                if (localPlayerAttributes != 0) {
                    Mem.Write<bool>(localPlayerAttributes + Offsets::NoReload, true);
                }
            }
            else {
                uint32_t localPlayerAttributes = Mem.ReadS<uint32_t>(localPlayer + Offsets::LocalPlayerAttributes);
                if (localPlayerAttributes != 0) {
                    Mem.Write<bool>(localPlayerAttributes + Offsets::NoReload, false);
                }
            }
            // ==================== FAST SHOT IMPLEMENTATION ====================
            uint32_t attributesAddr = 0;
            if (Mem.Read<uint32_t>(Context::LocalPlayerAddress + Offsets::LocalPlayerAttributes, attributesAddr) && attributesAddr) {
                static std::unordered_map<uint64_t, float> originalFireIntervalValues;
                const uint64_t key = static_cast<uint64_t>(attributesAddr);
                float value = 0.0f;

                if (g_Options.Misc.Exploits.LocalPlayer.FastShot) {
                    // Correct offset: FireIntervalScaleTwo (0x18C) or RapidFire3 (0x18C)
                    if (Mem.Read<float>(attributesAddr + Offsets::FireIntervalScaleTwo, value)) {
                        originalFireIntervalValues.try_emplace(key, value);

                        // 🔥 ULTRA FAST FIRE - 0.1f = 10x faster
                        constexpr float rapidValue = 0.1f;  // Changed from 0.5f to 0.1f

                        if (std::fabs(value - rapidValue) > 0.001f) {
                            Mem.Write<float>(attributesAddr + Offsets::FireIntervalScaleTwo, rapidValue);
                        }
                    }

                    g_Options.Misc.Exploits.LocalPlayer.LastRapidFireState = true;
                }
                else if (g_Options.Misc.Exploits.LocalPlayer.LastRapidFireState) {
                    float restore = 1.0f;
                    if (auto it = originalFireIntervalValues.find(key); it != originalFireIntervalValues.end()) {
                        restore = it->second;
                    }

                    Mem.Write<float>(attributesAddr + Offsets::FireIntervalScaleTwo, restore);
                    g_Options.Misc.Exploits.LocalPlayer.LastRapidFireState = false;
                }
            }





            uint32_t mainTransform = 0;
            if (!Mem.Read<uint32_t>(savedLocalPlayer + Offsets::MainCameraTransform, mainTransform))
            {
                mainTransform = 0;
                Reset();
                return;
            }

            Vector3 mainPos = { 0.0f, 0.0f, 0.0f };
            if (mainTransform != 0) {
                if (!Transform::GetPosition(mainTransform, mainPos)) {
                    Reset();
                    return;
                }
            }

            Context::LocalMainCamera = mainPos;

            uint32_t followCamera = Mem.ReadS<uint32_t>(localPlayer + Offsets::FollowCamera);
            if (!followCamera) {
                Reset();
                return;
            }

            uint32_t camera = Mem.ReadS<uint32_t>(followCamera + Offsets::Camera);
            if (!camera) {
                Reset();
                return;
            }

            uint32_t cameraBase = Mem.ReadS<uint32_t>(camera + 0x8);
            if (!cameraBase) {
                Reset();
                return;
            }


            Matrix4x4 viewMatrix = Mem.ReadS<Matrix4x4>(cameraBase + Offsets::ViewMatrix);
            Context::HasMatrix = true;
            Context::ViewMatrix = viewMatrix;

            auto entities = Transform::RetrieveEntities(currentGame, Offsets::DictionaryEntities);

            for (auto entityAddr : entities)
            {
                if (!entityAddr || entityAddr == localPlayer)
                    continue;

                auto& entityMap = Context::Entities;

                auto it = entityMap.find(entityAddr);

                if (it != entityMap.end())
                {
                    Entity& entity = it->second;

                    EntityData(entityAddr, entity, mainPos);

                    if (entity.IsTeam == Bool3::True)
                        continue;

                    if (!entity.IsKnown)
                        continue;
                }
                else
                {
                    Entity newEntity{};

                    if (EntityData(entityAddr, newEntity, mainPos))
                    {
                        entityMap[entityAddr] = newEntity;
                    }
                }
            }

        }
        catch (const std::exception& e) {
            /*Reset();*/
            std::cerr << "Exception caught: " << e.what() << std::endl;
        }
        catch (...) {
            /*Reset();*/
            std::cerr << "Unknown exception caught!" << std::endl;
        }

    }

    bool Data::EntityData(uint32_t entity, Entity& player, Vector3& mainPos)
    {

        uint32_t avatarManager = Mem.ReadS<uint32_t>(entity + Offsets::AvatarManager);
        if (avatarManager == 0) {
            Context::Entities.erase(entity);
            return false;
        }

        uint32_t avatar = Mem.ReadS<uint32_t>(avatarManager + Offsets::Avatar);
        if (avatar == 0) {
            Context::Entities.erase(entity);
            return false;
        }

        bool isVisible = Mem.ReadS<bool>(avatar + Offsets::Avatar_IsVisible);
        player.IsVisible = isVisible;

        if (!g_Options.Visuals.ESP.Players.ShowActiveSkills && !player.IsVisible)
        {
            Context::Entities.erase(entity);
            return false;
        }

        uint32_t avatarData = Mem.ReadS<uint32_t>(avatar + Offsets::Avatar_Data);
        if (avatarData == 0) {
            Context::Entities.erase(entity);
            return false;
        }

        bool isTeam = Mem.ReadS<bool>(avatarData + Offsets::Avatar_Data_IsTeam);
        player.IsTeam = isTeam ? Bool3::True : Bool3::False;
        player.IsKnown = !isTeam;

        if (player.IsTeam == Bool3::True || !player.IsKnown) {
            return false;
        }

        uint32_t shadowBase = Mem.ReadS<uint32_t>(entity + Offsets::Player_ShadowBase);
        if (shadowBase != 0) {
            int xpose = Mem.ReadS<int>(shadowBase + Offsets::XPose);
            player.IsKnocked = (xpose == 8);
        }

        player.IsDead = Mem.ReadS<bool>(entity + Offsets::Player_IsDead);

        player.Address = entity;

        uint64_t UserID = Mem.ReadS<uint64_t>(entity + Offsets::Player_UserID);
        player.UserID = UserID;

        uint32_t dataPool = Mem.ReadS<uint32_t>(entity + Offsets::Player_Data);
        if (dataPool != 0) {
            uint32_t poolObj = Mem.ReadS<uint32_t>(dataPool + 0x8);
            if (poolObj != 0) {
                uint32_t pool = Mem.ReadS<uint32_t>(poolObj + 0x10);
                if (pool != 0) {
                    player.Health = Mem.ReadS<short>(pool + 0x10);
                }
            }
        }



        if (dataPool) {
            uint32_t poolObj = Mem.ReadS<uint32_t>(dataPool + 0x8);
            if (poolObj) {
                uint32_t pool = Mem.ReadS<uint32_t>(poolObj + 0x20);
                if (pool) {
                    uint32_t weaponid;
                    if (Mem.Read<uint32_t>(pool + 0x10, weaponid)) {

                        if (player.Address != NULL)
                        {
                            auto WeaponInfo = JuwonWeapon::NameWeapon(weaponid);
                            player.WeaponName = WeaponInfo;
                            player.WeaponIndex = weaponid;
                        }
                    }
                }
            }
        }



        struct BoneInfo {
            uintptr_t offset;
            Vector3* target;
        };
        std::vector<BoneInfo> bones = {
        {Offsets::Bones::Head, &player.Head},
        {Offsets::Bones::Neck, &player.Neck},
        {Offsets::Bones::ShoulderLeft, &player.ShoulderLeft},
        {Offsets::Bones::ShoulderRight, &player.ShoulderRight},
        {Offsets::Bones::ElbowLeft, &player.ElbowLeft},
        {Offsets::Bones::ElbowRight, &player.ElbowRight},
        {Offsets::Bones::WristLeft, &player.WristLeft},
        {Offsets::Bones::WristRight, &player.WristRight},
        {Offsets::Bones::Hip, &player.Hip},
        {Offsets::Bones::Root, &player.Root},
        {Offsets::Bones::HipCenter, &player.HipCenter},
        {Offsets::Bones::AnkleLeft, &player.AnkleLeft},
        {Offsets::Bones::AnkleRight, &player.AnkleRight},
        };

        for (const auto& bone : bones) {
            uint32_t boneAddr = 0;
            if (Mem.Read<uint32_t>(entity + bone.offset, boneAddr) && boneAddr != 0) {
                Vector3 boneTransform = { 0.0f, 0.0f, 0.0f };
                if (Transform::GetNodePosition(boneAddr, boneTransform)) {
                    *bone.target = boneTransform;
                    if (bone.target == &player.Head) {
                        player.Distance = Vector3::Distance(mainPos, boneTransform);
                    }
                }
            }
        }

        ReadAndStorePlayerName(entity, player);

        return true;
    }

    void Data::ReadAndStorePlayerName(uintptr_t entityAddr, Entity& entity)
    {
        uint32_t nameAddr = 0;
        if (Mem.Read(entityAddr + Offsets::Player_Name, nameAddr) && nameAddr != 0) {
            const int nameLen = 256;
            std::string name = Mem.String(nameAddr + 0xC, nameLen, true);


            name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) {

                return (!isprint(c) || (c < 0x20 || c > 0x7E)) && c != ' ' && c != '-';
                }), name.end());

            if (!name.empty()) {
                entity.Name = name;
            }
            else {
                entity.Name = "Training Doll";
            }
        }
        else {
            entity.Name = "Training Doll";
        }
    }

    void Data::Reset() {
        try {
            if (!Context::Entities.empty()) {
                Context::Entities.clear();
            }

            if (Context::HasMatrix) {
                Context::HasMatrix = false;
                Context::ViewMatrix = Matrix4x4();
            }

            Mem.ClearCache();
            Context::LocalMainCamera = Vector3(0, 0, 0);
            Context::CurrentMatch = 0;
            Context::LocalUserID = 0;
            Context::LocalProfileValid = false;
            Context::LocalClanID = 0;
            Context::LocalGroupID = 0;
            Context::LocalRoomID = 0;
            Context::LocalRegionID = 0;
            Context::LocalRegion.clear();
            Context::LocalNickName.clear();
            Context::LocalClanName.clear();
            Context::LocalWorkshopMapName.clear();
            Context::LocalLevel = 0;
            Context::LocalGroupMode = 0;
            Context::LocalGroupMemberCount = 0;
            Context::LocalGameMode = 0;
            Context::LocalMatchMode = 0;
            Context::LocalMapID = 0;
            Context::LocalHasPveTalent = false;
            Context::LocalRoomType = 0;
            Context::LocalRoomMember = 0;
            Context::LocalRoomMaxMember = 0;
            Context::LocalBRRank = 0;
            Context::LocalBRRankPoint = 0;
            Context::LocalCSRank = 0;
            Context::LocalCSRankPoint = 0;
            Context::MatchStatus = NotStarted;
            Context::LocalPlayerInMatch = false;
        }
        catch (const std::exception& e) {
            std::cerr << "Error clearing game state: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown error while clearing game state." << std::endl;
        }
    }

    // ==================== SILENT AIM IMPLEMENTATION ====================
    namespace Silent_CppX {
        struct SilentEntitySnap {
            Vector3 head = Vector3::Zero();
            Vector3 hip = Vector3::Zero();
            Vector3 root = Vector3::Zero();
            bool isKnown = false;
            bool isDead = false;
            bool isKnocked = false;
        };

        inline std::vector<SilentEntitySnap> SnapshotEntities() {
            std::vector<SilentEntitySnap> snapshot;
            try {
                snapshot.reserve(Context::Entities.size());
                for (const auto& [id, entity] : Context::Entities) {
                    snapshot.push_back({
                        entity.Head,
                        entity.Hip,
                        entity.Root,
                        entity.IsKnown,
                        entity.IsDead,
                        entity.IsKnocked
                        });
                }
            }
            catch (...) {
                snapshot.clear();
            }
            return snapshot;
        }

        static Vector3 s_LastRedirectBytes = Vector3::Zero();
        static uint32_t s_LastWeaponData = 0;
        static bool s_HasPreviousAim = false;

        inline void WriteSilentAim(uint32_t localPlayer, const Vector3& aimWorld) {
            bool isShooting = false;
            if (!Mem.Read<bool>(localPlayer + Offsets::sAim1, isShooting) || !isShooting) {
                s_HasPreviousAim = false;
                s_LastWeaponData = 0;
                return;
            }

            uint32_t weaponData = 0;
            if (!Mem.Read<uint32_t>(localPlayer + Offsets::sAim2, weaponData) || weaponData == 0)
                return;

            Vector3 startPos{};
            if (!Mem.Read<Vector3>(weaponData + Offsets::sAim3, startPos))
                return;

            Vector3 redirect = aimWorld - startPos;

            // Jab tak bytes change nahi hoti, tab tak aage request na bhej
            if (s_HasPreviousAim && weaponData == s_LastWeaponData &&
                std::memcmp(&s_LastRedirectBytes, &redirect, sizeof(Vector3)) == 0)
            {
                return;
            }

            // Bytes change hone se pehle sirf aik dafa write / request bheje
            Mem.Write<Vector3>(weaponData + Offsets::sAim4, redirect);

            s_LastRedirectBytes = redirect;
            s_LastWeaponData = weaponData;
            s_HasPreviousAim = true;
        }

        // Max (Head) — closest head to crosshair, +0.1 Y
        inline void WorkSilentMax(const std::vector<SilentEntitySnap>& entities,
            const Matrix4x4& viewMatrix, const Vector3& mainCamera,
            int width, int height, uint32_t localPlayer, bool ignoreKnocked)
        {
            const Vector2 screenCenter(width / 2.0f, height / 2.0f);
            Vector3 targetHead = Vector3::Zero();
            float bestDist = FLT_MAX;
            bool hasTarget = false;

            for (const auto& entity : entities) {
                if (!entity.isKnown || entity.isDead || (ignoreKnocked && entity.isKnocked))
                    continue;

                const Vector2 head2D = W2S::WorldToScreen(viewMatrix, entity.head, width, height);
                const float crossDist = Vector2::Distance(screenCenter, Vector2(head2D.X, head2D.Y));
                if (crossDist < bestDist) {
                    bestDist = crossDist;
                    targetHead = entity.head;
                    hasTarget = true;
                }
                (void)Vector3::Distance(mainCamera, entity.root);
            }

            if (!hasTarget)
                return;

            Vector3 adjusted = targetHead;
            adjusted.Y += 0.1f;
            WriteSilentAim(localPlayer, adjusted);
        }

        // Body — closest hip to crosshair, no Y bias
        inline void WorkSilentBody(const std::vector<SilentEntitySnap>& entities,
            const Matrix4x4& viewMatrix, const Vector3& mainCamera,
            int width, int height, uint32_t localPlayer)
        {
            const Vector2 screenCenter(width / 2.0f, height / 2.0f);
            Vector3 targetHip = Vector3::Zero();
            float bestDist = FLT_MAX;
            bool hasTarget = false;

            for (const auto& entity : entities) {
                if (entity.isDead || entity.isKnocked)
                    continue;

                const Vector2 hip2D = W2S::WorldToScreen(viewMatrix, entity.hip, width, height);
                const float crossDist = Vector2::Distance(screenCenter, Vector2(hip2D.X, hip2D.Y));
                if (crossDist < bestDist) {
                    bestDist = crossDist;
                    targetHip = entity.hip;
                    hasTarget = true;
                }
                (void)Vector3::Distance(mainCamera, entity.root);
            }

            if (!hasTarget)
                return;

            WriteSilentAim(localPlayer, targetHip);
        }

        inline void Work() {
            try {
                const int width = Context::WindowWidth;
                const int height = Context::WindowHeight;
                if (width == -1 || height == -1 || !Context::HasMatrix) {
                    return;
                }

                const Matrix4x4 viewMatrix = Context::ViewMatrix;
                const Vector3 mainCamera = Context::LocalMainCamera;
                const uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
                const bool ignoreKnocked = g_Options.LegitBot.AimBot.IgnoreDowned;
                const std::vector<SilentEntitySnap> entities = SnapshotEntities();

                const bool silentMax = g_Options.LegitBot.AimBot.SilentEnabled;
                const bool silentBody = false;

                if (silentMax)
                    WorkSilentMax(entities, viewMatrix, mainCamera, width, height, localPlayer, ignoreKnocked);
                if (silentBody)
                    WorkSilentBody(entities, viewMatrix, mainCamera, width, height, localPlayer);
            }
            catch (...) {
            }
        }
    }

    void Data::SilentAimReworkedThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.LegitBot.AimBot.SilentEnabled) {
                Silent_CppX::Work();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    // ==================== FORCE AIM IMPLEMENTATION ====================
    namespace ForceAimCpp {
        static std::unordered_map<uint32_t, Vector3> s_originalRoots;
        static uint32_t s_lockedAddr = 0;
        static int s_lastForceAimMode = -1;

        static bool ResolveRootMatrix(uint32_t entityAddr, uint32_t& matrixPtr) {
            uint32_t rootBone = 0, t1 = 0, t2 = 0;
            matrixPtr = 0;
            if (!Mem.Read(entityAddr + Offsets::Bones::Root, rootBone) || rootBone == 0)
                return false;
            if (!Mem.Read(rootBone + 0x8, t1) || t1 == 0)
                return false;
            if (!Mem.Read(t1 + 0x8, t2) || t2 == 0)
                return false;
            if (!Mem.Read(t2 + 0x20, matrixPtr) || matrixPtr == 0)
                return false;
            return true;
        }

        static void WriteRootPosition(uint32_t entityAddr, const Vector3& pos) {
            uint32_t matrixPtr = 0;
            if (!ResolveRootMatrix(entityAddr, matrixPtr))
                return;
            Mem.Write<Vector3>(matrixPtr + 0x60, pos);
        }

        static void ClearLock() {
            s_lockedAddr = 0;
        }

        static Vector3 ReadLiveBone(uint32_t entityAddr, uintptr_t boneOffset)
        {
            uint32_t bonePtr = 0;
            if (!Mem.Read(entityAddr + boneOffset, bonePtr) || bonePtr == 0)
                return Vector3::Zero();
            Vector3 pos;
            if (!Transform::GetNodePosition(bonePtr, pos) || pos == Vector3::Zero())
                return Vector3::Zero();
            return pos;
        }

        static void RestoreAll() {
            for (const auto& entry : s_originalRoots)
                WriteRootPosition(entry.first, entry.second);
            s_originalRoots.clear();
            ClearLock();
        }

        static Vector3 GetAimPoint(const Entity& p) {
            if (p.Address == 0)
                return Vector3::Zero();

            if (g_Options.LegitBot.AimBot.Hitbox == 1) {
                Vector3 hip = ReadLiveBone(p.Address, Offsets::Bones::Hip);
                if (hip != Vector3::Zero())
                    return hip;
                return (p.Hip != Vector3::Zero()) ? p.Hip : Vector3::Zero();
            }

            Vector3 head = ReadLiveBone(p.Address, Offsets::Bones::Head);
            if (head != Vector3::Zero())
                return head;
            return (p.Head != Vector3::Zero()) ? p.Head : Vector3::Zero();
        }

        static bool IsValidTarget(const Entity& p, uint32_t localAddr) {
            if (p.Address == 0 || p.Address == localAddr)
                return false;
            if (!p.IsKnown || p.IsDead || p.IsTeam == Bool3::True)
                return false;
            if (g_Options.LegitBot.AimBot.IgnoreDowned && p.IsKnocked)
                return false;
            return GetAimPoint(p) != Vector3::Zero();
        }

        static void GetSearchFovAndDistance(uint32_t localPlayer, float& screenFov, float& maxDist3D)
        {
            screenFov = static_cast<float>(g_Options.LegitBot.AimBot.FOVRange);
            maxDist3D = 300.0f;

            if (localPlayer == 0)
                return;

            float distAim = static_cast<float>(g_Options.LegitBot.AimBot.DistanceSilent);
            if (distAim < 400.0f)
                distAim = 400.0f;
            if (maxDist3D < distAim)
                maxDist3D = distAim;
        }

        static Entity* FindTarget() {
            Entity* best = nullptr;
            float bestFov = FLT_MAX;
            const float cx = Context::WindowWidth / 2.0f;
            const float cy = Context::WindowHeight / 2.0f;
            const uint32_t localAddr = static_cast<uint32_t>(Context::LocalPlayerAddress);
            float maxDist = 0.0f;
            float aimFov = 0.0f;
            GetSearchFovAndDistance(localAddr, aimFov, maxDist);

            for (auto& kv : Context::Entities) {
                Entity* p = &kv.second;
                if (!IsValidTarget(*p, localAddr))
                    continue;

                Vector3 aimPt = GetAimPoint(*p);
                Vector2 scr = W2S::WorldToScreen(Context::ViewMatrix, aimPt,
                    Context::WindowWidth, Context::WindowHeight);
                if (scr.X < 1.0f || scr.Y < 1.0f)
                    continue;

                float dist = Vector3::Distance(Context::LocalMainCamera, aimPt);
                if (dist > maxDist)
                    continue;

                float fov = sqrtf((scr.X - cx) * (scr.X - cx) + (scr.Y - cy) * (scr.Y - cy));
                if (fov > aimFov || fov >= bestFov)
                    continue;

                bestFov = fov;
                best = p;
            }
            return best;
        }

        static Entity* FindLockedTarget() {
            if (s_lockedAddr == 0)
                return nullptr;
            auto it = Context::Entities.find(s_lockedAddr);
            if (it == Context::Entities.end())
                return nullptr;
            Entity* p = &it->second;
            if (!IsValidTarget(*p, static_cast<uint32_t>(Context::LocalPlayerAddress)))
                return nullptr;
            return p;
        }

        static void Tick(uint32_t localPlayer) {
            if (!g_Options.Misc.Exploits.LocalPlayer.ForceAim) {
                if (!s_originalRoots.empty())
                    RestoreAll();
                s_lastForceAimMode = -1;
                return;
            }
            if (Context::WindowWidth < 1 || Context::WindowHeight < 1 ||
                !Context::HasMatrix || localPlayer == 0)
                return;

            const int aimMode = (g_Options.Misc.Exploits.LocalPlayer.ForceAimMode == 1) ? 1 : 0;
            if (s_lastForceAimMode != aimMode) {
                s_lastForceAimMode = aimMode;
                if (!s_originalRoots.empty())
                    RestoreAll();
                ClearLock();
            }

            bool isFiring = false;
            Mem.Read<bool>(localPlayer + Offsets::sAim1, isFiring);
            const bool lmbHeld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            if (!isFiring && !lmbHeld) {
                if (!s_originalRoots.empty())
                    RestoreAll();
                ClearLock();
                return;
            }

            Entity* target = FindLockedTarget();
            if (!target) {
                target = FindTarget();
                if (!target)
                    return;
                s_lockedAddr = target->Address;
            }

            uint32_t rootBone = 0;
            if (!Mem.Read(target->Address + Offsets::Bones::Root, rootBone) || rootBone == 0)
                return;

            Vector3 currentRoot;
            if (!Transform::GetNodePosition(rootBone, currentRoot))
                return;

            if (s_originalRoots.find(target->Address) == s_originalRoots.end())
                s_originalRoots[target->Address] = currentRoot;

            const Vector3 originalRoot = s_originalRoots[target->Address];

            Vector3 aimTarget = GetAimPoint(*target);
            if (aimTarget == Vector3::Zero())
                return;

            const Vector3 rootOffsetFromAim = currentRoot - aimTarget;

            Vector3 fwd(Context::ViewMatrix.m[0][2],
                Context::ViewMatrix.m[1][2],
                Context::ViewMatrix.m[2][2]);
            float len = sqrtf(fwd.X * fwd.X + fwd.Y * fwd.Y + fwd.Z * fwd.Z);
            if (len < 1e-4f)
                return;
            fwd.X /= len;
            fwd.Y /= len;
            fwd.Z /= len;

            const Vector3 cam = Context::LocalMainCamera;
            Vector3 toAim = aimTarget - cam;
            float proj = fwd.X * toAim.X + fwd.Y * toAim.Y + fwd.Z * toAim.Z;
            Vector3 aimOnLine = cam + fwd * proj;
            Vector3 desiredRoot = aimOnLine + rootOffsetFromAim;

            float maxPull = g_Options.Misc.Exploits.LocalPlayer.ForceAimMaxPull;
            if (maxPull <= 0.0f)
                maxPull = 8.0f;
            float maxPullVertical = g_Options.Misc.Exploits.LocalPlayer.ForceAimMaxPullVertical;
            Vector3 delta = desiredRoot - originalRoot;
            if (fabsf(delta.X) > maxPull || fabsf(delta.Z) > maxPull || fabsf(delta.Y) > maxPullVertical) {
                WriteRootPosition(target->Address, originalRoot);
                return;
            }

            WriteRootPosition(target->Address, desiredRoot);
        }
    }

    void Data::ForceAimThread() {
        while (!g_Options.General.ShutDown) {
            uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (localPlayer != 0) {
                ForceAimCpp::Tick(localPlayer);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ForceAimCpp::RestoreAll();
    }

    // ==================== ENEMY PULL IMPLEMENTATION ====================
    namespace PullEnemy360Cpp {
        static std::unordered_map<uint32_t, Vector3> s_originalPositions;
        static std::chrono::steady_clock::time_point s_lastTick{};

        static void RestoreAllPositions() {
            for (const auto& entry : s_originalPositions) {
                uint32_t rootBonePtr = 0, t1 = 0, t2 = 0, matrixPtr = 0;
                if (!Mem.Read(entry.first + Offsets::Bones::Root, rootBonePtr) || rootBonePtr == 0)
                    continue;
                if (!Mem.Read(rootBonePtr + 0x8, t1) || t1 == 0)
                    continue;
                if (!Mem.Read(t1 + 0x8, t2) || t2 == 0)
                    continue;
                if (!Mem.Read(t2 + 0x20, matrixPtr) || matrixPtr == 0)
                    continue;
                Mem.Write<Vector3>(matrixPtr + 0x60, entry.second);
            }
            s_originalPositions.clear();
        }

        static bool EnemyPullActive() {
            return g_Options.Misc.Exploits.LocalPlayer.MagnetEnemy || g_Options.Misc.Exploits.LocalPlayer.PullEnemy360Enabled;
        }

        static int TickMs() {
            const int a = 6;
            const int b = g_Options.Misc.Exploits.LocalPlayer.PullEnemy360TickMs;
            const int v = (a > 0) ? a : b;
            return (v > 0) ? v : 6;
        }

        static float MaxDistance() {
            const float a = 250.0f;
            const float b = g_Options.Misc.Exploits.LocalPlayer.PullEnemy360MaxDistance;
            if (a > 1.0f) return a;
            if (b > 1.0f) return b;
            return 250.0f;
        }

        static Entity* FindBestTarget() {
            Entity* bestTarget = nullptr;
            float closestDist = FLT_MAX;
            const int w = Context::WindowWidth;
            const int h = Context::WindowHeight;
            if (w < 1 || h < 1 || !Context::HasMatrix)
                return nullptr;

            const Vector2 screenCenter(w / 2.0f, h / 2.0f);
            const float aimFov = static_cast<float>(g_Options.LegitBot.AimBot.FOVRange);
            const float maxDist3D = MaxDistance();
            const bool ignoreKnocked = g_Options.LegitBot.AimBot.IgnoreDowned;
            const uint32_t localAddr = static_cast<uint32_t>(Context::LocalPlayerAddress);

            for (auto& kv : Context::Entities) {
                Entity& ent = kv.second;
                if (ent.Address == 0 || ent.Address == localAddr)
                    continue;
                if (!ent.IsKnown || ent.IsDead)
                    continue;
                if (ent.IsTeam == Bool3::True)
                    continue;
                if (ignoreKnocked && ent.IsKnocked)
                    continue;

                Vector2 head2D = W2S::WorldToScreen(Context::ViewMatrix, ent.Head, w, h);
                if (head2D.X < 1.0f || head2D.Y < 1.0f)
                    continue;

                float dist3D = Vector3::Distance(Context::LocalMainCamera, ent.Head);
                if (dist3D > maxDist3D)
                    continue;

                float crosshairDist = Vector2::Distance(screenCenter, head2D);
                if (crosshairDist > aimFov)
                    continue;

                if (crosshairDist < closestDist) {
                    closestDist = crosshairDist;
                    bestTarget = &ent;
                }
            }
            return bestTarget;
        }

        static void ApplyInstantPull(Entity& entity) {
            uint32_t rootBonePtr = 0, transformValue = 0, transformObjPtr = 0, matrixPtr = 0;
            if (!Mem.Read(entity.Address + Offsets::Bones::Root, rootBonePtr) || rootBonePtr == 0)
                return;
            if (!Mem.Read(rootBonePtr + 0x8, transformValue) || transformValue == 0)
                return;
            if (!Mem.Read(transformValue + 0x8, transformObjPtr) || transformObjPtr == 0)
                return;
            if (!Mem.Read(transformObjPtr + 0x20, matrixPtr) || matrixPtr == 0)
                return;

            Vector3 currentPos;
            if (!Transform::GetNodePosition(rootBonePtr, currentPos))
                return;

            if (s_originalPositions.find(entity.Address) == s_originalPositions.end())
                s_originalPositions[entity.Address] = currentPos;

            const Vector3 originalPos = s_originalPositions[entity.Address];

            Vector3 fireDir(
                Context::ViewMatrix.m[0][2],
                Context::ViewMatrix.m[1][2],
                Context::ViewMatrix.m[2][2]);
            float len = sqrtf(fireDir.X * fireDir.X + fireDir.Y * fireDir.Y + fireDir.Z * fireDir.Z);
            if (len < 1e-4f)
                return;
            fireDir.X /= len;
            fireDir.Y /= len;
            fireDir.Z /= len;

            const Vector3 camPos = Context::LocalMainCamera;
            Vector3 toEnemy = currentPos - camPos;
            float projLength = toEnemy.X * fireDir.X + toEnemy.Y * fireDir.Y + toEnemy.Z * fireDir.Z;
            Vector3 finalPos = camPos + fireDir * projLength;

            constexpr float kClamp = 5.0f;
            float deltaX = finalPos.X - originalPos.X;
            float deltaZ = finalPos.Z - originalPos.Z;
            if (fabsf(deltaX) > kClamp)
                finalPos.X = originalPos.X + copysignf(kClamp, deltaX);
            if (fabsf(deltaZ) > kClamp)
                finalPos.Z = originalPos.Z + copysignf(kClamp, deltaZ);
            finalPos.Y = originalPos.Y;

            Mem.Write<Vector3>(matrixPtr + 0x60, finalPos);
        }

        static void Tick(uint32_t localPlayer) {
            if (!EnemyPullActive()) {
                if (!s_originalPositions.empty())
                    RestoreAllPositions();
                s_lastTick = std::chrono::steady_clock::now();
                return;
            }

            const int tickMs = TickMs();
            const auto now = std::chrono::steady_clock::now();
            if (now - s_lastTick < std::chrono::milliseconds(tickMs))
                return;
            s_lastTick = now;

            if (Context::WindowWidth < 1 || Context::WindowHeight < 1 ||
                !Context::HasMatrix || localPlayer == 0)
                return;

            bool isFiring = false;
            if (!Mem.Read<bool>(localPlayer + Offsets::sAim1, isFiring) || !isFiring) {
                RestoreAllPositions();
                return;
            }

            Entity* target = FindBestTarget();
            if (target != nullptr)
                ApplyInstantPull(*target);
            else
                RestoreAllPositions();
        }

        void Stop()
        {
            RestoreAllPositions();
            s_lastTick = std::chrono::steady_clock::now();
        }
    }

    void Data::EnemyPullThread() {
        while (!g_Options.General.ShutDown) {
            uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (localPlayer != 0) {
                PullEnemy360Cpp::Tick(localPlayer);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        PullEnemy360Cpp::Stop();
    }

    // ==================== FLY HACK IMPLEMENTATION ====================
    namespace FlyHack_LocalPlayer
    {
        // OFFSETS
        constexpr uint32_t MOVEMENT_COMPONENT_OFFSET = 0x1178;
        constexpr uint32_t POSITION_OFFSET = 0x1C;
        constexpr uint32_t VSPEED_OFFSET = 0x28;
        constexpr uint32_t IS_GROUNDED_OFFSET = 0x150;

        // SPEED CONFIG
        constexpr float HORIZONTAL_SPEED = 6.0f;
        constexpr float UP_SPEED = 3.0f;
        constexpr float DOWN_SPEED = -3.0f;
        constexpr float HOVER_SPEED = 0.02f;

        // HEIGHT LIMIT
        constexpr float MAX_FLY_HEIGHT = 9000.0f;
        static float baseHeight = 0.0f;
        static bool heightInitialized = false;

        static std::atomic<bool> isRunning{ false };
        static std::thread workThread;

        void ApplyFly()
        {
            uint32_t player = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (player == 0) return;

            uint32_t moveComp = 0;
            if (!Mem.Read(player + MOVEMENT_COMPONENT_OFFSET, moveComp) || moveComp == 0)
                return;

            // keep airborne
            Mem.Write<uint8_t>(moveComp + IS_GROUNDED_OFFSET, 0);

            // read position
            Vector3 pos;
            if (!Mem.Read<Vector3>(moveComp + POSITION_OFFSET, pos))
                return;

            // init base height
            if (!heightInitialized)
            {
                baseHeight = pos.Y;
                heightInitialized = true;
            }

            float maxAllowedHeight = baseHeight + MAX_FLY_HEIGHT;

            // camera direction
            Vector3 forward(
                Context::ViewMatrix.m[0][2],
                Context::ViewMatrix.m[1][2],
                Context::ViewMatrix.m[2][2]
            );
            Vector3 right(
                Context::ViewMatrix.m[0][0],
                Context::ViewMatrix.m[1][0],
                Context::ViewMatrix.m[2][0]
            );

            Vector3 tempFwd(forward.X, 0.0f, forward.Z);
            float fwdLen = std::sqrt(tempFwd.X * tempFwd.X + tempFwd.Y * tempFwd.Y + tempFwd.Z * tempFwd.Z);
            Vector3 fwd = (fwdLen > 0.0f) ? Vector3(tempFwd.X / fwdLen, tempFwd.Y / fwdLen, tempFwd.Z / fwdLen) : Vector3::Zero();

            Vector3 tempRgt(right.X, 0.0f, right.Z);
            float rgtLen = std::sqrt(tempRgt.X * tempRgt.X + tempRgt.Y * tempRgt.Y + tempRgt.Z * tempRgt.Z);
            Vector3 rgt = (rgtLen > 0.0f) ? Vector3(tempRgt.X / rgtLen, tempRgt.Y / rgtLen, tempRgt.Z / rgtLen) : Vector3::Zero();

            Vector3 moveDir = Vector3::Zero();

            if (GetAsyncKeyState(0x57) & 0x8000) moveDir = moveDir + fwd; // W
            if (GetAsyncKeyState(0x53) & 0x8000) moveDir = moveDir - fwd; // S
            if (GetAsyncKeyState(0x44) & 0x8000) moveDir = moveDir + rgt; // D
            if (GetAsyncKeyState(0x41) & 0x8000) moveDir = moveDir - rgt; // A

            float moveLen = std::sqrt(moveDir.X * moveDir.X + moveDir.Z * moveDir.Z);
            if (moveLen > 0.0f) {
                moveDir.X = (moveDir.X / moveLen) * HORIZONTAL_SPEED;
                moveDir.Z = (moveDir.Z / moveLen) * HORIZONTAL_SPEED;
            }

            pos.X += moveDir.X * 0.016f;
            pos.Z += moveDir.Z * 0.016f;

            // height limit
            if (pos.Y > maxAllowedHeight)
                pos.Y = maxAllowedHeight;

            Mem.Write<Vector3>(moveComp + POSITION_OFFSET, pos);

            float vSpeed = HOVER_SPEED;

            if ((GetAsyncKeyState(0x20) & 0x8000) && pos.Y < maxAllowedHeight - 0.1f) // SPACE
                vSpeed = UP_SPEED;
            else if (GetAsyncKeyState(0xA2) & 0x8000) // LCONTROL
                vSpeed = DOWN_SPEED;

            Mem.Write<float>(moveComp + VSPEED_OFFSET, vSpeed);
        }

        static void Loop()
        {
            while (isRunning.load())
            {
                try
                {
                    // Only run fly hack when local player exists and feature is enabled
                    if (Context::LocalPlayerAddress != 0 && g_Options.Misc.Exploits.LocalPlayer.FlyHackInternalEnabled)
                    {
                        ApplyFly();
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(40));
                        continue;
                    }
                }
                catch (...) {}

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        void Start()
        {
            if (isRunning.load()) return;
            isRunning = true;
            heightInitialized = false;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            isRunning = false;
        }
    }

    namespace NoGravityFly
    {
        // Offsets from updated dump (porulfly / Player.IFGAOAHPNOC) — Silent Aim X
        constexpr uint32_t MOVEMENT_COMPONENT_OFFSET = 0x14E8; // Player.IFGAOAHPNOC
        constexpr uint32_t POSITION_OFFSET = 0x20;              // POAJHDKFLGE
        constexpr uint32_t VSPEED_OFFSET = 0x2C;                // EPCIJJLKNCO
        constexpr uint32_t IS_GROUNDED_OFFSET = 0x1554;         // Player.m_GroundHitResult

        constexpr float HORIZONTAL_SPEED = 9.0f;
        constexpr float VERTICAL_SPEED = 5.5f;
        constexpr float MAX_FLY_HEIGHT = 9000.0f;
        constexpr float SMOOTH_DAMPING = 8.0f; // Smooth inertia interpolation factor

        static float baseHeight = 0.0f;
        static bool heightInitialized = false;
        static Vector3 currentVelocity = Vector3::Zero();
        static std::chrono::steady_clock::time_point lastTickTime;

        static std::atomic<bool> isRunning{ false };
        static std::thread workThread;

        static void ApplyFly()
        {
            const uint32_t player = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (player == 0) {
                heightInitialized = false;
                currentVelocity = Vector3::Zero();
                return;
            }

            uint32_t moveComp = 0;
            if (!Mem.Read(player + MOVEMENT_COMPONENT_OFFSET, moveComp) || moveComp == 0)
                return;

            // keep airborne (ground flag is on Player, not PhysXData)
            Mem.Write<uint8_t>(player + IS_GROUNDED_OFFSET, 0);
            Mem.Write<float>(moveComp + VSPEED_OFFSET, 0.0f);

            Vector3 pos{};
            if (!Mem.Read<Vector3>(moveComp + POSITION_OFFSET, pos))
                return;

            if (!heightInitialized)
            {
                baseHeight = pos.Y;
                heightInitialized = true;
                currentVelocity = Vector3::Zero();
                lastTickTime = std::chrono::steady_clock::now();
            }

            // High precision Delta Time calculation
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastTickTime).count();
            lastTickTime = now;
            if (dt <= 0.0f || dt > 0.1f) dt = 0.016f;

            const float maxAllowedHeight = baseHeight + MAX_FLY_HEIGHT;

            if (!Context::HasMatrix)
                return;

            const Matrix4x4& matrix = Context::ViewMatrix;
            Vector3 forward(matrix.m[0][2], matrix.m[1][2], matrix.m[2][2]);
            Vector3 right(matrix.m[0][0], matrix.m[1][0], matrix.m[2][0]);

            Vector3 tempFwd(forward.X, 0.0f, forward.Z);
            float fwdLen = std::sqrt(tempFwd.X * tempFwd.X + tempFwd.Z * tempFwd.Z);
            Vector3 fwd = (fwdLen > 0.001f) ? Vector3(tempFwd.X / fwdLen, 0.0f, tempFwd.Z / fwdLen) : Vector3::Zero();

            Vector3 tempRgt(right.X, 0.0f, right.Z);
            float rgtLen = std::sqrt(tempRgt.X * tempRgt.X + tempRgt.Z * tempRgt.Z);
            Vector3 rgt = (rgtLen > 0.001f) ? Vector3(tempRgt.X / rgtLen, 0.0f, tempRgt.Z / rgtLen) : Vector3::Zero();

            // Horizontal input
            Vector3 targetVelocity = Vector3::Zero();

            if (GetAsyncKeyState(0x57) & 0x8000) targetVelocity = targetVelocity + fwd; // W
            if (GetAsyncKeyState(0x53) & 0x8000) targetVelocity = targetVelocity - fwd; // S
            if (GetAsyncKeyState(0x44) & 0x8000) targetVelocity = targetVelocity + rgt; // D
            if (GetAsyncKeyState(0x41) & 0x8000) targetVelocity = targetVelocity - rgt; // A

            float inputLen = std::sqrt(targetVelocity.X * targetVelocity.X + targetVelocity.Z * targetVelocity.Z);
            if (inputLen > 0.001f)
            {
                targetVelocity.X = (targetVelocity.X / inputLen) * HORIZONTAL_SPEED;
                targetVelocity.Z = (targetVelocity.Z / inputLen) * HORIZONTAL_SPEED;
            }

            // Vertical input: Space = Up, LControl = Down
            if ((GetAsyncKeyState(0x20) & 0x8000) && pos.Y < maxAllowedHeight - 0.2f) // SPACE
            {
                targetVelocity.Y = VERTICAL_SPEED;
            }
            else if (GetAsyncKeyState(0xA2) & 0x8000) // LCONTROL
            {
                targetVelocity.Y = -VERTICAL_SPEED;
            }
            else
            {
                targetVelocity.Y = 0.0f;
            }

            // Smooth exponential decay interpolation for butter-smooth gliding
            const float blend = 1.0f - std::exp(-SMOOTH_DAMPING * dt);
            currentVelocity.X += (targetVelocity.X - currentVelocity.X) * blend;
            currentVelocity.Y += (targetVelocity.Y - currentVelocity.Y) * blend;
            currentVelocity.Z += (targetVelocity.Z - currentVelocity.Z) * blend;

            // Smooth displacement integration
            pos.X += currentVelocity.X * dt;
            pos.Y += currentVelocity.Y * dt;
            pos.Z += currentVelocity.Z * dt;

            if (pos.Y > maxAllowedHeight)
                pos.Y = maxAllowedHeight;

            Mem.Write<Vector3>(moveComp + POSITION_OFFSET, pos);
            Mem.Write<float>(moveComp + VSPEED_OFFSET, 0.0f);
        }

        static void Loop()
        {
            while (isRunning.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.NoGravityFlyEnabled || Context::LocalPlayerAddress == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                        continue;
                    }

                    ApplyFly();
                }
                catch (...) {}

                std::this_thread::sleep_for(std::chrono::milliseconds(8));
            }
        }

        void Start()
        {
            if (isRunning.load()) return;
            isRunning = true;
            heightInitialized = false;
            currentVelocity = Vector3::Zero();
            lastTickTime = std::chrono::steady_clock::now();
            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            isRunning = false;
            heightInitialized = false;
            currentVelocity = Vector3::Zero();
        }
    }

    void Data::FlyHackInternalThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.FlyHackInternalEnabled) {
                FlyHack_LocalPlayer::Start();
            }
            else {
                FlyHack_LocalPlayer::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        FlyHack_LocalPlayer::Stop();
    }

    void Data::NoGravityFlyThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.NoGravityFlyEnabled) {
                NoGravityFly::Start();
            }
            else {
                NoGravityFly::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        NoGravityFly::Stop();
    }

    // ==================== SILENT LITE IMPLEMENTATION ====================
    namespace SilentLite_CppX {
        static std::atomic<bool> g_Running{ false };
        static std::atomic<bool> g_Cancel{ false };
        static std::thread g_SilentThread;

        void SilentLoop()
        {
            while (!g_Cancel.load() && !g_Options.General.ShutDown)
            {
                if (!g_Options.LegitBot.AimBot.SilentLiteEnabled)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }

                // Left mouse button check
                bool isLeftMousePressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
                if (!isLeftMousePressed)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                int width = Context::WindowWidth;
                int height = Context::WindowHeight;
                if (width <= 0 || height <= 0)
                {
                    RECT rc{};
                    HWND hTarget = FrameWork::Overlay::GetTargetWindow();
                    if (hTarget && GetClientRect(hTarget, &rc) && (rc.right - rc.left) > 0)
                    {
                        width = rc.right - rc.left;
                        height = rc.bottom - rc.top;
                    }
                    else
                    {
                        width = GetSystemMetrics(SM_CXSCREEN);
                        height = GetSystemMetrics(SM_CYSCREEN);
                    }
                    if (width <= 0) width = 1920;
                    if (height <= 0) height = 1080;
                }

                if (!Context::HasMatrix)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                const Vector2 screenCenter(width / 2.0f, height / 2.0f);
                const Entity* target = nullptr;
                float distance = FLT_MAX;

                const float maxDistance = (g_Options.LegitBot.AimBot.DistanceSilent > 0)
                    ? static_cast<float>(g_Options.LegitBot.AimBot.DistanceSilent)
                    : 300.0f;

                Vector3 cameraPos = Context::LocalMainCamera;
                if (cameraPos == Vector3::Zero())
                {
                    auto it = Context::Entities.find(static_cast<uint32_t>(Context::LocalPlayerAddress));
                    if (it != Context::Entities.end() && it->second.Head != Vector3::Zero())
                        cameraPos = it->second.Head;
                }

                for (const auto& [id, entity] : Context::Entities)
                {
                    if (!entity.Address || entity.Address == Context::LocalPlayerAddress)
                        continue;
                    if (!entity.IsKnown || entity.IsDead)
                        continue;
                    if (entity.IsKnocked)
                        continue;
                    if (entity.IsTeam == Bool3::True)
                        continue;
                    if (g_Options.LegitBot.AimBot.IgnoreTrainingBots && entity.IsBot)
                        continue;

                    Vector2 head2D = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, width, height);
                    if (head2D.X < 1.0f || head2D.Y < 1.0f)
                        continue;

                    // Reject coords far off-screen
                    if (head2D.X > width + 200.0f || head2D.Y > height + 200.0f)
                        continue;

                    float playerDistance = Vector3::Distance(cameraPos, entity.Head);
                    if (playerDistance > maxDistance)
                        continue;

                    float x = head2D.X - screenCenter.X;
                    float y = head2D.Y - screenCenter.Y;
                    float crosshairDist = std::sqrt(x * x + y * y);

                    if (crosshairDist >= distance || crosshairDist == FLT_MAX)
                        continue;
                    if (crosshairDist > 10000.0f)
                        continue;

                    if (g_Options.LegitBot.AimBot.UseFOV && crosshairDist > static_cast<float>(g_Options.LegitBot.AimBot.FOVRange))
                        continue;

                    distance = crosshairDist;
                    target = &entity;
                }

                if (target != nullptr)
                {
                    uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
                    if (localPlayer != 0)
                    {
                        bool firecheck2 = false;
                        Mem.Read<bool>(localPlayer + Offsets::sAim1, firecheck2);
                        if (firecheck2)
                        {
                            uint32_t testeee22 = 0;
                            Mem.Read<uint32_t>(localPlayer + Offsets::sAim2, testeee22);
                            if (testeee22 != 0)
                            {
                                Vector3 StartPosition{};
                                if (Mem.Read<Vector3>(testeee22 + Offsets::sAim3, StartPosition))
                                {
                                    Vector3 aimHead = target->Head;
                                    if (g_Options.LegitBot.AimBot.Hitbox == 1 && target->Neck != Vector3::Zero())
                                        aimHead = target->Neck;
                                    else
                                        aimHead.Y += 0.08f; // Head adjustment for Free Fire

                                    Vector3 redirectVector = aimHead - StartPosition;

                                    static Vector3 s_LastLiteRedirect = Vector3::Zero();
                                    static uint32_t s_LastLiteWeapon = 0;
                                    static bool s_HasLiteAim = false;

                                    // Jab tak bytes change nahi hoti tab tak aage request na bhej
                                    if (s_HasLiteAim && testeee22 == s_LastLiteWeapon &&
                                        std::memcmp(&s_LastLiteRedirect, &redirectVector, sizeof(Vector3)) == 0)
                                    {
                                        // Bytes change nahi hui hain -> skip
                                        continue;
                                    }

                                    // Bytes change hone se pehle sirf aik dafa write bheje
                                    Mem.Write<Vector3>(testeee22 + Offsets::sAim4, redirectVector);

                                    s_LastLiteRedirect = redirectVector;
                                    s_LastLiteWeapon = testeee22;
                                    s_HasLiteAim = true;
                                }
                            }
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        void Start()
        {
            if (g_Running.load())
                return;

            g_Cancel.store(false);
            g_Running.store(true);

            if (g_SilentThread.joinable())
                g_SilentThread.join();

            g_SilentThread = std::thread(SilentLoop);
        }

        void Stop()
        {
            if (!g_Running.load())
                return;

            g_Cancel.store(true);

            if (g_SilentThread.joinable())
                g_SilentThread.join();

            g_Running.store(false);
        }
    }

    void Data::SilentLiteThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.LegitBot.AimBot.SilentLiteEnabled) {
                SilentLite_CppX::Start();
            }
            else {
                SilentLite_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        SilentLite_CppX::Stop();
    }

    // ==================== MAP TELEPORT IMPLEMENTATION ====================
    namespace MapTeleport_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;
        static uint32_t staticGameFacade = 0;

        static void TeleportToPosition(const Vector3& pos)
        {
            try
            {
                uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
                if (localPlayer == 0) return;

                uint32_t root = 0;
                if (!Mem.Read<uint32_t>(localPlayer + Offsets::Bones::Root, root) || root == 0)
                    return;

                uint32_t transform = 0;
                if (!Mem.Read<uint32_t>(root + 0x8, transform) || transform == 0)
                    return;

                uint32_t obj = 0;
                if (!Mem.Read<uint32_t>(transform + 0x8, obj) || obj == 0)
                    return;

                uint32_t matrix = 0;
                if (!Mem.Read<uint32_t>(obj + 0x20, matrix) || matrix == 0)
                    return;

                Mem.Write<Vector3>(matrix + 0x60, pos);
                Mem.Write<float>(matrix + 0x6C, 1.0f); // Set W = 1 for matrix stability
            }
            catch (...) {}
        }

        static Vector3 GetMarkedPosition()
        {
            try
            {
                // Re-read facade if lost
                if (staticGameFacade == 0)
                {
                    uint32_t baseGameFacade = 0;
                    if (!Mem.Read<uint32_t>(Offsets::Il2Cpp + 0 + Offsets::InitBase, baseGameFacade) || baseGameFacade == 0)
                        return Vector3::Zero();

                    uint32_t gameFacade = 0;
                    if (!Mem.Read<uint32_t>(baseGameFacade, gameFacade) || gameFacade == 0)
                        return Vector3::Zero();

                    uint32_t staticClass = 0;
                    if (!Mem.Read<uint32_t>(gameFacade + Offsets::StaticClass, staticClass) || staticClass == 0)
                        return Vector3::Zero();

                    staticGameFacade = staticClass;
                }

                uint32_t currentGame = 0;
                if (!Mem.Read<uint32_t>(staticGameFacade, currentGame) || currentGame == 0)
                {
                    staticGameFacade = 0; // force re-read next time
                    return Vector3::Zero();
                }

                uint32_t UIInGameScene = 0;
                if (!Mem.Read<uint32_t>(currentGame + 0x8, UIInGameScene) || UIInGameScene == 0)
                    return Vector3::Zero();

                uint32_t m_BigMapCtrl = 0;
                if (!Mem.Read<uint32_t>(UIInGameScene + 0x240, m_BigMapCtrl) || m_BigMapCtrl == 0)
                    return Vector3::Zero();

                uint32_t m_MapContentCtrl = 0;
                if (!Mem.Read<uint32_t>(m_BigMapCtrl + 0x5C, m_MapContentCtrl) || m_MapContentCtrl == 0)
                    return Vector3::Zero();

                uint32_t m_LocalMapMarkController = 0;
                if (!Mem.Read<uint32_t>(m_MapContentCtrl + 0x94, m_LocalMapMarkController) || m_LocalMapMarkController == 0)
                    return Vector3::Zero();

                Vector3 selectedPos;
                if (!Mem.Read<Vector3>(m_LocalMapMarkController + 0x60, selectedPos))
                    return Vector3::Zero();

                return selectedPos;
            }
            catch (...)
            {
                staticGameFacade = 0;
                return Vector3::Zero();
            }
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.MapTeleportEnabled || Context::LocalPlayerAddress == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }

                    Vector3 markedPos = GetMarkedPosition();
                    if (markedPos == Vector3::Zero())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }

                    // Fix: Check if Y is suspiciously high (marker Y is often 500-1000)
                    Vector3 teleportPos = markedPos;
                    if (teleportPos.Y > 150.0f)
                    {
                        teleportPos.Y = 1.5f; // Set to a safe near-ground level
                    }

                    TeleportToPosition(teleportPos);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Slightly more delay for stability
                }
                catch (...)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;
            staticGameFacade = 0;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
            staticGameFacade = 0;
        }
    }

    void Data::MapTeleportThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.MapTeleportEnabled) {
                MapTeleport_CppX::Start();
            }
            else {
                MapTeleport_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        MapTeleport_CppX::Stop();
    }

    // ==================== AIM LOCK IMPLEMENTATION ====================
    namespace AimLock_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;

        // Sniper weapon IDs
        static bool IsSniperWeapon(uint32_t weaponId)
        {
            static const int sniperIds[] = { 4, 65, 21, 64, 128, 129, 45, 75, 78, 197 };
            for (int id : sniperIds)
            {
                if (weaponId == (uint32_t)id)
                    return true;
            }
            return false;
        }

        static uint32_t GetCurrentWeaponId(uint32_t localPlayer)
        {
            uint32_t weapon = 0;
            if (!Mem.Read<uint32_t>(localPlayer + Offsets::Weapon, weapon) || weapon == 0)
                return 0;

            uint32_t weaponData = 0;
            if (!Mem.Read<uint32_t>(weapon + Offsets::WeaponData, weaponData) || weaponData == 0)
                return 0;

            // This would need proper offset for weapon ID, returning 0 for now
            return 0;
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.AimLockEnabled || Context::LocalPlayerAddress == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }

                    uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);

                    // Don't work with sniper weapons
                    uint32_t weaponId = GetCurrentWeaponId(localPlayer);
                    if (weaponId != 0 && IsSniperWeapon(weaponId))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }

                    if ((GetAsyncKeyState(g_Options.Misc.Exploits.LocalPlayer.AimLockKey) & 0x8000) == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }

                    if (Context::WindowWidth == -1 || Context::WindowHeight == -1 || !Context::HasMatrix)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }

                    Entity* target = nullptr;
                    float minFovDistanceSquared = FLT_MAX;
                    float minDistanceSquared = FLT_MAX;

                    const Vector2 screenCenter(Context::WindowWidth / 2.0f, Context::WindowHeight / 2.0f);
                    const float fov = 50.0f; // Default FOV
                    const float fovSquared = fov * fov;

                    for (auto& [id, entity] : Context::Entities)
                    {
                        if (entity.IsDead || !entity.IsKnown || (g_Options.LegitBot.AimBot.IgnoreDowned && entity.IsKnocked))
                            continue;

                        const Vector2 head2D = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                        if (head2D.X < 1.0f || head2D.Y < 1.0f)
                            continue;

                        const float playerDistance = Vector3::Distance(Context::LocalMainCamera, entity.Head);
                        const float maxDist = static_cast<float>(g_Options.LegitBot.AimBot.DistanceSilent);
                        if (playerDistance > maxDist)
                            continue;

                        const float x = head2D.X - screenCenter.X;
                        const float y = head2D.Y - screenCenter.Y;
                        const float crosshairDist = std::sqrt(x * x + y * y);

                        if (crosshairDist > fov)
                            continue;

                        if (crosshairDist < minFovDistanceSquared && playerDistance < minDistanceSquared)
                        {
                            minFovDistanceSquared = crosshairDist;
                            minDistanceSquared = playerDistance;
                            target = &entity;
                        }
                    }

                    if (target != nullptr)
                    {
                        Quaternion playerLook = Quaternion::GetRotationToLocation(target->Head, 0.1f, Context::LocalMainCamera);
                        Mem.Write<Quaternion>(localPlayer + Offsets::AimRotation, playerLook);
                    }
                }
                catch (...)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
        }
    }

    void Data::AimLockThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.AimLockEnabled) {
                AimLock_CppX::Start();
            }
            else {
                AimLock_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        AimLock_CppX::Stop();
    }

    // ==================== SMOOTH AIMBOT IMPLEMENTATION ====================
    namespace SmoothAimbot_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;

        static Entity* FindTarget()
        {
            Entity* best = nullptr;
            float bestDist = FLT_MAX;

            const Vector2 center(Context::WindowWidth / 2.0f, Context::WindowHeight / 2.0f);
            const float fov = 9999.0f; // No FOV limit by default
            const float fovSq = fov * fov;

            for (auto& [id, entity] : Context::Entities)
            {
                if (!entity.IsKnown || entity.IsDead)
                    continue;
                if (entity.Address == Context::LocalPlayerAddress)
                    continue;
                if (g_Options.LegitBot.AimBot.IgnoreDowned && entity.IsKnocked)
                    continue;
                if (entity.IsTeam == Bool3::True)
                    continue;
                if (entity.Head == Vector3::Zero())
                    continue;

                const float worldDist = Vector3::Distance(Context::LocalMainCamera, entity.Head);
                const float maxDist = static_cast<float>(g_Options.LegitBot.AimBot.DistanceSilent);
                if (worldDist > maxDist)
                    continue;

                const Vector2 head2D = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                if (head2D.X < 0.0f || head2D.Y < 0.0f ||
                    head2D.X > Context::WindowWidth || head2D.Y > Context::WindowHeight)
                    continue;

                const float dx = head2D.X - center.X;
                const float dy = head2D.Y - center.Y;
                const float distSq = dx * dx + dy * dy;

                if (distSq > fovSq)
                    continue;

                if (distSq < bestDist)
                {
                    bestDist = distSq;
                    best = &entity;
                }
            }

            return best;
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.SmoothAimbotEnabled || Context::LocalPlayerAddress == 0 || !Context::HasMatrix)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        continue;
                    }

                    // Hold key check
                    if ((GetAsyncKeyState(g_Options.Misc.Exploits.LocalPlayer.SmoothAimHoldKey) & 0x8000) == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        continue;
                    }

                    // Find best target
                    Entity* target = FindTarget();
                    if (target == nullptr)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        continue;
                    }

                    // Get real-time head position from memory (not cached)
                    Vector3 headPos = Vector3::Zero();
                    uint32_t headBone = 0;
                    if (Mem.Read<uint32_t>(target->Address + Offsets::Bones::Head, headBone) && headBone != 0)
                    {
                        Transform::GetNodePosition(headBone, headPos);
                    }

                    if (headPos == Vector3::Zero())
                        headPos = target->Head;

                    headPos.Y += 0.05f; // head center offset

                    // Calculate exact rotation to head
                    Quaternion targetRot = Quaternion::GetRotationToLocation(headPos, 0.1f, Context::LocalMainCamera);

                    // Smoothness: 100 = instant, 1 = very slow
                    // Convert to lerp factor: higher smoothness = faster tracking
                    float lerpFactor = g_Options.Misc.Exploits.LocalPlayer.SmoothAimSmoothness / 10.0f;
                    if (lerpFactor < 0.1f) lerpFactor = 0.1f;
                    if (lerpFactor > 1.0f) lerpFactor = 1.0f;

                    // Read current rotation
                    Quaternion currentRot = Quaternion::Identity();
                    Mem.Read<Quaternion>(static_cast<uint32_t>(Context::LocalPlayerAddress) + Offsets::AimRotation, currentRot);

                    // Lerp towards target
                    Quaternion newRot = Quaternion::Slerp(currentRot, targetRot, lerpFactor);

                    // Write aim rotation
                    Mem.Write<Quaternion>(static_cast<uint32_t>(Context::LocalPlayerAddress) + Offsets::AimRotation, newRot);
                }
                catch (...)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                // 1ms loop — maximum tracking speed
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
        }
    }

    void Data::SmoothAimbotThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.SmoothAimbotEnabled) {
                SmoothAimbot_CppX::Start();
            }
            else {
                SmoothAimbot_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        SmoothAimbot_CppX::Stop();
    }

    // ==================== SPOOF NAME IMPLEMENTATION ====================
    namespace SpoofName_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;
        static long lastApplyTicks = 0;

        // Default display name
        static const std::string DefaultDisplayName = "VOIDX XITERS";

        static const uint32_t NameLengthOffset = 0x8;
        static const uint32_t NameDataOffset = 0xC;
        static const int ApplyIntervalMs = 350;

        static void ApplyToPlayer(uint32_t localPlayer)
        {
            if (!g_Options.Misc.Exploits.LocalPlayer.SpoofNameEnabled || localPlayer == 0)
                return;

            long now = GetTickCount64();
            if (lastApplyTicks != 0 && now - lastApplyTicks < ApplyIntervalMs)
                return;

            lastApplyTicks = now;

            try
            {
                uint32_t nameAddr = 0;
                if (!Mem.Read<uint32_t>(localPlayer + Offsets::Player_Name, nameAddr) || nameAddr == 0)
                    return;

                // Convert string to UTF-16
                std::wstring wideName(DefaultDisplayName.begin(), DefaultDisplayName.end());

                Mem.Write<int>(nameAddr + NameLengthOffset, DefaultDisplayName.length());

                // Write UTF-16 characters
                for (size_t i = 0; i < wideName.size() && i < 32; i++)
                {
                    Mem.Write<uint16_t>(nameAddr + NameDataOffset + (i * 2), wideName[i]);
                }
            }
            catch (...)
            {
            }
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (g_Options.Misc.Exploits.LocalPlayer.SpoofNameEnabled && Context::LocalPlayerAddress != 0)
                    {
                        ApplyToPlayer(static_cast<uint32_t>(Context::LocalPlayerAddress));
                    }
                }
                catch (...)
                {
                }

                int sleepTime = g_Options.Misc.Exploits.LocalPlayer.SpoofNameEnabled ? ApplyIntervalMs : 200;
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;
            lastApplyTicks = 0;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
        }
    }

    void Data::SpoofNameThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.SpoofNameEnabled) {
                SpoofName_CppX::Start();
            }
            else {
                SpoofName_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        SpoofName_CppX::Stop();
    }

    // ==================== CLIMB UP IMPLEMENTATION ====================
    namespace ClimbUp_CppX {
        constexpr float TargetOffset = 65.0f;
        constexpr float AscentSpeed = 0.05f;
        constexpr int LoopSleepMs = 5;

        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;
        static float baseY = 0.0f;
        static bool baseSaved = false;
        static bool ascending = false;
        static bool neverFall = false;
        static bool active = false;

        static bool ReadMatrix(uint32_t localPlayer, uint32_t& matrixPtr)
        {
            matrixPtr = 0;
            uint32_t root = 0;
            if (!Mem.Read(localPlayer + Offsets::Bones::Root, root) || !root)
                return false;

            uint32_t transformOne = 0;
            if (!Mem.Read(root + 0x8, transformOne) || !transformOne)
                return false;

            uint32_t transformTwo = 0;
            if (!Mem.Read(transformOne + 0x8, transformTwo) || !transformTwo)
                return false;

            return Mem.Read(transformTwo + 0x20, matrixPtr) && matrixPtr != 0;
        }

        static void ConfigureNeverFall(uint32_t localPlayer)
        {
            uint32_t movementComponent = 0;
            if (!Mem.Read(localPlayer + Offsets::PlayerMoveComponent, movementComponent) ||
                !movementComponent)
                return;

            Mem.Write<bool>(localPlayer + 0x1492, true);
            Mem.Write<bool>(localPlayer + 0x155, true);
            Mem.Write<float>(movementComponent + 0x1488, 0.0f);
            Mem.Write<float>(movementComponent + 0x148C, 0.0f);
            Mem.Write<float>(movementComponent + 0x1484, 999.0f);
            Mem.Write<bool>(movementComponent + 0x169C, false);
            Mem.Write<bool>(movementComponent + 0x169D, false);
            Mem.Write<float>(movementComponent + 0x16A4, 0.005f);
            Mem.Write<float>(movementComponent + 0x16A0, 0.005f);
            Mem.Write<bool>(movementComponent + 0x16D8, true);
            neverFall = true;
        }

        static void ApplyClimbUp()
        {
            const uint32_t localPlayer =
                static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (!localPlayer)
                return;

            uint32_t matrixPtr = 0;
            if (!ReadMatrix(localPlayer, matrixPtr))
                return;

            if (!baseSaved) {
                Vector3 position{};
                if (!Mem.Read(matrixPtr + 0x60, position))
                    return;
                baseY = position.Y;
                baseSaved = true;
                ascending = true;
            }

            if (!neverFall)
                ConfigureNeverFall(localPlayer);

            Vector3 position{};
            if (!Mem.Read(matrixPtr + 0x60, position))
                return;

            const float targetY = baseY + TargetOffset;
            if (ascending && position.Y < targetY) {
                position.Y = (targetY < position.Y + AscentSpeed)
                    ? targetY
                    : position.Y + AscentSpeed;
                if (position.Y >= targetY)
                    ascending = false;
            }
            else {
                position.Y = targetY;
            }

            Mem.Write(matrixPtr + 0x60, position);
            active = true;
        }

        static void RestoreState()
        {
            const uint32_t localPlayer =
                static_cast<uint32_t>(Context::LocalPlayerAddress);
            uint32_t matrixPtr = 0;
            if (localPlayer && baseSaved && ReadMatrix(localPlayer, matrixPtr)) {
                Vector3 position{};
                if (Mem.Read(matrixPtr + 0x60, position)) {
                    position.Y = baseY;
                    Mem.Write(matrixPtr + 0x60, position);
                }
            }

            baseSaved = false;
            ascending = false;
            neverFall = false;
            active = false;
        }

        static void Loop()
        {
            while (!shouldStop.load()) {
                try {
                    if (g_Options.Misc.Exploits.LocalPlayer.ClimbUpEnabled)
                        ApplyClimbUp();
                    else if (active)
                        RestoreState();
                }
                catch (...) {
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(LoopSleepMs));
            }

            if (active)
                RestoreState();
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load())
                return;

            shouldStop = false;
            isRunning = true;
            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load())
                return;
            shouldStop = true;
        }
    }

    void Data::ClimbUpThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.ClimbUpEnabled) {
                ClimbUp_CppX::Start();
            }
            else {
                ClimbUp_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ClimbUp_CppX::Stop();
    }

    // ==================== SHAKE KILL IMPLEMENTATION ====================
    namespace ShakeKill_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;

        static bool IsInsideFOV(int x, int y)
        {
            const int fov = 1500;
            const int centerX = Context::WindowWidth / 2;
            const int centerY = Context::WindowHeight / 2;
            return (x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= fov * fov;
        }

        static void ApplyShakeKill()
        {
            try
            {
                uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
                if (localPlayer == 0) return;

                if (Context::WindowWidth == -1 || Context::WindowHeight == -1 || !Context::HasMatrix)
                    return;

                // Check if player is firing (pomba offset)
                bool isFiring = false;
                if (!Mem.Read<bool>(localPlayer + Offsets::sAim1, isFiring) || !isFiring)
                    return;

                for (auto& [id, entity] : Context::Entities)
                {
                    if (entity.IsDead || entity.IsKnocked || !entity.IsKnown)
                        continue;

                    // Convert head position to screen space
                    Vector2 head2D = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                    if (head2D.X < 1.0f || head2D.Y < 1.0f)
                        continue;

                    // Check if inside FOV
                    if (!IsInsideFOV(static_cast<int>(head2D.X), static_cast<int>(head2D.Y)))
                        continue;

                    // Read enemy root bone transform chain
                    uint32_t enemyRootBonePtr = 0;
                    if (!Mem.Read<uint32_t>(entity.Address + Offsets::Bones::Root, enemyRootBonePtr) || enemyRootBonePtr == 0)
                        continue;

                    uint32_t enemyTransformValue = 0;
                    if (!Mem.Read<uint32_t>(enemyRootBonePtr + 0x8, enemyTransformValue) || enemyTransformValue == 0)
                        continue;

                    uint32_t enemyTransformObjPtr = 0;
                    if (!Mem.Read<uint32_t>(enemyTransformValue + 0x8, enemyTransformObjPtr) || enemyTransformObjPtr == 0)
                        continue;

                    uint32_t enemyMatrixValue = 0;
                    if (!Mem.Read<uint32_t>(enemyTransformObjPtr + 0x20, enemyMatrixValue) || enemyMatrixValue == 0)
                        continue;

                    // Read enemy position
                    Vector3 enemyPos;
                    if (!Mem.Read<Vector3>(enemyMatrixValue + 0x60, enemyPos))
                        continue;

                    // Add offset to Y position
                    enemyPos.Y += 0.04f;

                    // Write back to matrix
                    Mem.Write<Vector3>(enemyMatrixValue + 0x60, enemyPos);
                }
            }
            catch (...)
            {
            }
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.ShakeKillEnabled || Context::LocalPlayerAddress == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }

                    ApplyShakeKill();
                }
                catch (...)
                {
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
        }
    }

    void Data::ShakeKillThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.ShakeKillEnabled) {
                ShakeKill_CppX::Start();
            }
            else {
                ShakeKill_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ShakeKill_CppX::Stop();
    }

    // ==================== TELEPORT BASE IMPLEMENTATION ====================
    namespace TeleportBase_CppX {
        static std::atomic<bool> isRunning{ false };
        static std::atomic<bool> shouldStop{ false };
        static std::thread workThread;

        static void TeleportToPosition(uint32_t localRootBonePtr, const Vector3& targetPos)
        {
            try
            {
                // Get local transform value
                uint32_t localTransformValue = 0;
                if (!Mem.Read<uint32_t>(localRootBonePtr + 0x8, localTransformValue) || localTransformValue == 0)
                    return;

                // Get local transform object pointer
                uint32_t localTransformObjPtr = 0;
                if (!Mem.Read<uint32_t>(localTransformValue + 0x8, localTransformObjPtr) || localTransformObjPtr == 0)
                    return;

                // Get local matrix value
                uint32_t localMatrixValue = 0;
                if (!Mem.Read<uint32_t>(localTransformObjPtr + 0x20, localMatrixValue) || localMatrixValue == 0)
                    return;

                // Write target position to local player (using 0x60 offset)
                Mem.Write<Vector3>(localMatrixValue + 0x60, targetPos);
            }
            catch (...)
            {
            }
        }

        static void ApplyTeleportBase()
        {
            try
            {
                uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
                if (localPlayer == 0) return;

                // Get local player root bone
                uint32_t localRootBonePtr = 0;
                if (!Mem.Read<uint32_t>(localPlayer + Offsets::Bones::Root, localRootBonePtr) || localRootBonePtr == 0)
                    return;

                // Get local player position
                Vector3 localRootPos;
                if (!Transform::GetNodePosition(localRootBonePtr, localRootPos))
                    return;

                // Find closest enemy
                float closestDistance = FLT_MAX;
                uint32_t bestEnemy = 0;
                Vector3 bestEnemyPos = Vector3::Zero();

                for (auto& [id, entity] : Context::Entities)
                {
                    // Skip invalid entities
                    if (entity.IsDead || entity.Address == localPlayer || entity.Address == 0)
                        continue;

                    // Skip teammates
                    if (entity.IsTeam == Bool3::True)
                        continue;

                    // Get enemy position
                    float playerDistance = Vector3::Distance(localRootPos, entity.Root);
                    float maxDistance = g_Options.Misc.Exploits.LocalPlayer.TeleportBaseDistance;
                    if (playerDistance > maxDistance)
                        continue;

                    // Find closest enemy
                    if (playerDistance < closestDistance)
                    {
                        closestDistance = playerDistance;
                        bestEnemy = entity.Address;
                        bestEnemyPos = entity.Root;
                    }
                }

                // Teleport to closest enemy position
                if (bestEnemy != 0)
                {
                    TeleportToPosition(localRootBonePtr, bestEnemyPos);
                }
            }
            catch (...)
            {
            }
        }

        static void Loop()
        {
            while (!shouldStop.load())
            {
                try
                {
                    if (!g_Options.Misc.Exploits.LocalPlayer.TeleportBaseEnabled || Context::LocalPlayerAddress == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }

                    ApplyTeleportBase();
                }
                catch (...)
                {
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            isRunning = false;
        }

        void Start()
        {
            if (isRunning.load()) return;

            shouldStop = false;
            isRunning = true;

            workThread = std::thread(Loop);
            workThread.detach();
        }

        void Stop()
        {
            if (!isRunning.load()) return;

            shouldStop = true;
        }
    }

    void Data::TeleportBaseThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.TeleportBaseEnabled) {
                TeleportBase_CppX::Start();
            }
            else {
                TeleportBase_CppX::Stop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        TeleportBase_CppX::Stop();
    }

    namespace Underground_CppX {
        constexpr float ClimbDepth = 1.5f;
        constexpr int MoveDurationMs = 700;
        constexpr int SmoothSteps = 50;
        constexpr int LoopSleepMs = 20;
        constexpr uintptr_t PositionOffset = 0x80;

        std::atomic<bool> running{ false };
        std::atomic<bool> stopRequested{ false };
        std::thread workerThread;
        std::optional<Vector3> savedPosition;
        bool isMoving = false;
        bool isUnderground = false;

        bool ReadMatrix(uint32_t localPlayer, uint32_t& matrixPtr)
        {
            matrixPtr = 0;
            uint32_t root = 0;
            if (!Mem.Read(localPlayer + Offsets::Bones::Root, root) || !root)
                return false;

            uint32_t transformOne = 0;
            if (!Mem.Read(root + 0x8, transformOne) || !transformOne)
                return false;

            uint32_t transformTwo = 0;
            if (!Mem.Read(transformOne + 0x8, transformTwo) || !transformTwo)
                return false;

            return Mem.Read(transformTwo + 0x20, matrixPtr) && matrixPtr != 0;
        }

        void SmoothMove(uint32_t matrixPtr, const Vector3& start, const Vector3& end)
        {
            const float stepMs =
                MoveDurationMs / static_cast<float>(SmoothSteps);

            for (int i = 1; i <= SmoothSteps && !stopRequested.load(); ++i) {
                const float t = i / static_cast<float>(SmoothSteps);
                Vector3 position{
                    start.X + (end.X - start.X) * t,
                    start.Y + (end.Y - start.Y) * t,
                    start.Z + (end.Z - start.Z) * t
                };
                Mem.Write(matrixPtr + PositionOffset, position);
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(static_cast<int>(stepMs)));
            }

            Mem.Write(matrixPtr + PositionOffset, end);
        }

        void Loop()
        {
            while (!stopRequested.load()) {
                const uint32_t localPlayer =
                    static_cast<uint32_t>(Context::LocalPlayerAddress);
                if (!localPlayer) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(LoopSleepMs));
                    continue;
                }

                uint32_t matrixPtr = 0;
                if (!ReadMatrix(localPlayer, matrixPtr)) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(LoopSleepMs));
                    continue;
                }

                Vector3 currentPosition{};
                if (!Mem.Read(matrixPtr + PositionOffset, currentPosition)) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(LoopSleepMs));
                    continue;
                }

                if (g_Options.Misc.Exploits.LocalPlayer.UndergroundEnabled &&
                    !isUnderground && !isMoving) {
                    savedPosition = currentPosition;
                    isMoving = true;
                    const Vector3 target{
                        currentPosition.X,
                        currentPosition.Y - ClimbDepth,
                        currentPosition.Z
                    };
                    SmoothMove(matrixPtr, currentPosition, target);
                    isUnderground = true;
                    isMoving = false;
                }
                else if (!g_Options.Misc.Exploits.LocalPlayer.UndergroundEnabled &&
                    isUnderground && !isMoving && savedPosition) {
                    isMoving = true;
                    SmoothMove(matrixPtr, currentPosition, *savedPosition);
                    isUnderground = false;
                    savedPosition.reset();
                    isMoving = false;
                }
                else if (g_Options.Misc.Exploits.LocalPlayer.UndergroundEnabled &&
                    isUnderground && !isMoving && savedPosition) {
                    const Vector3 locked{
                        savedPosition->X,
                        savedPosition->Y - ClimbDepth,
                        savedPosition->Z
                    };
                    for (int i = 0; i < 5; ++i)
                        Mem.Write(matrixPtr + PositionOffset, locked);
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(LoopSleepMs));
            }
        }

        void Start()
        {
            if (running.load())
                return;

            stopRequested.store(false);
            savedPosition.reset();
            isMoving = false;
            isUnderground = false;
            running.store(true);
            workerThread = std::thread(Loop);
        }

        void Stop()
        {
            if (!running.load() && !stopRequested.load())
                return;

            stopRequested.store(true);
            if (workerThread.joinable())
                workerThread.join();

            if (isUnderground && savedPosition) {
                const uint32_t localPlayer =
                    static_cast<uint32_t>(Context::LocalPlayerAddress);
                uint32_t matrixPtr = 0;
                Vector3 currentPosition{};
                if (localPlayer && ReadMatrix(localPlayer, matrixPtr) &&
                    Mem.Read(matrixPtr + PositionOffset, currentPosition)) {
                    stopRequested.store(false);
                    SmoothMove(matrixPtr, currentPosition, *savedPosition);
                    stopRequested.store(true);
                }
            }

            savedPosition.reset();
            isMoving = false;
            isUnderground = false;
            running.store(false);
        }
    }

    void Data::UndergroundThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Misc.Exploits.LocalPlayer.UndergroundEnabled)
                Underground_CppX::Start();
            else
                Underground_CppX::Stop();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        Underground_CppX::Stop();
    }

    void Data::DownPlayerThread()
    {
        while (!g_Options.General.ShutDown) {
            if (!g_Options.Misc.Exploits.LocalPlayer.DownPlayerEnabled ||
                Context::WindowWidth <= 0 || Context::WindowHeight <= 0 ||
                !Context::HasMatrix || Context::LocalPlayerAddress == 0) {
                DownPlayerState::isFiring = false;
                DownPlayerState::RefreshOriginalY();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            DownPlayerState::RefreshOriginalY();

            bool isFiring = false;
            if (!Mem.Read(Context::LocalPlayerAddress + Offsets::sAim1, isFiring) || !isFiring) {
                DownPlayerState::isFiring = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            DownPlayerState::isFiring = true;
            DownPlayerState::AimAtTarget(DownPlayerState::FindBestTarget());
            DownPlayerState::forceRefresh = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        DownPlayerState::isFiring = false;
        DownPlayerState::RefreshOriginalY();
    }

    // ==================== BACK JUMP IMPLEMENTATION ====================
    namespace BackJump_CppX {
        constexpr uint32_t kMinValidPtr = 0x10000;
        constexpr uintptr_t kRotSensMin = 0xFD4;
        constexpr uintptr_t kRotSensMax = 0xFD8;
        constexpr uintptr_t kAimRotSensMin = 0xFDC;
        constexpr uintptr_t kAimRotSensMax = 0xFE0;

        constexpr float kDefRotSensMin = 15.0f;
        constexpr float kDefRotSensMax = 35.0f;
        constexpr float kDefAimRotSensMin = 10.0f;
        constexpr float kDefAimRotSensMax = 20.0f;

        constexpr uint32_t kTelaSlots[] = { 0xABFF414, 0xABFF734, 0xABFF3B8, 0xABFF6D8 };
        constexpr uint32_t kScanWindow = 0x400;
        constexpr uint32_t kRetryMs = 3000;

        inline bool IsValidPtr(uint32_t v) {
            return v >= kMinValidPtr && v != static_cast<uint32_t>(-1);
        }

        inline bool FloatEq(float a, float b) {
            if (a != a || b != b) return false;
            float d = a - b;
            if (d < 0) d = -d;
            return d <= 0.01f * (b > 1.0f ? b : 1.0f);
        }

        inline bool FullFingerprint(uint32_t gv, const char** why) {
            if (!IsValidPtr(gv)) { *why = "invalid ptr"; return false; }
            if (!FloatEq(Mem.ReadS<float>(gv + kRotSensMin), kDefRotSensMin)) { *why = "rotMin"; return false; }
            if (!FloatEq(Mem.ReadS<float>(gv + kRotSensMax), kDefRotSensMax)) { *why = "rotMax"; return false; }
            if (!FloatEq(Mem.ReadS<float>(gv + kAimRotSensMin), kDefAimRotSensMin)) { *why = "aimMin"; return false; }
            if (!FloatEq(Mem.ReadS<float>(gv + kAimRotSensMax), kDefAimRotSensMax)) { *why = "aimMax"; return false; }
            return true;
        }

        inline bool LightFingerprint(uint32_t gv) {
            return IsValidPtr(gv) &&
                   FloatEq(Mem.ReadS<float>(gv + kRotSensMin), kDefRotSensMin) &&
                   FloatEq(Mem.ReadS<float>(gv + kRotSensMax), kDefRotSensMax);
        }

        inline bool ResolveFromSlot(uint32_t slot, uint32_t* outGv, const char** why) {
            uint32_t ti = Mem.ReadS<uint32_t>(Offsets::Il2Cpp + slot);
            if (!IsValidPtr(ti)) { *why = "ti invalid"; return false; }

            uint32_t sf = Mem.ReadS<uint32_t>(ti + Offsets::StaticClass);
            if (FullFingerprint(sf, why)) {
                *outGv = sf;
                return true;
            }

            uint32_t klass = Mem.ReadS<uint32_t>(ti);
            if (IsValidPtr(klass)) {
                uint32_t sf2 = Mem.ReadS<uint32_t>(klass + Offsets::StaticClass);
                if (FullFingerprint(sf2, why)) {
                    *outGv = sf2;
                    return true;
                }
                uint32_t first = Mem.ReadS<uint32_t>(sf2);
                if (FullFingerprint(first, why)) {
                    *outGv = first;
                    return true;
                }
            }
            return false;
        }

        static uint32_t g_gameVar = 0;
        static uint64_t g_lastScanAttempt = 0;

        inline void ScanForGameVar() {
            const char* why = "?";
            uint32_t gv = 0;

            for (uint32_t d = 0; d <= kScanWindow && gv == 0; d += 4) {
                if (ResolveFromSlot((uint32_t)(Offsets::InitBase + d), &gv, &why)) break;
                if (d != 0 && ResolveFromSlot((uint32_t)(Offsets::InitBase - d), &gv, &why)) break;
            }

            for (uint32_t slot : kTelaSlots) {
                if (gv != 0) break;
                ResolveFromSlot(slot, &gv, &why);
            }

            if (gv != 0) {
                g_gameVar = gv;
            }
        }

        inline void Run() {
            static bool lastState = false;
            const bool enabled = g_Options.Misc.Exploits.LocalPlayer.BackJump;

            if (!enabled && !lastState) return;

            if (Offsets::Il2Cpp == 0) {
                lastState = enabled;
                return;
            }

            if (g_gameVar != 0 && !LightFingerprint(g_gameVar)) {
                g_gameVar = 0;
            }

            if (g_gameVar == 0) {
                uint64_t now = GetTickCount64();
                if (now - g_lastScanAttempt >= kRetryMs) {
                    g_lastScanAttempt = now;
                    ScanForGameVar();
                }
            }

            if (g_gameVar == 0) {
                lastState = enabled;
                return;
            }

            if (enabled) {
                bool accelOnFalling = false;
                bool lowFallingSwapWeapon = false;
                Mem.ReadFast2<bool>(g_gameVar + Offsets::EnableAccelerationOnFalling, &accelOnFalling);
                Mem.ReadFast2<bool>(g_gameVar + Offsets::EnableLowFallingSwapWeapon, &lowFallingSwapWeapon);

                if (accelOnFalling == true || lowFallingSwapWeapon == false) {
                    Mem.Write<bool>(g_gameVar + Offsets::EnableAccelerationOnFalling, false);
                    Mem.Write<bool>(g_gameVar + Offsets::EnableLowFallingSwapWeapon, true);
                }
            }
            else {
                if (lastState) {
                    Mem.Write<bool>(g_gameVar + Offsets::EnableAccelerationOnFalling, true);
                    Mem.Write<bool>(g_gameVar + Offsets::EnableLowFallingSwapWeapon, true);
                }
            }

            lastState = enabled;
        }
    }

    void Data::BackJumpThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Hook.IsInjected && Offsets::Il2Cpp != 0 && Context::LocalPlayerAddress != 0) {
                if (g_Options.Misc.Exploits.LocalPlayer.BackJump) {
                    BackJump_CppX::Run();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    /* ==================== INFINITE AMMO IMPLEMENTATION ====================*/
    namespace InfiniteAmmo_CppX {
        static std::mutex ammoMutex;
        static bool ammoWasActive = false;
        static bool defaultReloadNoConsumeAmmoClip = false;

        inline void Run() {
            uint32_t localPlayer = Context::LocalPlayerAddress;
            if (!localPlayer) return;

            uint32_t playerAttr = 0;
            if (!Mem.ReadFast2<uint32_t>(localPlayer + Offsets::LocalPlayerAttributes, &playerAttr) || playerAttr == 0)
                return;

            const uint32_t flagAddr = playerAttr + Offsets::ReloadNoConsumeAmmoClip;

            std::lock_guard<std::mutex> lock(ammoMutex);
            if (g_Options.Misc.Exploits.LocalPlayer.InfiniteAmmo) {
                if (!ammoWasActive) {
                    Mem.ReadFast2<bool>(flagAddr, &defaultReloadNoConsumeAmmoClip);
                }
                Mem.Write<bool>(flagAddr, true);
                ammoWasActive = true;
            }
            else {
                if (ammoWasActive) {
                    Mem.Write<bool>(flagAddr, defaultReloadNoConsumeAmmoClip);
                    ammoWasActive = false;
                }
            }
        }
    }

    void Data::InfiniteAmmoThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Hook.IsInjected && Offsets::Il2Cpp != 0 && Context::LocalPlayerAddress != 0) {
                if (g_Options.Misc.Exploits.LocalPlayer.InfiniteAmmo) {
                    InfiniteAmmo_CppX::Run();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // ==================== SPEED HACK IMPLEMENTATION ====================
    namespace SpeedHack_CppX {
        static std::mutex speedMutex;
        static bool speedWasActive = false;
        static float defaultRunSpeedUpScale = 1.0f;
        static float defaultFallingSpeedUpScale = 1.0f;

        inline void Run() {
            std::lock_guard<std::mutex> lock(speedMutex);

            uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (!localPlayer) return;

            uint32_t playerAttr = 0;
            if (!Mem.ReadFast2<uint32_t>(localPlayer + Offsets::LocalPlayerAttributes, &playerAttr) || playerAttr == 0)
                return;

            const uint32_t runSpeedAddr = playerAttr + Offsets::RunSpeedUpScale;
            const uint32_t fallingSpeedAddr = playerAttr + Offsets::FallingSpeedUpScale;

            if (g_Options.Misc.Exploits.LocalPlayer.speed) {
                if (!speedWasActive) {
                    Mem.ReadFast2<float>(runSpeedAddr, &defaultRunSpeedUpScale);
                    Mem.ReadFast2<float>(fallingSpeedAddr, &defaultFallingSpeedUpScale);
                }

                const float scale = g_Options.Misc.Exploits.LocalPlayer.SpeedMultiplier;
                Mem.Write<float>(runSpeedAddr, scale);
                Mem.Write<float>(fallingSpeedAddr, scale);
                speedWasActive = true;
            }
            else if (speedWasActive) {
                Mem.Write<float>(runSpeedAddr, defaultRunSpeedUpScale);
                Mem.Write<float>(fallingSpeedAddr, defaultFallingSpeedUpScale);
                speedWasActive = false;
            }
        }
    }

    void Data::SpeedHackThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Hook.IsInjected && Offsets::Il2Cpp != 0 && Context::LocalPlayerAddress != 0) {
                if (g_Options.Misc.Exploits.LocalPlayer.speed) {
                    SpeedHack_CppX::Run();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // ==================== VISION HACK IMPLEMENTATION ====================
    namespace VisionHack_CppX {
        static bool visionHackActive = false;

        inline void Run() {
            uint32_t localPlayer = static_cast<uint32_t>(Context::LocalPlayerAddress);
            if (!localPlayer) return;

            uint32_t followCamera = 0;
            if (!Mem.ReadFast2<uint32_t>(localPlayer + Offsets::FollowCamera, &followCamera) || !followCamera)
                return;

            if (g_Options.Misc.Exploits.LocalPlayer.wall) {
                Mem.Write<float>(followCamera + Offsets::FollowCameraFOVOffset, 75.0f);
                visionHackActive = true;
            }
            else if (visionHackActive) {
                Mem.Write<float>(followCamera + Offsets::FollowCameraFOVOffset, 0.0f);
                visionHackActive = false;
            }
        }
    }

    void Data::VisionHackThread() {
        while (!g_Options.General.ShutDown) {
            if (g_Options.Hook.IsInjected && Offsets::Il2Cpp != 0 && Context::LocalPlayerAddress != 0) {
                if (g_Options.Misc.Exploits.LocalPlayer.wall) {
                    VisionHack_CppX::Run();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

