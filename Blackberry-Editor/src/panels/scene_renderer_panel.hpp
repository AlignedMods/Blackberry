#pragma once

#include "blackberry.hpp"

namespace BlackberryEditor {

    class SceneRendererPanel {
    public:
        void OnUIRender(bool& open);

        void SetContext(Blackberry::Ref<Blackberry::Scene> scene);

    private:
        Blackberry::Ref<Blackberry::Scene> m_Context;
        int m_CurrentDeferredImage = 0;
        int m_CurrentBloomDownsampleStage = 1;
        int m_CurrentBloomUpsamlingStage = 1;
    };

} // namespace BlackberryEditor