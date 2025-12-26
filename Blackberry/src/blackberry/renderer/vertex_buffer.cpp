#include "blackberry/renderer/vertex_buffer.hpp"

#include "glad/gl.h"

namespace Blackberry {

    GLenum GetOpenGLBufferUsage(BufferUsage usage) {
        switch (usage) {
            case BufferUsage::Static: return GL_STATIC_DRAW;
            case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
            case BufferUsage::Stream: return GL_STREAM_DRAW;
        }
    }

    Ref<VertexBuffer> VertexBuffer::Create(BufferUsage usage) {
        Ref<VertexBuffer> buffer = CreateRef<VertexBuffer>();

        glCreateBuffers(1, &buffer->ID);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->ID);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GetOpenGLBufferUsage(usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        buffer->Usage = usage;

        return buffer;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void* vertices, u32 size, u32 count, BufferUsage usage) {
        Ref<VertexBuffer> buffer = CreateRef<VertexBuffer>();

        glCreateBuffers(1, &buffer->ID);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->ID);
        glBufferData(GL_ARRAY_BUFFER, size * count, vertices, GetOpenGLBufferUsage(usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        buffer->Count = count;
        buffer->Usage = usage;

        return buffer;
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &ID);
    }

    void VertexBuffer::UpdateData(void* vertices, u32 size, u32 count) {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, size * count, vertices, GetOpenGLBufferUsage(Usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        Count = count;
    }

    Ref<IndexBuffer> IndexBuffer::Create(BufferUsage usage) {
        Ref<IndexBuffer> buffer = CreateRef<IndexBuffer>();

        glCreateBuffers(1, &buffer->ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GetOpenGLBufferUsage(usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        buffer->Usage = usage;

        return buffer;
    }

    Ref<IndexBuffer> IndexBuffer::Create(u32* indices, u32 size, u32 count, BufferUsage usage) {
        Ref<IndexBuffer> buffer = CreateRef<IndexBuffer>();

        glCreateBuffers(1, &buffer->ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * count, indices, GetOpenGLBufferUsage(usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        buffer->Count = count;
        buffer->Usage = usage;

        return buffer;
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &ID);
    }

    void IndexBuffer::UpdateData(u32* indices, u32 size, u32 count) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * count, indices, GetOpenGLBufferUsage(Usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        Count = count;
    }

    Ref<VertexArray> VertexArray::Create() {
        Ref<VertexArray> array = CreateRef<VertexArray>();

        glGenVertexArrays(1, &array->ID);

        return array;
    }

    VertexArray::~VertexArray() {
        glDeleteBuffers(1, &ID);
    }

    void VertexArray::SetVertexBuffer(Ref<VertexBuffer> buffer) {
        glBindVertexArray(ID);

        m_VertexBuffer = buffer;
        glBindBuffer(GL_ARRAY_BUFFER, buffer->ID);

        glBindVertexArray(0);
    }

    void VertexArray::SetIndexBuffer(Ref<IndexBuffer> buffer) {
        glBindVertexArray(ID);

        m_IndexBuffer = buffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ID);

        glBindVertexArray(0);
    }

    void VertexArray::SetVertexLayout(const std::initializer_list<VertexArrayLayout>& layout) {
        glBindVertexArray(ID);

        u32 stride = 0;
        u32 offset = 0;
        for (auto& l : layout) {
            u32 size = 0;

            switch (l.Type) {
                case ShaderDataType::Float:
                    size = sizeof(GLfloat);
                    break;
                case ShaderDataType::Float2:
                    size = sizeof(GLfloat) * 2;
                    break;
                case ShaderDataType::Float3:
                    size = sizeof(GLfloat) * 3;
                    break;
                case ShaderDataType::Float4:
                    size = sizeof(GLfloat) * 4;
                    break;
                case ShaderDataType::Int:
                    size = sizeof(GLint);
                    break;
            }

            stride += size;
        }

        for (auto& l : layout) {
            GLenum type = GL_FLOAT;
            u32 size = 0;
            u32 count = 0;

            switch (l.Type) {
                case ShaderDataType::Float:
                    type = GL_FLOAT;
                    size = sizeof(GLfloat);
                    count = 1;
                    break;
                case ShaderDataType::Float2:
                    type = GL_FLOAT;
                    size = sizeof(GLfloat) * 2;
                    count = 2;
                    break;
                case ShaderDataType::Float3:
                    type = GL_FLOAT;
                    size = sizeof(GLfloat) * 3;
                    count = 3;
                    break;
                case ShaderDataType::Float4:
                    type = GL_FLOAT;
                    size = sizeof(GLfloat) * 4;
                    count = 4;
                    break;
                case ShaderDataType::Int:
                    type = GL_INT;
                    size = sizeof(GLint);
                    count = 1;
                    break;
            }

            if (l.Type == ShaderDataType::Int) {
                glVertexAttribIPointer(l.Location, size, type, stride, reinterpret_cast<void*>(offset));
                glEnableVertexAttribArray(l.Location);
            } else {
                glVertexAttribPointer(l.Location, count, type, GL_FALSE, stride, reinterpret_cast<void*>(offset));
                glEnableVertexAttribArray(l.Location);
            }
            
            offset += size;
        }

        glBindVertexArray(0);
    }

    Ref<VertexBuffer>& VertexArray::GetVertexBuffer() {
        return m_VertexBuffer;
    }

    const Ref<VertexBuffer>& VertexArray::GetVertexBuffer() const {
        return m_VertexBuffer;
    }

    Ref<IndexBuffer>& VertexArray::GetIndexBuffer() {
        return m_IndexBuffer;
    }

    const Ref<IndexBuffer>& VertexArray::GetIndexBuffer() const {
        return m_IndexBuffer;
    }

    bool VertexArray::HasIndexBuffer() const {
        return m_IndexBuffer; // This looks so cursed lol
    }

} // namespace Blackberry