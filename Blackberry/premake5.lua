group "dependencies"
    include "external.lua"
group ""

BlackberryLinks = { "Blackberry", "glfw", "glad", "imgui", "imguizmo", "freetype", "lua", "msdfgen", "msdf-atlas-gen", "yaml-cpp" }

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
                  "%{BlackberryIncludes.freetype}",
                  "%{BlackberryIncludes.msdfgen}",
                  "%{BlackberryIncludes.msdfgen}/static/",
                  "%{BlackberryIncludes.msdf_atlas_gen}",
                  "%{BlackberryIncludes.yaml}",
                  "vendor/stb/",
                  "vendor/cgltf/",
                  "vendor/lua/src/" }

    files { "src/platform/opengl/**.cpp", "src/platform/opengl/**.hpp" }

    defines { "YAML_CPP_STATIC_DEFINE"}

    filter "system:Windows"
        table.insert(BlackberryLinks, 1, "user32")
        table.insert(BlackberryLinks, 1, "gdi32")
        table.insert(BlackberryLinks, 1, "opengl32")
        table.insert(BlackberryLinks, 1, "shlwapi")

        files { "src/platform/glfw/**.cpp", "src/platform/glfw/**.hpp" }
        files { "src/platform/win32/win32_platform.cpp" }

        buildoptions { "/utf-8" }

    -- since this is a static library it doesn't need to link with anything, all linking is done with the "BlackberryLinks" variable