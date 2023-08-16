#include <iostream>
#include <algorithm>

#include <ConsoleGL/Window.hpp>
#include <ConsoleGL/Colour.hpp>

using Window = ConsoleGL::Window;
using Pixel = ConsoleGL::Pixel;
using Colour = ConsoleGL::Colour;
using EConsoleColour = ConsoleGL::EConsoleColour;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	Window* window = Window::Create( "window", 128, 128, 4, 4 );
	Window::SetActive( window );

	Colour seedColours[ 16 ]
	{
		{ 0,   0,   0   }, //Black          
		{ 0,   0,   128 }, //Dark_Blue     
		{ 0,   128, 0   }, //Dark_Green  
		{ 0,   128, 128 }, //Dark_Cyan    
		{ 128, 0,   0   }, //Dark_Red          
		{ 128, 0,   128 }, //Dark_Magenta   
		{ 128, 128, 0   }, //Dark_Yellow      
		{ 192, 192, 192 }, //Bright_Grey
		{ 128, 128, 128 }, //Dark_Grey  
		{ 0,  0,  255 }, //Bright_Blue   
		{ 0,  255, 0  }, //Bright_Green    
		{ 0,  255, 255 }, //Bright_Cyan
		{ 255, 0,  0  }, //Bright_Red    
		{ 255, 0,  255 }, //Bright_Magenta
		{ 255, 255, 0  }, //Bright_Yellow 
		{ 255, 255, 255 }, //White
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

	for ( uint32_t r = 0u; r < 256u; ++r )
	{
		auto buff = window->GetBuffer();

		for ( uint32_t g = 0u; g < w; ++g )
		{
			for ( uint32_t b = 0u; b < h; ++b )
			{
				Colour c{ r, g * 2, b * 2 };
				Pixel p( c );

				buff[ w * g + b ] = p;
			}
		}

		Sleep( 100 );
		Window::SwapBuffer();
	}

	Window::Destroy( window );
	return 0;
}