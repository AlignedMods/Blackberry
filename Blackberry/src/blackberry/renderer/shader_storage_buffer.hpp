#pragma once

#include "blackberry/core/types.hpp"

namespace Blackberry {

    struct ShaderStorageBuffer {
        [[nodiscard]] static ShaderStorageBuffer Create(u32 binding);

        void ReserveMemory(u32 size, void* data = nullptr);
        
        void* MapMemory() const;
        void UnMapMemory() const;

        template <typename T>
        T GetMappedMemory() const {
            return reinterpret_cast<T>(MapMemory());
        }

        u32 ID = 0;
        u32 Binding = 0;
        u32 Size = 0;
    };

} // namespace Blackberry