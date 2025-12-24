#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/util.hpp"

#include <cstring>

namespace Blackberry {

    // std::unique_ptr inspired class which allows for safe handling of unique (non shared) pointers
    template <typename T>
    class Scope {
    public:
        Scope()
            : m_Ptr(new T{}) {}

        ~Scope() {
            delete m_Ptr;
            m_Ptr = nullptr;
        }

        Scope(const Scope& other) = delete;

        T& operator*() {
            BL_ASSERT(m_Ptr, "Scope use after free!");
            return *m_Ptr;
        }

        T* operator->() {
            BL_ASSERT(m_Ptr, "Scope use after free!");
            return m_Ptr;
        }

        T* Data() {
            return m_Ptr;
        }

        void SetData(const T* data) {
            memcpy(m_Ptr, data, sizeof(T));
        }

    private:
        T* m_Ptr = nullptr;
    };

    // std::shared_ptr inspired class which allows for safe handling of shared pointers
    template <typename T>
    class Ref {
    public:
        Ref()
            : m_Ptr(nullptr), m_Counter(new u32{0}) {}

        Ref(T* ptr)
            : m_Ptr(ptr), m_Counter(new u32{1}) {}

        ~Ref() {
            Release();
        }

        Ref(const Ref& other) {
            m_Ptr = other.m_Ptr;
            m_Counter = other.m_Counter;

            IncrementCounter();
        }

        Ref(Ref&& other) noexcept {
            m_Ptr = other.m_Ptr;
            m_Counter = other.m_Counter;

            other.m_Ptr = nullptr;
            other.m_Counter = nullptr;
        }

        Ref& operator=(const Ref& other) {
            if (this != &other) {
                Release();
                m_Ptr = other.m_Ptr;
                m_Counter = other.m_Counter;
                
                IncrementCounter();
            }

            return *this;
        }

        Ref& operator=(Ref&& other) noexcept {
            if (this != &other) {
                Release();

                m_Ptr = other.m_Ptr;
                m_Counter = other.m_Counter;

                other.m_Ptr = nullptr;
                other.m_Counter = nullptr;
            }

            return *this;
        }

        T& operator*() {
            BL_ASSERT(m_Ptr, "Ref use after free!");
            return *m_Ptr;
        }

        const T& operator*() const {
            BL_ASSERT(m_Ptr, "Ref use after free!");
            return *m_Ptr;
        }

        T* operator->() {
            BL_ASSERT(m_Ptr, "Ref use after free!");
            return m_Ptr;
        }

        const T* operator->() const {
            BL_ASSERT(m_Ptr, "Ref use after free!");
            return m_Ptr;
        }

        operator bool() const { return m_Ptr != nullptr; }

        T* Data() {
            return m_Ptr;
        }

        bool Unique() const {
            return *m_Counter == 1;
        }

        void Release() {
            if (!m_Counter) return;

            if ((*m_Counter)-- == 0) {
                delete m_Ptr;
                delete m_Counter;

                m_Ptr = nullptr;
                m_Counter = nullptr;
            }   
        }

    private:
        void IncrementCounter() {
            if (m_Counter) (*m_Counter)++;
        }

    public:
        T* m_Ptr = nullptr;
        u32* m_Counter = nullptr;
    };

    template <typename T, typename... Args>
    Scope<T> CreateScope(const Args&&... args) {
        T* data = new T{args...};
        Scope<T> scope;
        scope.SetData(data);
        delete data;
        return scope;
    }

    template <typename T>
    Scope<T> CreateScope(const T& s) {
        return CreateScope<T>(T{s});
    }

    template <typename T>
    Scope<T> CreateScope() {
        return CreateScope<T>(T{});
    }

    template <std::_Not_builtin_array T, typename... Args>
    Ref<T> CreateRef(const Args&&... args) {
        T* data = new T{args...};
        Ref<T> ref(data);

        return ref;
    }

    template <typename T>
    Ref<T> CreateRef() {
        T* data = new T{};
        Ref<T> ref(data);

        return ref;
    }

} // namespace Blackberry