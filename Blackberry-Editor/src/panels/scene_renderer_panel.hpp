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
        int m_CurrentBloomDownsampleStage = 1;
        int m_CurrentBloomUpsamlingStage = 1;
    };

} // namespace BlackberryEditor