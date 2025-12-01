#pragma once

#include "blackberry/core/types.hpp"

namespace Blackberry {

    struct ShaderStorageBuffer {
        [[nodiscard]] static ShaderStorageBuffer Create(u32 binding);

        void ReserveMemory(u32 size) const;

        void* MapMemory();
        void UnMapMemory();

        u32 ID = 0;
        u32 Binding = 0;
    };

} // namespace Blackberry