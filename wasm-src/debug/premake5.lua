workspace "graphick-debug"
  architecture "x64"
  startproject "graphick-debug"

  configurations {
    "Debug",
    "Release",
    "Dist"
  }

  outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
  
  IncludeDir = {}
  IncludeDir["glfw"] = "graphick-debug/lib/glfw/include"
  IncludeDir["glad"] = "graphick-debug/lib/glad/include"
  IncludeDir["optick"] = "graphick-debug/lib/optick/src"
  
  include "graphick-debug/lib/glfw"
  include "graphick-debug/lib/glad"
  include "graphick-debug/lib/optick"
  
  project "graphick-debug"
  kind "ConsoleApp"
  language "C++"
  location "graphick-debug"
  cppdialect "C++17"
  staticruntime "off"

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

  files {
    "../**.h",
    "../**.hpp",
    "../**.cpp",
    "%{prj.name}/src/**.h",
    "%{prj.name}/src/**.cpp"
  }

  removefiles {
    "../export.cpp",
    "%{prj.name}/lib/**.h",
    "%{prj.name}/lib/**.hpp",
    "%{prj.name}/lib/**.cpp",
  }

  includedirs {
    "%{prj.name}/src",
    "%{IncludeDir.glfw}",
    "%{IncludeDir.glad}",
    "%{IncludeDir.optick}",
    "../../"
  }

  links {
    "glfw",
    "glad",
    "optick",
    "opengl32.lib"
  }

  defines {
    "GLFW_INCLUDE_NONE"
  }

  linkoptions { 
    "/NODEFAULTLIB:\"LIBCMTD\"",
    "/NODEFAULTLIB:\"LIBCMT\""
  }

  flags {
    "MultiProcessorCompile"
  }

  filter "system:windows"
    systemversion "latest"

    defines {
      "GK_PLATFORM_WINDOWS"
    }

  filter "configurations:Debug"
    defines { "GK_CONF_DEBUG", "GK_USE_OPTICK" }
    runtime "Debug"
    symbols "On"

  filter "configurations:Release"
    defines { "GK_CONF_RELEASE", "GK_USE_OPTICK" }
    runtime "Release"
    optimize "On"
    symbols "On"
    floatingpoint "Fast"

  filter "configurations:Dist"
    defines { "GK_CONF_DIST" }
    runtime "Release"
    optimize "On"
    symbols "Off"
    floatingpoint "Fast"
