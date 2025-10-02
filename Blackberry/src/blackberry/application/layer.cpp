#include "blackberry/application/layer.hpp"
#include "blackberry/ecs/ecs.hpp"

namespace Blackberry {

    Layer::Layer() : m_Name("Layer") {
        m_Coordinator = std::make_unique<Coordinator>();
    }

    std::string& Layer::GetName() {
        return m_Name;
    }

    void Layer::SetName(const std::string& name) {
        m_Name = name;
    }

} // namespace Blackberry