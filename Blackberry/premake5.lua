include "external.lua"

BlackberryLinks = { "Blackberry", "glfw", "glad", "imgui", "freetype" }

project "Blackberry" -- The game engine
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "src/**.cpp", "src/**.hpp" }

    includedirs { "src/", "vendor/glfw/include/", "vendor/imgui/", "vendor/glad/include/", "vendor/glm/", "vendor/stb/", "vendor/freetype/include/" }

    filter "system:Windows"
        table.insert(BlackberryLinks, 1, "user32")
        table.insert(BlackberryLinks, 1, "gdi32")

    -- since this is a static library it doesn't need to link with anything, all linking is done with the "BlackberryLinks" variable