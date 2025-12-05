#pragma once

#include "blackberry.hpp"

namespace BlackberryEditor {

    class MaterialEditorPanel {
    public:
        void OnUIRender(bool& open);

        void SetContext(u64 materialHandle);

    private:
        u64 m_Context = 0;
    };

} // namespace BlackberryEditor