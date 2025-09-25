include "external.lua"

project "Blackberry" -- The game engine
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "src/**.cpp", "src/**.hpp" }

    includedirs { "src/", "vendor/glfw/include/", "vendor/imgui/", "vendor/glad/include/", "vendor/glm/", "vendor/stb/", "vendor/freetype/include/" }

    links { "glfw", "glad", "imgui", "freetype" }
