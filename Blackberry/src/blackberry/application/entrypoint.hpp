#pragma once

#include "application.hpp"

namespace Blackberry {

    extern Application* CreateApplication(u32 argc, char** argv);

} // namespace Blackberry

int main(int argc, char** argv) {
    Blackberry::Application *app = Blackberry::CreateApplication(static_cast<u32>(argc), argv);
    app->Run();
    delete app;
}