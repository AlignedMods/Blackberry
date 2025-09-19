project "glfw"
    language "C"
    cdialect "C99"
    kind "StaticLib"
    staticruntime "On"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/glfw/include/GLFW/*.h", 
            "vendor/glfw/src/glfw_config.h", 
            "vendor/glfw/src/context.c", 
            "vendor/glfw/src/init.c", 
            "vendor/glfw/src/input.c", 
            "vendor/glfw/src/monitor.c", 
            "vendor/glfw/src/vulkan.c", 
            "vendor/glfw/src/window.c", 
            "vendor/glfw/src/platform.c",
            "vendor/glfw/src/platform.c",     
            "vendor/glfw/src/null_init.c",    
            "vendor/glfw/src/null_joystick.c",
            "vendor/glfw/src/null_monitor.c", 
            "vendor/glfw/src/null_window.c" }

    filter "system:Windows"
        files { "vendor/glfw/src/win32_init.c", 
                "vendor/glfw/src/win32_joystick.c",
                "vendor/glfw/src/win32_monitor.c", 
                "vendor/glfw/src/win32_time.c", 
                "vendor/glfw/src/win32_thread.c", 
                "vendor/glfw/src/win32_window.c",
                "vendor/glfw/src/win32_module.c",
                "vendor/glfw/src/wgl_context.c", 
                "vendor/glfw/src/egl_context.c", 
                "vendor/glfw/src/osmesa_context.c" }

        defines { "_GLFW_WIN32" }

project "glad"
    language "C"
    cdialect "C99"
    kind "StaticLib"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/glad/src/glad.c" }

    includedirs { "vendor/glad/include" }

project "imgui"
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/imgui/*.cpp",
            "vendor/imgui/*.h",
            "vendor/imgui/backends/imgui_impl_glfw.h",
            "vendor/imgui/backends/imgui_impl_glfw.cpp",
            "vendor/imgui/backends/imgui_impl_opengl3.h",
            "vendor/imgui/backends/imgui_impl_opengl3.cpp"}

    includedirs { "vendor/imgui/", "vendor/glfw/include" }