call GenerateProjectFiles.bat
set PLATFORM=Win64
set CONFIGURATION=Debug
devenv ConsoleGL.sln /build "%CONFIGURATION%|%PLATFORM%" /project ConsoleGL-PixelMapGenerator
cd Generated\Binaries\Debug\Win64\ConsoleGL-PixelMapGenerator
call ConsoleGL-PixelMapGenerator.exe
cd ..\..\..\..\..\
xcopy Generated\Binaries\Debug\Win64\ConsoleGL-PixelMapGenerator\PixelMap.inl Generated\Code\Debug\ConsoleGL-PixelMap\ /F /R /Y /I
devenv ConsoleGL.sln /build "%CONFIGURATION%|%PLATFORM%" /project ConsoleGL-PixelMap
devenv ConsoleGL.sln /build "%CONFIGURATION%|%PLATFORM%" /project ConsoleGL-Static
xcopy Generated\Binaries\Debug\Win64\ConsoleGL-Static\ConsoleGL-Static.lib Output\Debug\ConsoleGL-Static\ /F /R /Y /I