#include <iostream>
#include <algorithm>
#include <fstream>

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
	Window::SetColours( ConsoleGL::EColourSet::GREYSCALE );

	auto w = window->GetWidth();
	auto h = window->GetHeight();

	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		auto buff = window->GetBuffer();

		for ( uint32_t y = 0u; y < window->GetHeight(); ++y )
		{
			for ( uint32_t x = 0u; x < window->GetWidth(); ++x )
			{
				uint32_t g = ( float )y / ( float )( window->GetHeight() - 1u ) * 255.0f;
				uint32_t r = ( float )x / ( float )( window->GetWidth() - 1u ) * 255.0f;
				buff[ y * window->GetWidth() + x ] = Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b };
			}
		}

		Sleep( 100 );
		Window::SwapBuffer();
	}

	Window::Destroy( window );
	return 0;
}