#pragma once

#include "application.hpp"

int main(int argc, char **argv) {
    Blackberry::Application *app = Blackberry::Application::CreateApplication(Blackberry::CommandLineArgs(argc, argv));
    app->Run();
    delete app;
}