#pragma once

#include "layer.hpp"

#include <vector>

namespace Blackberry {

    class LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PopLayer();
        void PopLayer(const std::string& name);

        std::vector<Layer*>::iterator GetLayer(const std::string& name);

        std::vector<Layer*>& GetAllLayers();

    private:
        std::vector<Layer*> m_Layers;
    };

} // namespace Blackberry