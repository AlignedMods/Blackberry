#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/input/mousecodes.hpp"

#include <sstream>

namespace Blackberry {

    class MouseButtonPressedEvent : public Event {
    public:
        MouseButtonPressedEvent(const MouseCode button)
            : m_Button(button) {}

        MouseCode GetButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse);
        EVENT_CLASS_TYPE(MouseButtonPressed);

    private:
        MouseCode m_Button;
    };

    class MouseButtonReleasedEvent : public Event {
    public:
        MouseButtonReleasedEvent(const MouseCode button)
            : m_Button(button) {}

        MouseCode GetButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse);
        EVENT_CLASS_TYPE(MouseButtonReleased);

    private:
        MouseCode m_Button;
    };

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(const u32 x, const u32 y)
            : m_X(x), m_Y(y) {}

        u32 GetX() const { return m_X; }
        u32 GetY() const { return m_Y; }

        virtual std::string ToString() const override {
            std::stringstream ss;
            ss << GetName() << ": " << m_X << ", " << m_Y;

            return ss.str();
        }

        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse);
        EVENT_CLASS_TYPE(MouseMoved);

    private:
        u32 m_X;
        u32 m_Y;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(const f32 scroll)
            : m_Scroll(scroll) {}

        f32 GetScroll() const { return m_Scroll; }

        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Mouse);
        EVENT_CLASS_TYPE(MouseScrolled);

    private:
        f32 m_Scroll;
    };

} // namespace Blackberry