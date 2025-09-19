local function SetupExample()
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "%{prj.name}/**.cpp", "%{prj.name}/**.hpp" }

    includedirs { "../Blackberry/src/" }

    links { "Blackberry" }
end

project "10_creating_a_window"
    SetupExample()

project "20_basic_rendering"
    SetupExample()