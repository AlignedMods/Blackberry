#include "event.hpp"
#include "blackberry/application/application.hpp"

namespace Blackberry {

    void Dispatcher::Subscribe(const SlotType& slot) {
        m_Observers.push_back(slot);
    }

    void Dispatcher::Post(const Event& event) {
        Application::Get().OnEvent(event);

        for (auto&& obeserver : m_Observers) {
            obeserver(event);
        }
    }

} // namespace Blackberry