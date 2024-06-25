require ("utils")

generated_folder = "../Generated"
binaries_folder = "../Generated/Binaries"
intermediate_folder = "../Generated/Intermediate"
logs_folder = "../Generated/Logs"
project_folder = "../Generated/Project"

consolegl_source_folder = "Source"
consolegl_dependencies_folder = "Dependencies"
consolegl_resources_folder = "Resources"

consolegl_dependencies = {
	"glm"
}

project "ConsoleGL"
	location "../Generated/Project"
    kind "SharedItems"
    language "C++"

    files {
		path.join(consolegl_source_folder, "**.c"),
		path.join(consolegl_source_folder, "**.cpp"),
		path.join(consolegl_source_folder, "**.h"),
		path.join(consolegl_source_folder, "**.hpp"),
		path.join(consolegl_source_folder, "**.inl"),
		path.join(consolegl_source_folder, "**.natvis"),
		path.join(consolegl_dependencies_folder, "**.c"),
		path.join(consolegl_dependencies_folder, "**.cpp"),
		path.join(consolegl_dependencies_folder, "**.h"),
		path.join(consolegl_dependencies_folder, "**.hpp"),
		path.join(consolegl_dependencies_folder, "**.inl"),
		path.join(consolegl_dependencies_folder, "**.natvis"),
	}
	
	add_dependencies(consolegl_dependencies_folder, consolegl_dependencies, false)