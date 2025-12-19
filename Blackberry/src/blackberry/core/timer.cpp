#include "blackberry/core/timer.hpp"
#include "blackberry/core/log.hpp"

#include <chrono>

namespace Blackberry {

    void Timer::Start() {
        Reset();
    }

    void Timer::Reset() {
        m_StartTime = std::chrono::steady_clock::now();
    }

    f32 Timer::ElapsedSeconds() const {
        return ElapsedMilliseconds() * 0.001f;
    }

    f32 Timer::ElapsedMilliseconds() const {
        return ElapsedNanoseconds() * 0.001f * 0.001f;
    }

    f32 Timer::ElapsedNanoseconds() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_StartTime).count();
    }

    ScopedTimer::ScopedTimer(const char* name) {
        m_Name = name;
        m_Timer.Start();
    }

    ScopedTimer::~ScopedTimer() {
        // BL_CORE_INFO("Timer {} took {}ms", m_Name, m_Timer.ElapsedMilliseconds());
    }

} // namespace Blackberry