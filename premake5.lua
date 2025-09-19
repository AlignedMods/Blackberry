workspace "Blackberry"
    configurations { "Debug", "Release" }
    platforms { "x86", "x86_64" }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- annoying
    defines { "_CRT_SECURE_NO_WARNINGS" }

    include "Blackberry/"

    group "examples"
        include "examples/"
    group ""