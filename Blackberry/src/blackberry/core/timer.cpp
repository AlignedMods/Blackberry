#include "blackberry/core/timer.hpp"

#include <chrono>

namespace Blackberry {

    void Timer::Start() {
        Reset();
    }

    void Timer::Reset() {
        m_StartTime = std::chrono::steady_clock::now();
    }

    f32 Timer::ElapsedSeconds() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_StartTime).count() * 0.001f * 0.001f * 0.001f;
    }

    f32 Timer::ElapsedMilliseconds() const {
        return ElapsedSeconds() * 1000.0f;
    }

} // namespace Blackberry