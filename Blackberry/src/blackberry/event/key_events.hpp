#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/input/keycodes.hpp"

#include <sstream>

namespace Blackberry {

    class KeyEvent : public Event {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        virtual std::string ToString() const override {
            std::stringstream ss;
            ss << GetName();
            ss << ", Keycode: ";
            ss << (u32)m_KeyCode;

            return ss.str();
        }

        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Keyboard);

    protected:
        KeyEvent(const KeyCode keycode)
            : m_KeyCode(keycode) {}

    protected:
        KeyCode m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(const KeyCode keycode, const bool repeated)
            : KeyEvent(keycode), m_Repeated(repeated) {}

        bool IsRepeated() const { return m_Repeated; }

        EVENT_CLASS_TYPE(KeyPressed);

    private:
        bool m_Repeated;
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(const KeyCode keycode)
            : KeyEvent(keycode) {}

        EVENT_CLASS_TYPE(KeyReleased);
    };

    class KeyTypedEvent : public Event {
    public:
        KeyTypedEvent(const u32 key)
            : m_Key(key) {}

        u32 GetKey() const { return m_Key; }

        virtual std::string ToString() const override {
            std::stringstream ss;
            ss << GetName();
            ss << ", Key pressed: ";
            ss << m_Key;

            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyTyped);
        EVENT_CLASS_CATEGORY(EventCategory_Input | EventCategory_Keyboard);

    private:
        u32 m_Key;
    };

} // namespace Blackberry