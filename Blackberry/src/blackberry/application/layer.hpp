#pragma once

#include "blackberry/event/event.hpp"
#include "blackberry/core/types.hpp"
// #include "blackberry/ecs/ecs.hpp"

#include <string>
#include <vector>
#include <memory>

namespace Blackberry {

    class Coordinator;

    class Layer {
    public:
        Layer();
        virtual ~Layer() = default;

        std::string& GetName();
        void SetName(const std::string& name);

        // overridable functions
        virtual void OnAttach() {}                  // gets called when layer is first initialized (NOTE: you should prefer to use this instead of the constructor!)
        virtual void OnDetach() {}                  // gets called when layer is being removed

        virtual void OnRender() {}                  // gets called each time the frame is rendered
        virtual void OnUIRender() {}                // gets called when the ImGui UI is being rendered
        virtual void OnOverlayRender() {}           // gets called LAST in the pipeline (after normal render and ImGui)
        virtual void OnUpdate(f32 dt) {}            // gets called each time when the frame is being updated
        virtual void OnFixedUpdate() {}             // gets called exactly 60 times per second
        virtual void OnEvent(const Event& event) {} // gets called whenever an event occurs

    protected:
        std::string m_Name;

        friend class Application;
    };

} // namespace Blackberry