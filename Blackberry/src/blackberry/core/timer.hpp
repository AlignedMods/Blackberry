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

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
    };

} // namespace Blackberry