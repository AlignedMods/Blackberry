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
                  "%{BlackberryIncludes.glfw}", 
                  "%{BlackberryIncludes.imgui}", 
                  "%{BlackberryIncludes.glad}", 
                  "%{BlackberryIncludes.glm}", 
                  "%{BlackberryIncludes.json}",
                  "%{BlackberryIncludes.entt}",
                  "%{BlackberryIncludes.spdlog}",
                  "vendor/stb/",
                  "vendor/msdf-atlas-gen/",
                  "vendor/msdf-atlas-gen/msdfgen/",
                  "vendor/msdf-atlas-gen/msdfgen/include/",
                  "vendor/lua/src/" }

    files { "src/platform/opengl/**.cpp", "src/platform/opengl/**.hpp" }

    filter "system:Windows"
        table.insert(BlackberryLinks, 1, "user32")
        table.insert(BlackberryLinks, 1, "gdi32")
        table.insert(BlackberryLinks, 1, "opengl32")

        files { "src/platform/glfw/**.cpp", "src/platform/glfw/**.hpp" }
        files { "src/platform/win32/win32_platform.cpp" }

        buildoptions { "/utf-8" }

    -- since this is a static library it doesn't need to link with anything, all linking is done with the "BlackberryLinks" variable