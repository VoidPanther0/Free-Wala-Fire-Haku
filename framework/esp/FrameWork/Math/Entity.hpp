#pragma once
#include <string>
#include <FrameWork/Math/Vectors/Vector3.hpp>
#include <chrono>
enum class Bool3 {
    True,
    False,
    Unknown
};

class Entity {
public:
    bool IsKnown;
    bool IsBot;
    Bool3 IsTeam;
    bool IsDead;
    bool IsKnocked;
    short Health;
    float Distance;
    bool IsVisible;
    uint32_t Address;
    Vector3 Root;
    Vector3 Head;
    Vector3 Neck;
    Vector3 Hip;
    Vector3 ShoulderLeft;
    Vector3 ShoulderRight;
    Vector3 ElbowLeft;
    Vector3 ElbowRight;
    Vector3 WristLeft;
    Vector3 WristRight;
    Vector3 HipCenter;
    Vector3 AnkleRight;
    Vector3 AnkleLeft;
    Vector3 ToeLeft;
    Vector3 ToeRight;
    std::string Name;
    int Level;
    std::string WeaponName;
    int WeaponIndex;
    uint64_t UserID;
    std::chrono::steady_clock::time_point lastSeen;


};
