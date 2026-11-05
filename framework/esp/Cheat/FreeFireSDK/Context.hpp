#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <Windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <FrameWork/Math/Matrix4x4.hpp>
#include <FrameWork/Math/Entity.hpp>
#include <FrameWork/Math/Vectors/Vector3.hpp>
#include <mutex>

class Context {
public:
    static inline uint32_t LibraryAddress = 0;
    static inline bool HasMatrix = false;
    static inline Matrix4x4 ViewMatrix{};
    static inline std::unordered_map<uint32_t, Entity> Entities;
    static inline std::unordered_map<uint32_t, Entity> LastValidEntities;
    std::unordered_map<uint32_t, Entity> emptyMap;
    static inline std::mutex EntityMutex;
    static inline std::mutex EntitiesMutex;
    static inline int MatchStatus = 0;
    static inline Vector3 LocalMainCamera{};
    static inline uint32_t LocalPlayerAddress = 0;
    static inline uint32_t LocalPlayerOrObserver = 0;
    static inline uint32_t CurrentMatch = 0;
    static inline bool LocalPlayerInMatch;
    static inline uint64_t LocalUserID = 0;
    static inline bool LocalProfileValid = false;
    static inline uint64_t LocalClanID = 0;
    static inline uint64_t LocalGroupID = 0;
    static inline uint64_t LocalRoomID = 0;
    static inline uint32_t LocalRegionID = 0;
    static inline std::string LocalRegion;
    static inline std::string LocalNickName;
    static inline std::string LocalClanName;
    static inline std::string LocalWorkshopMapName;
    static inline uint32_t LocalLevel = 0;
    static inline uint32_t LocalGroupMode = 0;
    static inline uint32_t LocalGroupMemberCount = 0;
    static inline uint32_t LocalGameMode = 0;
    static inline uint32_t LocalMatchMode = 0;
    static inline uint32_t LocalMapID = 0;
    static inline bool LocalHasPveTalent = false;
    static inline uint32_t LocalRoomType = 0;
    static inline uint32_t LocalRoomMember = 0;
    static inline uint32_t LocalRoomMaxMember = 0;
    static inline int LocalBRRank = 0;
    static inline int LocalBRRankPoint = 0;
    static inline int LocalCSRank = 0;
    static inline int LocalCSRankPoint = 0;
    static inline int WindowWidth = 0;
    static inline int WindowHeight = 0;
    static inline float ViewYaw = 0.0f;

    // Set true by emu_connect once BstkVMM hook + libil2cpp.so are ready.
    static inline bool IsReady = false;

};

enum MatchStates {
    NotStarted = 0,
    InMatch,
    Ended
};

inline MatchStates MatchStatus = MatchStates::NotStarted;

inline std::string GetRegionName(uint32_t id, const std::string& raw = "")
{
    if (!raw.empty() && raw.length() < 30)
    {
        bool allPrintable = true;
        for (char c : raw) {
            if (c < 32 || c > 126) { allPrintable = false; break; }
        }
        if (allPrintable)
            return raw;
    }

    switch (id)
    {
    case 1: return "BR";
    case 2: return "US";
    case 3: return "SAC";
    case 4: return "IND";
    case 5: return "ID";
    case 6: return "SG";
    case 7: return "RU";
    case 8: return "TH";
    case 9: return "VN";
    case 10: return "TW";
    case 11: return "ME";
    case 12: return "PK";
    case 13: return "BD";
    case 14: return "NA";
    case 15: return "EU";
    default:
        if (id > 0 && id < 100) {
            char b[32];
            std::snprintf(b, sizeof(b), "Region %u", id);
            return b;
        }
        return "Unknown";
    }
}

struct VirtualMemoryFunctions {
    using PGMPhysReadType = int(__cdecl*)(void*, uintptr_t, void*, size_t);
    using PGMPhysSimpleWriteGCPhysType = int(__cdecl*)(void*, uintptr_t, void*, size_t);
    using PGMPhysGCPtr2GCPhysType = int(__cdecl*)(void*, uintptr_t, uintptr_t*);
    using VMMGetCpuByIdType = void* (__cdecl*)(void*, int);

    static inline PGMPhysReadType physRead = nullptr;
    static inline PGMPhysReadType tempPhysRead = nullptr;
    static inline PGMPhysSimpleWriteGCPhysType physWrite = nullptr;
    static inline PGMPhysGCPtr2GCPhysType physConvert = nullptr;
    static inline VMMGetCpuByIdType physCpu = nullptr;
    static inline std::unordered_map<uintptr_t, uintptr_t> cache;
};

struct FreeFireContext {
    bool SDKReady = false;
    static inline VirtualMemoryFunctions VirtualMemory;
};

inline FreeFireContext g_FreeFireContext;

#endif
