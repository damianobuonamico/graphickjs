project "harfbuzz"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
    "src/hb-aat-layout.cc",
    "src/hb-aat-map.cc",
    "src/hb-blob.cc",
    "src/hb-buffer-serialize.cc",
    "src/hb-buffer.cc",
    "src/hb-common.cc",
    "src/hb-draw.cc",
    "src/hb-face.cc",
    "src/hb-fallback-shape.cc",
    "src/hb-font.cc",
    "src/hb-map.cc",
    "src/hb-number.cc",
    "src/hb-ot-cff1-table.cc",
    "src/hb-ot-cff2-table.cc",
    "src/hb-ot-color.cc",
    "src/hb-ot-face.cc",
    "src/hb-ot-font.cc",
    "src/hb-ot-layout.cc",
    "src/hb-ot-map.cc",
    "src/hb-ot-math.cc",
    "src/hb-ot-meta.cc",
    "src/hb-ot-metrics.cc",
    "src/hb-ot-name.cc",
    "src/hb-ot-shape-complex-arabic.cc",
    "src/hb-ot-shape-complex-default.cc",
    "src/hb-ot-shape-complex-hangul.cc",
    "src/hb-ot-shape-complex-hebrew.cc",
    "src/hb-ot-shape-complex-indic-table.cc",
    "src/hb-ot-shape-complex-indic.cc",
    "src/hb-ot-shape-complex-khmer.cc",
    "src/hb-ot-shape-complex-myanmar.cc",
    "src/hb-ot-shape-complex-syllabic.cc",
    "src/hb-ot-shape-complex-thai.cc",
    "src/hb-ot-shape-complex-use.cc",
    "src/hb-ot-shape-complex-vowel-constraints.cc",
    "src/hb-ot-shape-fallback.cc",
    "src/hb-ot-shape-normalize.cc",
    "src/hb-ot-shape.cc",
    "src/hb-ot-tag.cc",
    "src/hb-ot-var.cc",
    "src/hb-set.cc",
    "src/hb-shape-plan.cc",
    "src/hb-shape.cc",
    "src/hb-shaper.cc",
    "src/hb-static.cc",
    "src/hb-style.cc",
    "src/hb-ucd.cc",
    "src/hb-unicode.cc",
    "src/hb-glib.cc",
    "src/hb-ft.cc",
    "src/hb-graphite2.cc",
    "src/hb-uniscribe.cc",
    "src/hb-gdi.cc",
    "src/hb-directwrite.cc",
    "src/hb-coretext.cc",
    "src/**.h",
    "src/**.hh"
	}

  includedirs {
    "src"
  }

  defines {
    "HB_NO_MT"
  }
  
	filter "configurations:Debug"
    runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
    symbols "off"
