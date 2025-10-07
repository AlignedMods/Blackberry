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

    filter "system:windows"
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

    filter "system:linux"
        defines { "_GLFW_X11" }

project "glad"
    language "C"
    cdialect "C99"
    kind "StaticLib"
    staticruntime "On"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/glad/src/glad.c" }

    includedirs { "vendor/glad/include/" }

project "imgui"
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"
    staticruntime "On"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/imgui/*.cpp",
            "vendor/imgui/*.h",
            "vendor/imgui/backends/imgui_impl_glfw.h",
            "vendor/imgui/backends/imgui_impl_glfw.cpp",
            "vendor/imgui/backends/imgui_impl_opengl3.h",
            "vendor/imgui/backends/imgui_impl_opengl3.cpp",
            "vendor/imgui/misc/cpp/**.cpp",
            "vendor/imgui/misc/cpp/**.h" }

    includedirs { "vendor/imgui/", "vendor/glfw/include/" }

project "freetype"
    language "C++"
    cppdialect "C++20"
    kind "StaticLib"

    targetdir("../build/bin/" .. OutputDir .. "/%{prj.name}")
    objdir("../build/obj/" .. OutputDir .. "/%{prj.name}")

    files { "vendor/freetype/include/ft2build.h",
		    "vendor/freetype/include/freetype/*.h",
		    "vendor/freetype/include/freetype/config/*.h",
		    "vendor/freetype/include/freetype/internal/*.h",
            
		    "vendor/freetype/src/autofit/autofit.c",
		    "vendor/freetype/src/base/ftbase.c",
		    "vendor/freetype/src/base/ftbbox.c",
		    "vendor/freetype/src/base/ftbdf.c",
		    "vendor/freetype/src/base/ftbitmap.c",
		    "vendor/freetype/src/base/ftcid.c",
		    "vendor/freetype/src/base/ftdebug.c",
		    "vendor/freetype/src/base/ftfstype.c",
		    "vendor/freetype/src/base/ftgasp.c",
		    "vendor/freetype/src/base/ftglyph.c",
		    "vendor/freetype/src/base/ftgxval.c",
		    "vendor/freetype/src/base/ftinit.c",
		    "vendor/freetype/src/base/ftmm.c",
		    "vendor/freetype/src/base/ftotval.c",
		    "vendor/freetype/src/base/ftpatent.c",
		    "vendor/freetype/src/base/ftpfr.c",
		    "vendor/freetype/src/base/ftstroke.c",
		    "vendor/freetype/src/base/ftsynth.c",
		    "vendor/freetype/src/base/ftsystem.c",
		    "vendor/freetype/src/base/fttype1.c",
		    "vendor/freetype/src/base/ftwinfnt.c",
		    "vendor/freetype/src/bdf/bdf.c",
		    "vendor/freetype/src/bzip2/ftbzip2.c",
		    "vendor/freetype/src/cache/ftcache.c",
		    "vendor/freetype/src/cff/cff.c",
		    "vendor/freetype/src/cid/type1cid.c",
		    "vendor/freetype/src/gzip/ftgzip.c",
		    "vendor/freetype/src/lzw/ftlzw.c",
		    "vendor/freetype/src/pcf/pcf.c",
		    "vendor/freetype/src/pfr/pfr.c",
		    "vendor/freetype/src/psaux/psaux.c",
		    "vendor/freetype/src/pshinter/pshinter.c",
		    "vendor/freetype/src/psnames/psnames.c",
		    "vendor/freetype/src/raster/raster.c",
		    "vendor/freetype/src/sfnt/sfnt.c",
		    "vendor/freetype/src/smooth/smooth.c",
		    "vendor/freetype/src/truetype/truetype.c",
		    "vendor/freetype/src/type1/type1.c",
		    "vendor/freetype/src/type42/type42.c",
		    "vendor/freetype/src/winfonts/winfnt.c" }

    includedirs { "vendor/freetype/include/" }

    defines { "FT2_BUILD_LIBRARY", "_CRT_NONSTDC_NO_WARNINGS" }
