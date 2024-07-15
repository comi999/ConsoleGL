#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#undef CreateWindow
#include <ConsoleGL.hpp>

//#include <stb_image/stb_image.h>


INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	ConsoleGL::RunTest();
}
