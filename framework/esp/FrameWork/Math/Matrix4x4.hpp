#pragma once

typedef struct _Matrix4x4
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
    };

    void TransposeThisMatrix()
    {
        _Matrix4x4 TransposedMatrix;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                TransposedMatrix.m[i][j] = m[j][i];
            }
        }

        *this = TransposedMatrix;
    }

    _Matrix4x4 Transpose()
    {
        _Matrix4x4 TransposedMatrix;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                TransposedMatrix.m[i][j] = m[j][i];
            }
        }

        return TransposedMatrix;
    }

} Matrix4x4;