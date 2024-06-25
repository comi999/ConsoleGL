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