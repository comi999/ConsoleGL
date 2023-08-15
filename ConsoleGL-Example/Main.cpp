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

	Colour seedColours[ 16 ]{
			{ 0,   0,   0   }, //Black          
			{ 255, 0,   0   }, //Dark_Blue      
			{ 0,   255, 0   }, //Dark_Green     
			{ 0,   0,   255 }, //Dark_Cyan      
			{ 255, 255, 0   }, //Dark_Red       
			{ 255, 0,   255 }, //Dark_Magenta   
			{ 0,   255, 255 }, //Dark_Yellow    
			{ 255, 255, 255 }, //Dark_White     
			{ 85,  85,  85  }, //Bright_Black   
			{ 170, 85,  85  }, //Bright_Blue    
			{ 85,  170, 85  }, //Bright_Green   
			{ 85,  85,  170 }, //Bright_Cyan    
			{ 170, 170, 85  }, //Bright_Red     
			{ 170, 85,  170 }, //Bright_Magenta 
			{ 85,  170, 170 }, //Bright_Yellow  
			{ 170, 170, 170 }  //White
	};

	for ( int i = 0; i < 16; ++i )
	{
		auto val = 255u * i / 15.0f;
		seedColours[ i ] = { ( uint8_t )val, ( uint8_t )val, ( uint8_t )val, ( uint8_t )255u };
	}

	Window::SetColours( seedColours );

	auto buff = window->GetBuffer();
	
	for ( int i = 0; i < window->GetArea(); ++i )
	{
		buff[ i ] = ConsoleGL::ConsoleColourTable[ i % 16 ];
	}

	Window::SwapBuffer();
	Window::Destroy( window );
	return 0;
}