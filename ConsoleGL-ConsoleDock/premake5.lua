require ("utils")

consolegl_source_folder = "../ConsoleGL/Source"
consolegl_dependencies_folder = "../ConsoleGL/Dependencies"

generated_folder = "../Generated"
binaries_folder = "../Generated/Binaries"
intermediate_folder = "../Generated/Intermediate"
project_folder = "../Generated/Project"

consolegl_dependencies = {
	"glm"
}

project "ConsoleGL-ConsoleDock"
	location "../Generated/Project"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++20"
	
	links { "ConsoleGL" }
	dependson { "ConsoleGL" }
	includedirs { consolegl_source_folder, "." }
	forceincludes { path.join(consolegl_source_folder, "Common.hpp") }
	
    files {
		path.join("**.c"),
		path.join("**.cpp"),
		path.join("**.h"),
		path.join("**.hpp"),
		path.join("**.inl"),
		path.join("**.natvis"),
	}
	
	add_dependencies(consolegl_dependencies_folder, consolegl_dependencies, true)
	
    postbuildcommands {
		"copy /Y \"$(SolutionDir)ThirdParty\\File2Cpp\\File2Cpp.exe\" \"$(TargetDir)\"",
		"copy /Y \"$(SolutionDir)ConsoleGL\\Templates\\ConsoleDock.f2c\" \"$(TargetDir)\"",
		"mkdir \"$(SolutionDir)Generated\\Code\\%{cfg.buildcfg}\\Win64\\ConsoleGL-ConsoleDock\"",
		"cd \"$(TargetDir)\"",
		"\"File2Cpp.exe\" \"ConsoleDock.f2c\" \"--output\" \"$(SolutionDir)Generated\\Code\\%{cfg.buildcfg}\\Win64\\ConsoleGL-ConsoleDock\\ConsoleDock.inl\"",
		"del /Q File2Cpp.exe",
		"del /Q ConsoleDock.f2c"
    }
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"UNICODE",
		"IS_EXE",
		"IS_CONSOLE_DOCK",
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