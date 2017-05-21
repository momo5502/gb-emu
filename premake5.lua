workspace "gb-emu"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	configurations { "Debug", "Release" }
	platforms { "x32", "x64" }

	-- VS 2015 toolset only
	toolset "msc-140"

	configuration "windows"
		defines { "_WINDOWS", "WIN32" }
		flags { "StaticRuntime" }

		if symbols ~= nil then
			symbols "On"
		else
			flags { "Symbols" }
		end

	configuration "Release"
		defines { "NDEBUG" }
		flags { "MultiProcessorCompile", "LinkTimeOptimization" }
		optimize "Full"

	configuration "Debug"
		defines { "DEBUG", "_DEBUG" }
		flags { "MultiProcessorCompile" }
		optimize "Debug"

	project "gb-emu"
		kind "ConsoleApp"
		language "C++"
		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}
		includedirs {
			"./src"
		}

		-- Pre-compiled header
		pchheader "STDInclude.hpp" -- must be exactly same as used in #include directives
		pchsource "src/STDInclude.cpp" -- real path
		buildoptions { "/Zm100 -Zm100" }

		vpaths {
			["Docs/*"] = { "**.txt","**.md" },
		}

		-- Specific configurations
		flags { "UndefinedIdentifiers", "ExtraWarnings" }

		configuration "Release*"
			flags { "FatalCompileWarnings" }
		configuration {}
