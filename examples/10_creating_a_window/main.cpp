#include "blackberry/application/entrypoint.hpp"

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.width = 1280;
    spec.height = 720;
    spec.name = "Blackberry basic window!";

    return new Blackberry::Application(spec);
}