#pragma once

#include <cstdint>
#include <string>

class Offsets {
public:
    // Base Addresses
    static inline uintptr_t Il2Cpp = 0x0; // il2cpp.dll base (runtime resolved)
    static inline uintptr_t UnityCpp = 0x0; // UnityPlayer/UnityCpp base (runtime resolved)
    static inline uintptr_t InitBase = 0xA342EFC; // il2cpp static bootstrap pointer
    static inline uintptr_t StaticClass = 0x5C; // base + StaticClass -> singleton class ptr

    // Match / Game Info
    static inline uintptr_t CurrentMatch = 0x50; // GameFacade -> CurrentMatch
    static inline uintptr_t MatchStatus = 0x8C; // CurrentMatch -> MatchStatus
    static inline uintptr_t LocalPlayer = 0x94; // CurrentMatch -> LocalPlayer
    static inline uintptr_t DictionaryEntities = 0x68; // CurrentGame -> Dictionary<uint, ReplicationEntity>

    // Player
    static inline uintptr_t Player_IsDead = 0x50; // Player -> IsDead
    static inline uintptr_t Player_Name = 0x31C; // Player -> Name (MonoString*)
    static inline uintptr_t Player_Data = 0x48; // Player -> IPRIDataPool m_PRIDataPool
    static inline uintptr_t Player_ShadowBase = 0x1A60; // Player -> ShadowState
    static inline uintptr_t Player_BaseProfileInfo = 0x1A74; // Player -> BaseProfileInfo*
    static inline uintptr_t Player_AccountID = 0x8; // BaseProfileInfo -> AccountID (ulong)
    static inline uintptr_t Profile_Region = 0x10; // BaseProfileInfo -> region string
    static inline uintptr_t Profile_Level = 0x14; // BaseProfileInfo -> level
    static inline uintptr_t Profile_NickName = 0x18; // BaseProfileInfo -> nickname string
    static inline uintptr_t Profile_ClanName = 0x30; // BaseProfileInfo -> clan name string
    static inline uintptr_t Profile_ClanID = 0x38; // BaseProfileInfo -> Clan ID
    static inline uintptr_t Profile_Rank = 0x58; // BaseProfileInfo -> BR rank
    static inline uintptr_t Profile_RankPoint = 0x5C; // BaseProfileInfo -> BR rank points
    static inline uintptr_t Profile_CSRank = 0x64; // BaseProfileInfo -> CS rank
    static inline uintptr_t Profile_CSRankPoint = 0x68; // BaseProfileInfo -> CS rank points
    static inline uintptr_t Profile_GroupMemberCnt = 0x98; // BaseProfileInfo -> team member count
    static inline uintptr_t Profile_GroupMode = 0x9C; // BaseProfileInfo -> 0 Solo, 1 Duo, 2 Squad
    static inline uintptr_t Profile_WorkshopMapName = 0xAC; // BaseProfileInfo -> workshop map string
    static inline uintptr_t Profile_MatchMode = 0xE8; // BaseProfileInfo -> match mode enum
    static inline uintptr_t Profile_GameMode = 0xEC; // BaseProfileInfo -> 0 BR, 1 CS, 2 Lone Wolf
    static inline uintptr_t Profile_MapID = 0xF0; // BaseProfileInfo -> map identifier
    static inline uintptr_t Profile_HasPveTalent = 0xF4; // BaseProfileInfo -> PVE talent flag
    static inline uintptr_t Profile_RoomID = 0xF8; // BaseProfileInfo -> room ID
    static inline uintptr_t Profile_RoomType = 0x100; // BaseProfileInfo -> room type
    static inline uintptr_t Profile_RoomMember = 0x104; // BaseProfileInfo -> room member count
    static inline uintptr_t Profile_RoomMaxMember = 0x108; // BaseProfileInfo -> room max members
    static inline uintptr_t Profile_GroupID = 0x190; // BaseProfileInfo -> group/party ID
    static inline uintptr_t Player_BRRank = Profile_Rank;
    static inline uintptr_t Player_BRRankPoint = Profile_RankPoint;
    static inline uintptr_t Player_BRPeakRankPos = 0x60; // BaseProfileInfo -> PeakRankPos
    static inline uintptr_t Player_UserID = 0x260;
    static inline uintptr_t Player_CSRank = Profile_CSRank;
    static inline uintptr_t Player_CSRankPoint = Profile_CSRankPoint;
    static inline uintptr_t Player_CSPeakRankPos = 0x6C; // BaseProfileInfo -> CSPeakRankPos
    static inline uintptr_t Player_StreakWins = 0x1a4; // BaseProfileInfo -> StreakWins
    static inline uintptr_t XPose = 0x74; // ShadowState -> TargetPhysXPose
    static inline uintptr_t PlayerVitals = 0x10; // Replication value union (health/gun)
    static inline uintptr_t ReplicationDataPool = 0x8; // IPRIDataPool + m_Datas
    static inline uintptr_t ReplicationInt8Handlers = 0x10; // m_Datas + Int8Handlers dictionary
    static inline uintptr_t ReplicationUInt16Handlers = 0x1C; // m_Datas + UInt16Handlers dictionary
    static inline uintptr_t ManagedStringText = 0xC; // Mono string text pointer offset
    static inline uintptr_t ActiveSkillCdReduction = 0x128; // Mono string text pointer offset

    // Avatar
    static inline uintptr_t AvatarManager = 0x504; // Player -> AvatarManager
    static inline uintptr_t Avatar = 0xA8; // AvatarManager -> Avatar
    static inline uintptr_t Avatar_IsVisible = 0x95; // Avatar -> IsVisible
    static inline uintptr_t Avatar_Data = 0x14; // Avatar -> UMAData
    static inline uintptr_t Avatar_Data_IsTeam = 0x59; // UMAData -> isTeammate
    static inline uintptr_t HeadCollider = 0x4E8; // Player -> Head collider ptr
    static inline uintptr_t ReplaceCollider = 0x54; // Target slot used for collider swap
    static inline uintptr_t KillCount = 0x23C; // Player -> KillCount // private float <NCAKNOFBBCL>k__BackingField;
    static inline uintptr_t Killinfo = 0x228; // Player -> KillInfo
    // Camera
    static inline uintptr_t FollowCamera = 0x494; // Legacy project mapping; combo dump shows Player -> m_FollowCamera at 0x3B0
    static inline uintptr_t Camera = 0x18; // FollowCamera -> Camera
    static inline uintptr_t AimRotation = 0x440; // Legacy name; combo dump maps this slot to Player -> _mSightCameraRot
    static inline uintptr_t MainCameraTransform = 0x28C; // Player -> MainCameraTransform
    static inline uintptr_t ViewMatrix = 0xE8; // CameraBase -> ViewMatrix
    static inline uintptr_t CameraBase = 0x8; // Camera + base struct offset
    // Weapon
    static inline uintptr_t Weapon = 0x434; // Player -> ActiveUISightingWeapon
    static inline uintptr_t WeaponData = 0x58; // Weapon -> WeaponData
    static inline uintptr_t WeaponRecoil = 0xc; // WeaponData -> Recoil
    static inline uintptr_t sAim1 = 0x58C;
    static inline uintptr_t sAim2 = 0x9D0;
    static inline uintptr_t sAim3 = 0x38;
    static inline uintptr_t sAim4 = 0x2C;
    static inline uintptr_t LocalPlayerAttributes = 0x500; // Player -> PlayerAttributes
    static inline uintptr_t m_FireIntervalScale = 0x1C4; // PlayerAttributes -> 
    static inline uintptr_t FireIntervalScaleTwo = 0x1D4; // PlayerAttributes -> FireIntervalScale

    static inline uintptr_t NoReload = 0xC1; // PlayerAttributes -> ShootNoReload
    // static inline uintptr_t RunSpeedUpScale = 0x1C8; // PlayerAttributes -> run speed scalar
   //  static inline uintptr_t FallingSpeedUpScale = 0x1C4; // PlayerAttributes -> fall speed scalar
    static inline uintptr_t BuffWeaponAmmoClip = 0xD0; // PlayerAttributes -> fall speed scalar
    static inline uintptr_t PlayerMoveComponent = 0x124C; // Player -> movement component
    static inline uintptr_t MoveComponentGravity = 0x2C; // MovementComponent -> gravity scalar
    static inline uintptr_t MoveingDirectionScout = 0x40; // PhysXData -> MoveingDirectionScout (Vector3 starts here)
    static inline uintptr_t WeaponSwitchDelayA = 0x48; // Weapon -> switch delay A
    static inline uintptr_t WeaponSwitchDelayB = 0x4C; // Weapon -> switch delay B
    static inline uintptr_t BeHealingIncreaseRatio = 0x158; // PlayerAttributes -> m_beHealingIncreaseRatio
    static inline uintptr_t HealingIncreaseRatio = 0x15C; // PlayerAttributes -> m_healingIncreaseRatio // private float PABBILDAGPA; 
    static inline uintptr_t BuffWeaponDamageScale = 0xBC;
    static inline uintptr_t DamageAdditionScale = 0xAC;
    static inline uintptr_t ExecuteDamageScale = 0xB4;
    static inline uintptr_t NoHitDelayPrimary = 0x48C; // Player -> no-hit-delay chain ptr
    static inline uintptr_t NoHitDelaySecondary = 0x58; // no-hit-delay chain -> value holder
    static inline uintptr_t NoHitDelayValue = 0x20C; // no-hit-delay value field

    // Transform chain internals (used by Data.cpp)
    static inline uintptr_t TransformLink = 0x8; // node -> transform / transform -> transform object
    static inline uintptr_t TransformMatrix = 0x20; // transform object -> matrix
    static inline uintptr_t MatrixPosition = 0x80; // matrix -> position (Vector3)
    static inline uintptr_t MatrixRotation = 0x90; // matrix -> rotation (Quaternion)

    // Extras
    static inline uintptr_t IsClientBot = 0x324; // Player -> IsClientBot
    static inline uintptr_t FastSwitch = 0x4BC; // Player -> weapon switch speed field
    static inline uintptr_t FastMedkit = 0x88; // PlayerAttributes -> medkit speed
    static inline uintptr_t InfiniteAmmo = 0x90; // PlayerAttributes -> ammo consume flag
    static inline uintptr_t GhostHack = 0x4C0; // Player -> ghost/sync flag

    // Underground movement / PhysX data
    static inline uintptr_t UndergroundPhysXData = 0x14E8; // Player -> physxData
    static inline uintptr_t UndergroundPositionX = 0x20; // physxData -> position.x
    static inline uintptr_t UndergroundPositionY = 0x24; // physxData -> position.y
    static inline uintptr_t UndergroundPositionZ = 0x28; // physxData -> position.z
    static inline uintptr_t UndergroundGroundHitResult = 0x1554; // Player -> ground hit result

    static inline uintptr_t ReloadNoConsumeAmmoClip = 0xC0; 
    static inline uintptr_t RunSpeedUpScale = 0x210;
    static inline uintptr_t FallingSpeedUpScale = 0x20C;
    // Timer
    static inline uintptr_t FixedDeltaTime = 0x24; // GameTimer -> fixedDeltaTime
    static inline uintptr_t GameTimer = 0x10; // CurrentGame -> GameTimer manager
    static inline uintptr_t DictionaryEntitiesList = 0xC;   // entries pointer
    static inline uintptr_t DictionaryEntitiesCount = 0x10;  // count
    static inline uintptr_t DictionaryEntitiesStart = 0x10;  // entries + 0x10
    static inline uintptr_t DictionaryEntityStride = 0x10;  // each entry size
    static inline uintptr_t DictionaryEntityObject = 0xC;   // entity inside entry
    static inline uintptr_t DictionaryEntityHash = 0x0;   // hash
    static inline uintptr_t GameVarDefTypeInfo = 0xA545028; // il2cpp -> COW.GameVarDef_TypeInfo
    static inline uintptr_t GameVarDefToggle = 0x5E0; // GameVarDef -> glitch fire toggle field

    static inline uintptr_t EnableAccelerationOnFalling = 0x28F3; // Player.EnableAccelerationOnFalling (bool)
    static inline uintptr_t EnableLowFallingSwapWeapon = 0x2C3D; // Player.EnableLowFallingSwapWeapon (bool)
    static inline uintptr_t FollowCameraFOVOffset = 0x48; // FollowCamera -> FOV offset


    // Bones
    struct Bones {
        static inline uintptr_t Root = 0x4A8; // Player -> Root transform node (m_RootNode)
        static inline uintptr_t RootBone = 0x4AC; // Player -> Secondary root bone node (m_BoneRootNode)

        static inline uintptr_t Head = 0x49C; // Head node
        static inline uintptr_t Neck = 0x4A0; // Hip node (Neck ka direct field nahi, Hip se map)
        static inline uintptr_t Hip = 0x4A0; // Hip node
        static inline uintptr_t HipCenter = 0x4A0; // Hip center (same as Hip)

        static inline uintptr_t ShoulderLeft = 0x4D0; // LeftShoulder
        static inline uintptr_t ShoulderRight = 0x4D4; // RightShoulder

        static inline uintptr_t ElbowLeft = 0x4E4; // LeftElbow
        static inline uintptr_t ElbowRight = 0x4E0; // RightElbow

        static inline uintptr_t WristLeft = 0x4DC; // LeftWrist
        static inline uintptr_t WristRight = 0x4D8; // RightWrist

        static inline uintptr_t AnkleLeft = 0x4B8; // LeftAnkle
        static inline uintptr_t AnkleRight = 0x4BC; // RightAnkle

        static inline uintptr_t ToeLeft = 0x4C0; // LeftToe
        static inline uintptr_t ToeRight = 0x4C4; // RightToe
    };

};



class JuwonWeapon {
public:
    static std::string NameWeapon(int index) {
        switch (index) {
        case 0: return "AK47";
        case 1: return "FIST";
        case 2: return "M4A1";
        case 3: return "USP";
        case 4: return "AWM";
        case 5: return "M1014";
        case 6: return "AK47";
        case 7: return "UMP";
        case 8: return "MP5";
        case 9: return "DESERT-EAGLE";
        case 10: return "G18";
        case 11: return "M14";
        case 12: return "SCAR";
        case 13: return "VSS";
        case 14: return "GROZA";
        case 15: return "MP40";
        case 16: return "PAN";
        case 17: return "PARANG";
        case 18: return "SKS";
        case 19: return "M249";
        case 20: return "M1873";
        case 21: return "KAR98K";
        case 22: return "CROSSBOW";
        case 23: return "M79";
        case 24: return "FAMAS";
        case 25: return "M500";
        case 26: return "SVD";
        case 27: return "BAT";
        case 28: return "XM8";
        case 29: return "SPAS12";
        case 30: return "M60";
        case 32: return "P90";
        case 33: return "AN94";
        case 34: return "KATANA";
        case 35: return "CG15";
        case 36: return "RGS-50";
        case 37: return "GATLING";
        case 38: return "HOOK-GUN";
        case 39: return "PLASMA";
        case 41: return "M1887";
        case 43: return "THOMPSON";
        case 45: return "M82B";
        case 46: return "AUG";
        case 47: return "PARAFAL";
        case 48: return "WOODPECKER";
        case 49: return "VECTOR";
        case 50: return "MAG-7";
        case 51: return "SCYTHE";
        case 54: return "KORD";
        case 55: return "M1917";
        case 56: return "USP-2";
        case 57: return "KINGFISHER";
        case 58: return "MINI-UZI";
        case 60: return "MP5-I";
        case 61: return "M60-I";
        case 62: return "VSS-I";
        case 63: return "M14-I";
        case 64: return "KAR98K-I";
        case 65: return "AWM-Y";
        case 67: return "FAMAS-I";
        case 70: return "GROZA-X";
        case 71: return "M249-X";
        case 72: return "SVD-Y";
        case 73: return "G36-ASSAULT";
        case 74: return "G36-RANGE";
        case 75: return "M24";
        case 78: return "HEAL-SNIPER";
        case 80: return "M4A1-I";
        case 81: return "M4A1-II";
        case 82: return "M4A1-III";
        case 86: return "CHARGE-BUSTER";
        case 88: return "MAC10";
        case 89: return "AC80";
        case 93: return "HEAL-PISTOL";
        case 98: return "CG15";
        case 99: return "SHIELD-GUN";
        case 100: return "FLAMETHROWER";
        case 119: return "M1887-X";
        case 120: return "MP5-II";
        case 121: return "MP5-III";
        case 122: return "M60-II";
        case 123: return "M60-III";
        case 124: return "VSS-II";
        case 125: return "VSS-III";
        case 126: return "M14-II";
        case 127: return "M14-III";
        case 128: return "KAR98K-II";
        case 129: return "KAR98K-III";
        case 130: return "FAMAS-II";
        case 131: return "FAMAS-III";
        case 149: return "FF-KNIFE";
        case 150: return "BIZON";
        case 178: return "SCAR-I";
        case 179: return "SCAR-II";
        case 180: return "SCAR-III";
        case 181: return "TROGON";
        case 184: return "M1014-I";
        case 185: return "M1014-II";
        case 186: return "M1014-III";
        case 193: return "AUG-I";
        case 194: return "AUG-II";
        case 195: return "AUG-III";
        case 197: return "VSK94";
        case 21001: return "HEAL-PISTOL-Y";
        case 21002: return "M590";
        case 228: return "MAC10-I";
        case 229: return "MAC10-II";
        case 230: return "MAC10-III";
        case 601: return "GRENADE";
        case 602: return "FLASHBANG";
        case 603: return "SMOKE-GRENADE";
        case 607: return "MAGNETIC-SHIELD";
        case 608: return "FLASH-FREEZE";
        case 617: return "GLOO-MELTER";
        case 1006: return "UAV-LITE";
        case 1201: return "GLOO-WALL";
        case 1202: return "BRICK-GRENADE";
        case 1401: return "LANDMINE";
        case 2102: return "MINI-TURRET";
        case 2401: return "DECOY-GRENADE";
        case 2501: return "FIST";
        case 2502: return "KNIFE";
        case 2503: return "MOLOTOV";
        case 2504: return "STICKY-GRENADE";
        case 2505: return "CLUSTER-GRENADE";
        case 2506: return "FLASHBANG-II";
        case 2507: return "SMOKE-GRENADE-II";
        case 2508: return "HEAL-GRENADE";
        case 2509: return "POISON-GRENADE";
        case 2510: return "EMP-GRENADE";
        case 2511: return "THERMAL-GRENADE";
        case 2512: return "CONCUSSION-GRENADE";
        case 2513: return "INCENDIARY-GRENADE";
        case 2514: return "FRAG-GRENADE";
        case 2515: return "STUN-GRENADE";
        case 2516: return "GAS-GRENADE";
        case 2517: return "SHOCK-GRENADE";
        case 2518: return "FREEZE-GRENADE";
        case 2519: return "ACID-GRENADE";
        case 2520: return "EXPLOSIVE-GRENADE";
        case 3001: return "ROCKET-LAUNCHER";
        case 3002: return "MISSILE-LAUNCHER";
        case 3003: return "GRENADE-LAUNCHER";
        case 3004: return "FLAME-THROWER-II";
        case 3005: return "LASER-CANNON";
        case 3006: return "PLASMA-CANNON";
        case 3007: return "ION-CANNON";
        case 3008: return "GRAVITY-CANNON";
        case 3009: return "FREEZE-CANNON";
        case 3010: return "SHOCK-CANNON";
        case 3011: return "POISON-CANNON";
        case 3012: return "ACID-CANNON";
        case 3013: return "FIRE-CANNON";
        case 3014: return "ICE-CANNON";
        case 3015: return "LIGHTNING-CANNON";
        case 3016: return "WIND-CANNON";
        case 3017: return "EARTH-CANNON";
        case 3018: return "WATER-CANNON";
        case 3019: return "METAL-CANNON";
        case 3020: return "ENERGY-CANNON";
        case 4001: return "MELEE-WEAPON";
        case 4002: return "SWORD";
        case 4003: return "AXE";
        case 4004: return "HAMMER";
        case 4005: return "MACE";
        case 4006: return "SPEAR";
        case 4007: return "STAFF";
        case 4008: return "CLUB";
        case 4009: return "WHIP";
        case 4010: return "CHAIN";
        case 4011: return "SHIELD";
        case 4012: return "ARMOR";
        case 4013: return "HELMET";
        case 4014: return "GLOVES";
        case 4015: return "BOOTS";
        case 4016: return "BELT";
        case 4017: return "RING";
        case 4018: return "AMULET";
        case 4019: return "POTION";
        case 4020: return "SCROLL";
        case 5001: return "SPECIAL-WEAPON";
        case 5002: return "ULTIMATE-WEAPON";
        case 5003: return "LEGENDARY-WEAPON";
        case 5004: return "MYTHIC-WEAPON";
        case 5005: return "GOD-WEAPON";
        case 5006: return "DEMON-WEAPON";
        case 5007: return "ANGEL-WEAPON";
        case 5008: return "DRAGON-WEAPON";
        case 5009: return "PHOENIX-WEAPON";
        case 5010: return "UNICORN-WEAPON";
        case 5011: return "PEGASUS-WEAPON";
        case 5012: return "GRIFFIN-WEAPON";
        case 5013: return "CENTAUR-WEAPON";
        case 5014: return "MINOTAUR-WEAPON";
        case 5015: return "SATYR-WEAPON";
        case 5016: return "FAUN-WEAPON";
        case 5017: return "NYMPH-WEAPON";
        case 5018: return "DRYAD-WEAPON";
        case 5019: return "NAIAD-WEAPON";
        case 5020: return "OCEANID-WEAPON";
        default: {
            return "";
        }
        }
    }
};
