#pragma once

#include "blackberry/core/types.hpp"

#include <string>

namespace Blackberry {

    struct Shader {
        static Shader Create(const std::string& vert, const std::string& frag);
        void Delete();
    
        void SetFloat(const std::string& uniform, f32 val);
        void SetIntArray(const std::string& uniform, u32 count, int* array);
        void SetVec2(const std::string& uniform, BlVec2<f32> val);
        void SetVec3(const std::string& uniform, BlVec3<f32> val);
        void SetMatrix(const std::string& uniform, f32* mat);
    
        u32 ID = 0;
    };

} // namespace Blackberry