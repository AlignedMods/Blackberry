project "Blackberry-Runtime"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "src/**.cpp", "src/**.hpp" }

    includedirs { "../Blackberry/src/",
                  "%{BlackberryIncludes.spdlog}",
                  "%{BlackberryIncludes.glm}",
                  "%{BlackberryIncludes.entt}",
                  "%{BlackberryIncludes.imguizmo}" }
    
    links { BlackberryLinks }

    filter "system:windows"
        buildoptions { "/utf-8" }