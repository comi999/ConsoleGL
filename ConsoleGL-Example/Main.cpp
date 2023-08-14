#include <iostream>

#include <ConsoleGL/Window.hpp>
#include <ConsoleGL/Colour.hpp>

using Window = ConsoleGL::Window;
using Pixel = ConsoleGL::Pixel;
using Colour = ConsoleGL::Colour;
using EConsoleColour = ConsoleGL::EConsoleColour;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	Window* window = Window::Create( "window", 100, 100, 8, 8 );
	Window::SetActive( window );

	auto buff = window->GetBuffer();
	
	for ( int i = 0; i < window->GetArea(); ++i )
	{
		if ( i % 2 == 0 )
		{
			buff[ i ] = EConsoleColour::MAGENTA;
		}
		else
		{
			buff[ i ] = EConsoleColour::YELLOW;
		}
	}

	Colour c0 = { 1, 2, 3, 4 };
	c0 += { 2, 3, 4, 5 };

	auto r = 10 * c0;

	Window::SwapBuffer();
	Window::Destroy( window );
	return 0;
}