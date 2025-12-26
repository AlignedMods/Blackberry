#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/memory.hpp"

#include <initializer_list>

namespace Blackberry {

    enum class BufferUsage {
        Static,
        Dynamic,
        Stream
    };

    enum class ShaderDataType {
        Float,
        Float2,
        Float3,
        Float4,
        Int
    };

    struct VertexArrayLayout {
        u32 Location = 0; // Shader location
        ShaderDataType Type = ShaderDataType::Float; // type of each element
        const char* Name = nullptr; // Only here for readability, not used for anything
    };

    struct VertexBuffer {
        static Ref<VertexBuffer> Create(BufferUsage usage);
        static Ref<VertexBuffer> Create(void* vertices, u32 size, u32 count, BufferUsage usage);
        ~VertexBuffer();

        void UpdateData(void* vertices, u32 size, u32 count);

        u32 ID = 0;

        u32 Count = 0;
        BufferUsage Usage;
    };

    struct IndexBuffer {
        static Ref<IndexBuffer> Create(BufferUsage usage);
        static Ref<IndexBuffer> Create(u32* indices, u32 size, u32 count, BufferUsage usage);
        ~IndexBuffer();

        void UpdateData(u32* indices, u32 size, u32 count);

        u32 ID = 0;

        u32 Count = 0;
        BufferUsage Usage;
    };

    struct VertexArray {
        static Ref<VertexArray> Create();
        ~VertexArray();

        void SetVertexBuffer(Ref<VertexBuffer> buffer);
        void SetIndexBuffer(Ref<IndexBuffer> buffer);

        void SetVertexLayout(const std::initializer_list<VertexArrayLayout>& layout);

        Ref<VertexBuffer>& GetVertexBuffer();
        const Ref<VertexBuffer>& GetVertexBuffer() const;

        Ref<IndexBuffer>& GetIndexBuffer();
        const Ref<IndexBuffer>& GetIndexBuffer() const;

        bool HasIndexBuffer() const;

        u32 ID = 0;

    private:
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
    };

} // namespace Blackberry