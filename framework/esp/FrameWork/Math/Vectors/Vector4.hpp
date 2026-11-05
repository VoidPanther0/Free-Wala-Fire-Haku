#pragma once

#include <cmath>
#include <ostream>
#include <FrameWork/FrameWork.hpp>

class Vector4
{
public:
    // Members 
    float x, y, z, w;

    Vector4(void) { }
    Vector4(float X, float Y, float Z, float W) { x = X; y = Y; z = Z; w = W; }
    friend std::ostream& operator<<(std::ostream& os, const Vector4& vec) {
        os << ("Vector4(") << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
        return os;
    }
    Vector4 operator + (const Vector4& rhs) const { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
};