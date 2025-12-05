#pragma once

#include "blackberry.hpp"

namespace BlackberryEditor {

    class SceneRendererPanel {
    public:
        void OnUIRender(bool& open);

        void SetContext(Blackberry::Scene* scene);

    private:
        Blackberry::Scene* m_Context = nullptr;
        int m_CurrentDeferredImage = 0;
    };

} // namespace BlackberryEditor