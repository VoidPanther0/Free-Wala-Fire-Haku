#ifndef W2S_HPP
#define W2S_HPP
#pragma once
#include <FrameWork/Math/Vectors/Vector2.hpp>
#include <FrameWork/Math/Vectors/Vector3.hpp>
#include <FrameWork/Math/Matrix4x4.hpp>
#include "Vectors/Vector4.hpp"
#include <optional>

class W2S {
public:
    static Vector2 WorldToScreen(const Matrix4x4& viewMatrix, const Vector3& pos, int WindowWidth, int WindowHeight) {
        Vector2 result(-1, -1);

        if (WindowWidth <= 0 || WindowHeight <= 0) {
            return result;
        }

        float v9 = (pos.X * viewMatrix._11) + (pos.Y * viewMatrix._21) + (pos.Z * viewMatrix._31) + viewMatrix._41;
        float v10 = (pos.X * viewMatrix._12) + (pos.Y * viewMatrix._22) + (pos.Z * viewMatrix._32) + viewMatrix._42;
        float v12 = (pos.X * viewMatrix._14) + (pos.Y * viewMatrix._24) + (pos.Z * viewMatrix._34) + viewMatrix._44;

        if (v12 >= 0.001f) {
            float v13 = WindowWidth / 2.0f;
            float v14 = WindowHeight / 2.0f;

            result.X = v13 + (v13 * v9) / v12;
            result.Y = v14 - (v14 * v10) / v12;
        }

        return result;
    }

    static ImVec2 WorldToScreenImVec2(const Matrix4x4& viewMatrix, const Vector3& pos, int width, int height) {
        Vector2 result = WorldToScreen(viewMatrix, pos, width, height);
        return ImVec2(result.X, result.Y);
    }
};


#endif // W2S_HPP