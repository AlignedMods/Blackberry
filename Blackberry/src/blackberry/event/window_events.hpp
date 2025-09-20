#pragma once

#include "blackberry/event/event.hpp"

namespace Blackberry {

    // quite a simple one, no info to give
    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() {}

        virtual std::string ToString() const override {
            std::stringstream ss;

            ss << GetName() << ": " << "window leaving";

            return ss.str();
        }

        EVENT_CLASS_CATEGORY(EventCategory_Window);
        EVENT_CLASS_TYPE(WindowClose);
    };

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(const i32 width, const i32 height)
            : m_Width(width), m_Height(height) {}

        i32 GetWidth() const { return m_Width; }
        i32 GetHeight() const { return m_Height; }

        virtual std::string ToString() const override {
            std::stringstream ss;

            ss << GetName() << ": " << m_Width << ", " << m_Height;

            return ss.str();
        }

        EVENT_CLASS_CATEGORY(EventCategory_Window);
        EVENT_CLASS_TYPE(WindowResize);

    private:
        i32 m_Width;
        i32 m_Height;
    };

} // namespace Blackberry