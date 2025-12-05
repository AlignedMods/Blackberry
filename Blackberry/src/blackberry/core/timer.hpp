#pragma once

#include "blackberry/core/types.hpp"

#include <chrono>

namespace Blackberry {
    
    class Timer {
    public:
        void Start();
        void Reset();

        f32 ElapsedSeconds() const;
        f32 ElapsedMilliseconds() const;
        f32 ElapsedNanoseconds() const;

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
    };

    class ScopedTimer {
    public:
        ScopedTimer(const char* name);
        ~ScopedTimer();

    private:
        Timer m_Timer;
        const char* m_Name = nullptr;
    };

} // namespace Blackberry

#define BL_PROFILE_SCOPE(name) Blackberry::ScopedTimer(name)