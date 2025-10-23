#pragma once

#include "application.hpp"
#include "blackberry/lua/lua.hpp"

int main(int argc, char **argv) {
    Blackberry::Lua::Initialize();
    Blackberry::Application *app = Blackberry::Application::CreateApplication(Blackberry::CommandLineArgs(argc, argv));
    app->Run();
    Blackberry::Lua::Shutdown();
    delete app;
}