#include "layerstack.hpp"
#include "application.hpp"

namespace Blackberry {

    LayerStack::LayerStack() {}

    LayerStack::~LayerStack() {
        for (auto ptr : m_Layers) {
            ptr->OnDetach();
            delete ptr; // we must free up all the memory
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.push_back(layer);
        m_Layers.back()->OnAttach();
    }

    void LayerStack::PopLayer() {
        m_Layers.back()->OnDetach();
        delete m_Layers.back();
        m_Layers.pop_back();
    }

    void LayerStack::PopLayer(const std::string& name) {
        auto layer = GetLayer(name);

        if (*layer) {
            (*layer)->OnDetach();
            delete *layer;
            m_Layers.erase(layer);
        }
    }

    std::vector<Layer*>::iterator LayerStack::GetLayer(const std::string& name) {
        for (auto it = m_Layers.begin(); it < m_Layers.end(); it++) {
            Layer* layer = *it; // wow that's bullshit, storing pointers to pointers i suppose though

            if (layer->GetName() == name) {
                return it;
            }
        }

        return m_Layers.end();
    }

    std::vector<Layer*>& LayerStack::GetAllLayers() {
        return m_Layers;
    }

} // namespace Blackberry
