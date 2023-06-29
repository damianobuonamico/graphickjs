project "optick"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
    "src/**.cpp",
    "src/**.h",
	}

  includedirs {
    "src"
  }
  
	filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    defines { "GK_USE_OPTICK" }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
    defines { "GK_USE_OPTICK" }

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
    symbols "off"
