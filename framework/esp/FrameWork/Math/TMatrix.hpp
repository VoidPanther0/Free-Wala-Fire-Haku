#pragma once

#include <iostream>
#include <FrameWork/Math/Quaternion.hpp>
#include <FrameWork/FrameWork.hpp>
#include <FrameWork/Math/Vectors/Vector4.hpp>

struct TMatrix {

    Vector4 position;
    Quaternion rotation;
    Vector4 scale;

    friend std::ostream& operator<<(std::ostream& os, const TMatrix& matrix) {
        os << ("TMatrix {\n")
            << ("  Position: ") << matrix.position << ",\n"
            << ("  Rotation: ") << matrix.rotation << ",\n"
            << ("  Scale: ") << matrix.scale << "\n"
            << ("}");
        return os;
    }
};
