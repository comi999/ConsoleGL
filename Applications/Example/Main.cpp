#include <Window.hpp>

using ConsoleGL::Window;
using ConsoleGL::Colour;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	Window* window0 = Window::Create( "window0", 100, 100, 8, 8 );
	Window* window1 = Window::Create( "window1", 100, 100, 8, 8 );
	Window* window2 = Window::Create( "window2", 100, 100, 8, 8 );
	Window* window3 = Window::Create( "window3", 100, 100, 8, 8 );
	Window::SetActive( window0 );
	Window::SetColours( ConsoleGL::EColourSet::GREYSCALE );
	Window::SetActive( window1 );
	Window::SetColours( ConsoleGL::EColourSet::SEPIA );
	Window::SetActive( window2 );
	Window::SetColours( ConsoleGL::EColourSet::DEFAULT );
	Window::SetActive( window3 );
	Window::SetColours( ConsoleGL::EColourSet::LEGACY );

	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		auto buff0 = window0->GetBuffer();
		auto buff1 = window1->GetBuffer();
		auto buff2 = window2->GetBuffer();
		auto buff3 = window3->GetBuffer();

		for ( uint32_t y = 0u; y < 100; ++y )
		{
			for ( uint32_t x = 0u; x < 100; ++x )
			{
				uint32_t g = ( float )y / ( float )( 100 - 1u ) * 255.0f;
				uint32_t r = ( float )x / ( float )( 100 - 1u ) * 255.0f;
				buff0[ y * 100 + x ] = Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b };
				buff1[ y * 100 + x ] = Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b };
				buff2[ y * 100 + x ] = Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b };
				buff3[ y * 100 + x ] = Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b };
			}
		}

		Window::SetActive( window0 );
		Window::SwapBuffer();
		Window::SetActive( window1 );
		Window::SwapBuffer();
		Window::SetActive( window2 );
		Window::SwapBuffer();
		Window::SetActive( window3 );
		Window::SwapBuffer();
	}

	Window::Destroy( window0 );
	Window::Destroy( window1 );
	Window::Destroy( window2 );
	Window::Destroy( window3 );
}