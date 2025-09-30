local function SetupExample()
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "On"

    targetdir ( "../build/bin/" .. OutputDir .. "/%{prj.name}" )
    objdir ( "../build/obj/" .. OutputDir .. "/%{prj.name}" )

    files { "%{prj.name}/**.cpp", "%{prj.name}/**.hpp" }

    includedirs { "../Blackberry/src/" }

    links { BlackberryLinks }
end

project "10_creating_a_window"
    SetupExample()

project "10_events"
    SetupExample()

project "20_basic_rendering"
    SetupExample()

project "60_manual_rendering"
    SetupExample()
