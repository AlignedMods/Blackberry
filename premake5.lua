workspace "Blackberry"
    configurations { "Debug", "Release", "Dist" }
    platforms { "x86", "x86_64" }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- annoying
    defines { "_CRT_SECURE_NO_WARNINGS" }

    filter "configurations:Debug"
        symbols "On"
        defines { "BL_DEBUG_BUILD" }

    filter "configurations:Release"
        optimize "On"
        defines { "BL_RELEASE_BUILD" }

    filter "configurations:Dist"
        optimize "Full"
        defines { "BL_DIST_BUILD" }

    filter ""

    filter "system:windows"
        defines { "BL_WINDOWS_BUILD" }

    filter "system:linux"
        defines { "BL_LINUX_BUILD" }

    filter ""

    include "Blackberry/"

    group "examples"
        include "examples/"
    group ""

    group "tools"
        include "Blackberry-Editor/"
    group ""
