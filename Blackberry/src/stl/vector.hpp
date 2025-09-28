#pragma once

#include "blackberry/types.hpp"

#include "stdlib.h"

template<typename T>
class BlVector {
public:
    using SizeType = u32;
    using Iterator = T*;

    BlVector() 
        : m_Data(new T[1]), m_Size(0), m_Capacity(1) {}

    explicit BlVector(SizeType size)
        : m_Data(new T[size]), m_Size(0), m_Capacity(size) {}

    BlVector(const BlVector& other) {
        m_Size = other.m_Size;
        m_Capacity = other.m_Capacity;
        m_Data = new T[m_Capacity];
        memcpy(m_Data, other.m_Data, m_Size * sizeof(T));
    }

    BlVector(BlVector&& other) {
        m_Capacity = other.m_Capacity;
        m_Size = other.m_Size;
        m_Data = other.m_Data;

        delete[] other.m_Data;
        other.m_Size = 0;
        other.m_Capacity = 0;
        other.m_Data = nullptr;
    }

    ~BlVector() {
        delete[] m_Data;
        m_Size = 0;
        m_Capacity = 0;
    }

    void PushBack(const T& f) {
        if (IsResizeNeeded()) {
            ChangeCapacity(m_Capacity * 2);
        }

        m_Data[m_Size++] = f;
    }

    // resize vector and fill up to capacity
    void Resize(SizeType size) {
        ChangeCapacity(size);

        for (SizeType i = m_Size; i < m_Capacity; i++) {
            m_Data[i] = T{};
        }
    }

    // resize vector without filling up
    void Reserve(SizeType size) {
        ChangeCapacity(size);
    }

    const T& At(SizeType index)  const {
        if (index >= m_Size) {
            Log(Log_Critical, "BlVector: Trying to access an index past bounds %lu when size is %lu!", index, m_Size);
            exit(1);
        }

        return m_Data[index];
    }

    T* GetData() const {
        return m_Data;
    }

    SizeType Size() const {
        return m_Size;
    }

    Iterator begin() {
        return m_Data;
    }

    Iterator end() {
        return m_Data + m_Size;
    }

private:
    void ChangeCapacity(SizeType size) {
        T* temp = new T[size];
        memcpy(temp, m_Data, m_Size * sizeof(T));
        delete[] m_Data;
        m_Data = temp;
        m_Capacity = size;
    }

    bool IsResizeNeeded() {
        return (m_Size >= m_Capacity - 1) ? true : false;
    }

private:
    SizeType m_Size;
    SizeType m_Capacity;
    T* m_Data;
};