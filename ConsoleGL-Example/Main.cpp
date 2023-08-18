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

	Colour seedColours[ 16 ]
	{
		/*Black         */  { 0,   0,   0   },
		/*Dark_Blue     */  { 255, 0,   0   },
		/*Dark_Green    */  { 0,   255, 0   },
		/*Dark_Cyan     */  { 0,   0,   255 },
		/*Dark_Red      */  { 255, 255, 0   },
		/*Dark_Magenta  */  { 255, 0,   255 },
		/*Dark_Yellow   */  { 0,   255, 255 },
		/*Dark_White    */  { 255, 255, 255 },
		/*Bright_Black  */  { 85,  85,  85  },
		/*Bright_Blue   */  { 170, 85,  85  },
		/*Bright_Green  */  { 85,  170, 85  },
		/*Bright_Cyan   */  { 85,  85,  170 },
		/*Bright_Red    */  { 170, 170, 85  },
		/*Bright_Magenta*/  { 170, 85,  170 },
		/*Bright_Yellow */  { 85,  170, 170 },
		/*White         */  { 170, 170, 170 }
	};

	Window::SetColours( seedColours );

	auto w = window->GetWidth();
	auto h = window->GetHeight();

	//const char* colour_map = "colours.map";
	const char* colour_map = "pixels.map";

	std::ifstream f ( colour_map, std::ios::binary | std::ios::in );

	if ( !f.is_open() )
	{
		return false;
	}

	Pixel* pixels = new Pixel[ 16777216u ];

	f.read( (char*)pixels, 16777216u * sizeof( Pixel ) );
	f.close();

	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		auto buff = window->GetBuffer();

		for ( uint32_t y = 0u; y < window->GetHeight(); ++y )
		{
			for ( uint32_t x = 0u; x < window->GetWidth(); ++x )
			{
				uint32_t g = ( float )y / ( float )( window->GetHeight() - 1u ) * 255.0f;
				uint32_t r = ( float )x / ( float )( window->GetWidth() - 1u ) * 255.0f;

				Colour c{ r, g, b };

				//Pixel p( c );
				Pixel p = pixels[ 256u * 256u * b + 256u * g + r ];
				buff[ y * window->GetWidth() + x ] = p;
			}
		}

		Sleep( 100 );
		Window::SwapBuffer();
	}

	Window::Destroy( window );

	delete[] pixels;
	return 0;
}