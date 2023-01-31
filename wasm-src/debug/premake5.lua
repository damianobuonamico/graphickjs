workspace "debug"
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
  
  include "graphick-debug/lib/glfw"
  include "graphick-debug/lib/glad"
  
  project "graphick-debug"
  kind "ConsoleApp"
  language "C++"
  location "graphick-debug"
  cppdialect "C++17"
  staticruntime "off"

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-int/" .. outputdir .. "/%{prj.name}")


  files {
    "%{prj.name}/src/**.h",
    "%{prj.name}/src/**.cpp",
  }

  includedirs {
    "%{prj.name}/src",
    "%{IncludeDir.glfw}",
    "%{IncludeDir.glad}",
    "../../editor",
    "../../history",
    "../../math",
    "../../renderer",
    "../../utils",
    "../../values",
  }

  links {
    "glfw",
    "glad",
    "opengl32.lib"
  }

  defines {
    "GLFW_INCLUDE_NONE"
  }

  filter "system:windows"
    systemversion "latest"

    defines {
      "GK_PLATFORM_WINDOWS"
    }

  filter "configurations:Debug"
    defines { "GK_CONF_DEBUG" }
    runtime "Debug"
    symbols "On"

  filter "configurations:Release"
    defines { "GK_CONF_RELEASE" }
    runtime "Release"
    optimize "On"
    symbols "On"

  filter "configurations:Dist"
    defines { "GK_CONF_DIST" }
    runtime "Release"
    optimize "On"
    symbols "Off"
