#include "shader_storage_buffer.hpp"
#include "blackberry/core/util.hpp"

#include "glad/gl.h"

namespace Blackberry {

    ShaderStorageBuffer ShaderStorageBuffer::Create(u32 binding) {
        ShaderStorageBuffer buf;
        glGenBuffers(1, &buf.ID);

        buf.Binding = binding;

        return buf;
    }

    void ShaderStorageBuffer::ReserveMemory(u32 size) const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);

        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding, ID);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void* ShaderStorageBuffer::MapMemory() const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);

        void* mapped = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return mapped;
    }

    void ShaderStorageBuffer::UnMapMemory() const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

} // namespace Blackberry