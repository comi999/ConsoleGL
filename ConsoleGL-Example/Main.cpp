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
	//{ 0,   0,   0   }, // BLACK
	//{ 0,   0,   255 }, // BLUE
	//{ 0,   255, 0   }, // GREEN
	//{ 0,   128, 128 }, // CYAN
	//{ 255, 0,   0   }, // RED
	//{ 128, 0,   128 }, // MAGENTA
	//{ 128, 128, 0   }, // YELLOW
	//{ 170, 170, 170 }, // BRIGHT_GREY
	//{ 85,  85,  85  }, // GREY
	//{ 128, 128, 255 }, // BRIGHT_BLUE
	//{ 128, 255, 128 }, // BRIGHT_GREEN
	//{ 128, 192, 192 }, // BRIGHT_CYAN
	//{ 255, 128, 128 }, // BRIGHT_RED
	//{ 192, 128, 192 }, // BRIGHT_MAGENTA
	//{ 192, 192, 128 }, // BRIGHT_YELLOW
	//{ 255, 255, 255 }, // WHITE

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
	//AllocConsole();
	//freopen_s( ( FILE** )stdout, "CONOUT$", "w", stdout );

	// sepia
	//tr = 0.393R + 0.769G + 0.189B
	//tg = 0.349R + 0.686G + 0.168B
	//tb = 0.272R + 0.534G + 0.131B
	//for ( int i = 0; i < 16; ++i )
	//{
	//	Colour& c = seedColours[ i ];
	//	float r = c.r / 255.0f;
	//	float g = c.g / 255.0f;
	//	float b = c.b / 255.0f;
	//
	//	float x = 0.393 * r + 0.769 * g + 0.189 * b;
	//	float y = 0.349 * r + 0.686 * g + 0.168 * b;
	//	float z = 0.272 * r + 0.534 * g + 0.131 * b;

	//	x = std::clamp( x, 0.0f, 1.0f );
	//	y = std::clamp( y, 0.0f, 1.0f );
	//	z = std::clamp( z, 0.0f, 1.0f );
	//
	//	c.r = 255 * x;
	//	c.g = 255 * y;
	//	c.b = 255 * z;

	//	std::cout << ( int )c.r << ", " << ( int )c.g << ", " << ( int )c.b << std::endl;
	//}

	//for ( int i = 0; i < 16; ++i )
	//{
	//	auto val = 255u * i / 15.0f;
	//	seedColours[ i ] = { ( uint8_t )val, ( uint8_t )val, ( uint8_t )val, ( uint8_t )255u };
	//}

	/**/

	Window::SetColours( seedColours );

	//
	//
	//for ( int i = 0; i < window->GetArea(); ++i )
	//{
	//	buff[ i ] = ConsoleGL::ConsoleColourTable[ i % 16 ];
	//}

	auto w = window->GetWidth();
	auto h = window->GetHeight();

	const char* colour_map = "colours.map";

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
				uint32_t g = ( float )y / ( float )( window->GetHeight() - 1u ) * 255u;
				uint32_t r = ( float )x / ( float )( window->GetWidth() - 1 ) * 255u;

				Colour c{ r, g, b };

				Pixel p( c );

				//Pixel p = pixels[ 256u * 256u * b + 256u * g + r ];
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