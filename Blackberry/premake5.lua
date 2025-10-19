group "dependencies"
    include "external.lua"
group ""

BlackberryLinks = { "Blackberry", "glfw", "glad", "imgui", "freetype", "msdfgen", "msdf-atlas-gen", "lua" }

project "Blackberry" -- The game engine
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "src/blackberry/**.cpp", "src/blackberry/**.hpp", "src/blackberry.hpp" }

    includedirs { "src/", 
                  "vendor/glfw/include/", 
                  "vendor/imgui/", 
                  "vendor/glad/include/", 
                  "vendor/glm/", 
                  "vendor/stb/",
                  "vendor/json/",
                  "vendor/msdf-atlas-gen/",
                  "vendor/msdf-atlas-gen/msdfgen/",
                  "vendor/msdf-atlas-gen/msdfgen/include/" }

    files { "src/platform/opengl/**.cpp", "src/platform/opengl/**.hpp" }

    filter "system:Windows"
        table.insert(BlackberryLinks, 1, "user32")
        table.insert(BlackberryLinks, 1, "gdi32")
        table.insert(BlackberryLinks, 1, "opengl32")

        files { "src/platform/glfw/**.cpp", "src/platform/glfw/**.hpp" }

    -- since this is a static library it doesn't need to link with anything, all linking is done with the "BlackberryLinks" variable