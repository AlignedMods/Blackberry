project "stress-test"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "stress-test/**.cpp", "stress-test/**.hpp" }

    includedirs { "../Blackberry/src/",
                  "%{BlackberryIncludes.spdlog}",
                  "%{BlackberryIncludes.glm}" }
    
    links { BlackberryLinks }

    filter "system:windows"
        buildoptions { "/utf-8" }