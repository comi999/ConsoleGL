require ("utils")

consolegl_source_folder = "../ConsoleGL/Source"
consolegl_dependencies_folder = "../ConsoleGL/Dependencies"

generated_folder = "../Generated"
code_folder = "../Generated/Code"
binaries_folder = "../Generated/Binaries"
intermediate_folder = "../Generated/Intermediate"
project_folder = "../Generated/Project"

consolegl_dependencies = {
	"glm"
}

project "ConsoleGL-PixelMap"
	location "../Generated/Project"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++20"
	
	links { "ConsoleGL" }
	dependson { "ConsoleGL" }
	includedirs { consolegl_source_folder, "." }
	forceincludes { path.join(consolegl_source_folder, "Common.hpp") }
	
    postbuildcommands {
		"cd \"$(TargetDir)\"",
		"ConsoleGL-PixelMap.exe",
		"copy /Y \"$(SolutionDir)ThirdParty\\File2Cpp\\File2Cpp.exe\" \"$(TargetDir)\"",
		"copy /Y \"$(SolutionDir)ConsoleGL\\Templates\\PixelMap.f2c\" \"$(TargetDir)\"",
		"mkdir \"$(SolutionDir)Generated\\Code\\%{cfg.buildcfg}\\Win64\\ConsoleGL-PixelMap\"",
		"\"File2Cpp.exe\" \"PixelMap.f2c\" \"--output\" \"$(SolutionDir)Generated\\Code\\%{cfg.buildcfg}\\Win64\\ConsoleGL-PixelMap\\PixelMap.inl\"",
		"del /Q File2Cpp.exe",
		"del /Q PixelMap.f2c",
		"del /Q PixelMap"
    }
	
    files {
		path.join("**.c"),
		path.join("**.cpp"),
		path.join("**.h"),
		path.join("**.hpp"),
		path.join("**.inl"),
		path.join("**.natvis"),
	}
	
	add_dependencies(consolegl_dependencies_folder, consolegl_dependencies, true)
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"UNICODE",
		"IS_EXE",
		"IS_PIXEL_MAP_GENERATOR",
	}
	
	filter "configurations:Debug"
		symbols "On"
		defines {
			"CONFIG_DEBUG",
		}
	
	filter "configurations:Release"
		optimize "On"
		defines {
			"CONFIG_RELEASE",
		}
	
	filter "platforms:Win64"
		architecture "x86_64"