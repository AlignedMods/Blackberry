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
	language "C"
	kind "StaticLib"
    staticruntime "On"

	targetdir ("../build/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("../build/obj/" .. OutputDir .. "/%{prj.name}")

	files { "vendor/msdf-atlas-gen/msdfgen/freetype/include/ft2build.h",
			"vendor/msdf-atlas-gen/msdfgen/freetype/include/freetype/*.h",
			"vendor/msdf-atlas-gen/msdfgen/freetype/include/freetype/config/*.h",
			"vendor/msdf-atlas-gen/msdfgen/freetype/include/freetype/internal/*.h",

			"vendor/msdf-atlas-gen/msdfgen/freetype/src/autofit/autofit.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftbase.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftbbox.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftbdf.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftbitmap.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftcid.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftdebug.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftfstype.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftgasp.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftglyph.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftgxval.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftinit.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftmm.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftotval.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftpatent.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftpfr.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftstroke.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftsynth.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftsystem.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/fttype1.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/base/ftwinfnt.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/bdf/bdf.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/bzip2/ftbzip2.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/cache/ftcache.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/cff/cff.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/cid/type1cid.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/gzip/ftgzip.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/lzw/ftlzw.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/pcf/pcf.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/pfr/pfr.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/psaux/psaux.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/pshinter/pshinter.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/psnames/psnames.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/raster/raster.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/sdf/sdf.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/sfnt/sfnt.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/smooth/smooth.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/truetype/truetype.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/type1/type1.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/type42/type42.c",
			"vendor/msdf-atlas-gen/msdfgen/freetype/src/winfonts/winfnt.c"
	}

	includedirs {
		"vendor/msdf-atlas-gen/msdfgen/freetype/include/"
	}

	defines { "FT2_BUILD_LIBRARY", "_CRT_NONSTDC_NO_WARNINGS" }

project "msdfgen"
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"
    staticruntime "On"

	targetdir ("../build/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("..build/obj/" .. OutputDir .. "/%{prj.name}")

	files { "vendor/msdf-atlas-gen/msdfgen/core/**.h",
			"vendor/msdf-atlas-gen/msdfgen/core/**.hpp",
			"vendor/msdf-atlas-gen/msdfgen/core/**.cpp",
			"vendor/msdf-atlas-gen/msdfgen/ext/**.h",
			"vendor/msdf-atlas-gen/msdfgen/ext/**.hpp",
			"vendor/msdf-atlas-gen/msdfgen/ext/**.cpp",
			"vendor/msdf-atlas-gen/msdfgen/lib/**.cpp",
			"vendor/msdf-atlas-gen/msdfgen/include/**.h" }

	includedirs { "vendor/msdf-atlas-gen/msdfgen/include/",
				  "vendor/msdf-atlas-gen/msdfgen/freetype/include/" }

	defines { "MSDFGEN_USE_CPP11" }

project "msdf-atlas-gen"
	language "C++"
	cppdialect "C++20"
	kind "StaticLib"
    staticruntime "On"

	targetdir ("../build/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("../build/obj/" .. OutputDir .. "/%{prj.name}")

	files { "vendor/msdf-atlas-gen/msdf-atlas-gen/**.h",
    		"vendor/msdf-atlas-gen/msdf-atlas-gen/**.hpp",
    		"vendor/msdf-atlas-gen/msdf-atlas-gen/**.cpp" }

	includedirs {
		"vendor/msdf-atlas-gen/msdf-atlas-gen/",
		"vendor/msdf-atlas-gen/msdfgen/",
		"vendor/msdf-atlas-gen/msdfgen/include/" }