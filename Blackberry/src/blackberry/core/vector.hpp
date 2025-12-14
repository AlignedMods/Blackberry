#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/util.hpp"

#include <cstring>

namespace Blackberry {

    template <typename T>
    class Vector {
    public:
        // Iterators
        using iterator = T*;
        using const_iterator = const T*;

        Vector() {
            Alloc(1);
        }

        Vector(const u32 capacity) {
            Alloc(capacity);
        }

        Vector(const Vector<T>& other) {
            delete[] m_Data;

            m_Size = other.m_Size;
            m_Capacity = other.m_Capacity;
            m_Data = new T[m_Capacity];

            std::memcpy(m_Data, other.m_Data, m_Capacity);
        }

        ~Vector() {
            m_Size = 0;
            m_Capacity = 0;
            delete[] m_Data;
            m_Data = nullptr;
        }

        Vector& operator=(const Vector& other) {
            if (this != &other) {
                delete[] m_Data;

                m_Size = other.m_Size;
                m_Capacity = other.m_Capacity;
                m_Data = new T[m_Capacity];

                std::memcpy(m_Data, other.m_Data, m_Capacity);
            }

            return *this;
        }

        void Reserve(const u32 capacity) { Realloc(capacity); }

        T& At(const u32 index) {
            VerifyInRange(index);
            return m_Data[index];
        }

        const T& At(const u32 index) const {
            VerifyInRange(index);
            return m_Data[index];
        }

        T& operator[](const u32 index) {
            VerifyInRange(index);
            return m_Data[index];
        }

        const T& operator[](const u32 index) const {
            VerifyInRange(index);
            return m_Data[index];
        }

        T& Front() {
            VerifyNotEmpty();
            return m_Data[0];
        }

        const T& Front() const {
            VerifyNotEmpty();
            return m_Data[0];
        }

        T& Back() {
            VerifyNotEmpty();
            return m_Data[m_Size - 1];
        }

        const T& Back() const {
            VerifyNotEmpty();
            return m_Data[m_Size - 1];
        }

        T* Data() { return m_Data; }
        const T* Data() const { return m_Data; }

    private:
        // Allocate the buffer with a given capacity (NOTE: should ONLY be called on construction)
        void Alloc(const u32 capacity) {
            m_Data = new T[capacity]{};
            m_Capacity = capacity;
        }

        // Reallocate the buffer with a given capacity (NOTE: should NOT be called before Alloc())
        void Realloc(const u32 capacity) {
            T* old = m_Data;
            u32 oldCap = m_Capacity;
            Alloc(capacity);
            std::memcpy(m_Data, old, oldCap);
            delete[] old;
        }

        void ReallocIfNeeded(const u32 size) {
            if (size >= m_Capacity) {
                Realloc(m_Capacity * 2);
            }
        }

        const void VerifyInRange(const u32 index) const {
            BL_ASSERT(index < m_Size, "Vector index out of range!");
        }

        const void VerifyNotEmpty() const {
            BL_ASSERT(m_Size > 0, "Empty vector!");
        }

    private:
        u32 m_Size = 0;
        u32 m_Capacity = 0;
        T* m_Data = nullptr;
    };

} // namespace Blackberry