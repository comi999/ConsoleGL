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

project "ConsoleGL-Shared"
	location "../Generated/Project"
    kind "SharedLib"
    language "C++"
	cppdialect "C++20"
	
	links { "ConsoleGL" }
	dependson { "ConsoleGL", "ConsoleGL-PixelMapGenerator" }
	includedirs { consolegl_source_folder, "." }
	forceincludes { path.join(consolegl_source_folder, "Common.hpp") }
	add_dependencies(consolegl_dependencies_folder, consolegl_dependencies, true)
	includedirs { "%{code_folder}/%{cfg.buildcfg}/Win64/ConsoleGL-PixelMapGenerator" }
	
    files {
		path.join("**.c"),
		path.join("**.cpp"),
		path.join("**.h"),
		path.join("**.hpp"),
		path.join("**.inl"),
		path.join("**.natvis"),
	}
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"UNICODE",
		"IS_SHARED_LIB",
		"IS_CONSOLEGL",
	}
	
	filter "configurations:Debug"
		defines { "CONFIG_DEBUG" }
		symbols "On"
	
	filter "configurations:Release"
		defines { "CONFIG_RELEASE" }
		optimize "On"
	
	filter "platforms:Win64"
		architecture "x86_64"