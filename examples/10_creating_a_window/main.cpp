#define BL_ENTRYPOINT
#include "blackberry.hpp"

Blackberry::Application* Blackberry::Application::CreateApplication(const CommandLineArgs& args) {
    ApplicationSpecification spec;
    spec.Name = "Blackberry basic window!";
    spec.Width = 1280;
    spec.Height = 720;

    return new Blackberry::Application(spec);
}