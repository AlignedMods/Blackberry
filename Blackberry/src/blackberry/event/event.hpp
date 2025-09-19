#pragma once

#include "types.hpp"

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

// event category is NOT allowed to be an enum class because we need to be able to OR/AND them together
enum EventCategory : u32 {
    EventCategory_None     = 0,
    EventCategory_Window   = 1 << 0,
    EventCategory_Input    = 1 << 1,
    EventCategory_Keyboard = 1 << 2,
    EventCategory_Mouse    = 1 << 3,
    EventCategory_Gamepad  = 1 << 4
};

enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

#define EVENT_CAST(type) static_cast<const Blackberry::type&>(event)

#define EVENT_CLASS_TYPE(type) virtual EventType GetEventType() const override { return EventType::type; } \
                               virtual DiscriptorType GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual u32 GetCategoryFlags() const override { return category; }

namespace Blackberry {

    class Event {
    public:
        virtual ~Event() = default;
        using DiscriptorType = const char*;

        virtual EventType GetEventType() const = 0;
        virtual DiscriptorType GetName() const = 0;

        virtual std::string ToString() const { return GetName(); };

        virtual u32 GetCategoryFlags() const = 0;

        bool IsInCategory(EventCategory category) {
            return GetCategoryFlags() & category;
        }
    };

    class Dispatcher {
    public:
        using SlotType = std::function<void(const Event&)>;

        void Subscribe(const SlotType& slot);

        void Post(const Event& event);

    private:
        std::vector<SlotType> m_Observers;
    };

} // namespace Blackberry