#pragma once

#include "layer.hpp"

#include <vector>

namespace Blackberry {

    class LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        template <typename T>
        void PushLayer() {
            static_assert(std::is_base_of<Layer, T>(), "Pushed layer is NOT a subclass of Layer!");
            m_Layers.push_back(new T);
            m_Layers.back()->OnInit();
        }
        void PopLayer();

        Layer* GetLayer(const std::string& name);

        std::vector<Layer*>& GetAllLayers();

    private:
        std::vector<Layer*> m_Layers;
    };

} // namespace Blackberry