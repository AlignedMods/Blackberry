#include "layerstack.hpp"
#include "application.hpp"

namespace Blackberry {

    LayerStack::LayerStack() {}

    LayerStack::~LayerStack() {
        for (auto ptr : m_Layers) {
            delete ptr; // we must free up all the memory
        }
    }

    void LayerStack::PopLayer() {
        m_Layers.pop_back();
    }

    Layer* LayerStack::GetLayer(const std::string& name) {
        for (auto layer : m_Layers) {
            if (layer->GetName() == name) { // very advanced searching algorithm
                return layer;
            }
        }

        return nullptr;
    }

    std::vector<Layer*>& LayerStack::GetAllLayers() {
        return m_Layers;
    }

} // namespace Blackberry