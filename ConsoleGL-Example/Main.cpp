#include <iostream>

#include <ConsoleGL/ConsoleWindow.hpp>
#include <ConsoleGL/ConsoleColour.hpp>

using Window = ConsoleGL::Window;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	auto window = Window::Create( "Window", 125, 125, 8, 8 );
	Window::SetActive( window );
	float i = 0;
	while ( true )
	{
		i += 0.001f;
		float c = cos( i );
		Window::SetBuffer( ConsoleGL::ConsoleColour::RED );
		Window::SetRect( 64 + c * 10, 64, 30, 30, ConsoleGL::ConsoleColour::GREEN );
		Window::SwapBuffer();
	}

	Window::Destroy();

	return 0;
}