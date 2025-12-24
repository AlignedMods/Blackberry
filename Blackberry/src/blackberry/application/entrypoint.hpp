#pragma once

#include "application.hpp"
#include "blackberry/lua/lua.hpp"
#include "blackberry/physics/physics_engine.hpp"

namespace Blackberry {

    inline void InitializeCore() {
        Lua::Initialize();
        PhysicsEngine::Initialize();
    }

    inline void ShutdownCore() {
        Lua::Shutdown();
        PhysicsEngine::Shutdown();
    }

    extern Application* CreateApplication(u32 argc, char** argv);

} // namespace Blackberry

int main(int argc, char** argv) {
    Blackberry::InitializeCore();

    Blackberry::Application *app = Blackberry::CreateApplication(static_cast<u32>(argc), argv);
    app->Run();
    delete app;

    Blackberry::ShutdownCore();
}