#pragma once

namespace BlackberryEditor {

    class AssetManagerPanel {
    public:
        void OnUIRender(bool& open);

    private:
        bool m_AllowEditing = false;
    };

} // namespace BlackberryEditor