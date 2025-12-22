#include "blackberry/core/timer.hpp"
#include "blackberry/core/log.hpp"

#include <chrono>

namespace Blackberry {

    std::unordered_map<const char*, TimePoint> s_TimePoints;

#pragma region TimePoint

    f32 TimePoint::Seconds() const {
        return Milliseconds() * 0.001f;
    }

    f32 TimePoint::Milliseconds() const {
        return Nanoseconds() * 0.001f * 0.001f;
    }

    f32 TimePoint::Nanoseconds() const {
        return Time;
    }

#pragma endregion

#pragma region Instrumentor

    void Instrumentor::SetTimePoint(const char* name, TimePoint timePoint) {
        s_TimePoints[name] = timePoint;
    }

    TimePoint Instrumentor::GetTimePoint(const char* name) {
        if (!s_TimePoints.contains(name)) {
            BL_CORE_WARN("Trying to access non-existent TimePoint {}!", name);
            return {};
        }

        return s_TimePoints.at(name);
    }

#pragma endregion

#pragma region Timer

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

#pragma endregion

#pragma region ScopedTimer

    ScopedTimer::ScopedTimer(const char* name) {
        m_Name = name;
        m_Timer.Start();
    }

    ScopedTimer::~ScopedTimer() {
        Instrumentor::SetTimePoint(m_Name, { m_Timer.ElapsedNanoseconds() });
    }

#pragma endregion

} // namespace Blackberry