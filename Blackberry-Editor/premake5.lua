project "Blackberry-Editor"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "src/**.cpp", "src/**.hpp" }

    includedirs { "../Blackberry/src/",
                  "%{BlackberryIncludes.spdlog}",
                  "%{BlackberryIncludes.glm}" }
    
    links { BlackberryLinks }

    filter "system:windows"
        buildoptions { "/utf-8" }