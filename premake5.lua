require ("utils")

local generated_folder = "Generated"
local binaries_folder = "Generated/Binaries"
local intermediate_folder = "Generated/Intermediate"
local project_folder = "Generated/Project"

local apps_folder = "Applications"
local consolegl_folder = "ConsoleGL"

workspace "ConsoleGL"
	location "."
    configurations { "Debug", "Release" }
    platforms { "Win64" }
	startproject "Example"
	debugdir("$(OutDir)")
	targetdir("$(SolutionDir)Generated/Binaries/%{cfg.buildcfg}/%{cfg.platform}/%{prj.name}")
	objdir("$(SolutionDir)Generated/Intermediate/%{cfg.buildcfg}/%{cfg.platform}/%{prj.name}")
	defaultplatform "Win64"
	
	os.mkdir(binaries_folder)
	os.mkdir(intermediate_folder)
	os.mkdir(project_folder)
	
include "ConsoleGL"
include "ConsoleGL-ConsoleDock"
include "ConsoleGL-PixelMap"
include "ConsoleGL-Static"
include "ConsoleGL-Shared"

for _, app_folder in ipairs(os.matchdirs(path.join(apps_folder, "*"))) do
	local app_name = string.sub(app_folder, string.len(apps_folder) + 2, -1)
	print(string.format("Found App: %s", app_name))
	create_app(app_name, apps_folder, consolegl_folder)
end