#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#undef CreateWindow
#include <ConsoleGL.hpp>

int Width = 100, Height = 100;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	ConsoleGL::Window* window0 = ConsoleGL::CreateWindow( "window0", Width, Height, 8, 8, 2 );
	ConsoleGL::SetActiveWindow( window0 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::DEFAULT );
	auto buff0 = ConsoleGL::GetWindowBuffer( window0 );
	auto pix = *ConsoleGL::MapColourToPixel( { 255, 0, 0, 255 } );
	ConsoleGL::DrawTriangleFilled( buff0, 0, 45, 90, 23, 43, 12, pix );
	pix = *ConsoleGL::MapColourToPixel( { 0, 255, 0, 255 } );
	ConsoleGL::DrawTriangle( buff0, 0, 45, 90, 23, 43, 12, pix );
	ConsoleGL::SwapWindowBuffer();

	/*ConsoleGL::Window* window0 = ConsoleGL::CreateWindow( "window0", Width, Height, 8, 8, 2 );
	ConsoleGL::Window* window1 = ConsoleGL::CreateWindow( "window1", Width, Height, 8, 8, 2 );
	ConsoleGL::Window* window2 = ConsoleGL::CreateWindow( "window2", Width, Height, 8, 8, 2 );
	ConsoleGL::SetActiveWindow( window0 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::DEFAULT );
	ConsoleGL::SetActiveWindow( window1 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::SEPIA );
	ConsoleGL::SetActiveWindow( window2 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::GREYSCALE );

	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		auto buff0 = ConsoleGL::GetPixelBufferPixels( ConsoleGL::GetWindowBuffer( window0 ) );
		auto buff1 = ConsoleGL::GetPixelBufferPixels( ConsoleGL::GetWindowBuffer( window1 ) );
		auto buff2 = ConsoleGL::GetPixelBufferPixels( ConsoleGL::GetWindowBuffer( window2 ) );

		for ( uint32_t y = 0u; y < Height; ++y )
		{
			for ( uint32_t x = 0u; x < Width; ++x )
			{
				uint32_t g = ( float )y / ( float )( Width - 1u ) * 255.0f;
				uint32_t r = ( float )x / ( float )( Height - 1u ) * 255.0f;
				buff0[ y * Width + x ] = *ConsoleGL::MapColourToPixel(ConsoleGL::Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b });
				buff1[ y * Width + x ] = *ConsoleGL::MapColourToPixel(ConsoleGL::Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b });
				buff2[ y * Width + x ] = *ConsoleGL::MapColourToPixel(ConsoleGL::Colour{ ( uint8_t )r, ( uint8_t )g, ( uint8_t )b });
			}
		}

		ConsoleGL::SetActiveWindow( window0 );
		ConsoleGL::SwapWindowBuffer();
		ConsoleGL::SetActiveWindow( window1 );
		ConsoleGL::SwapWindowBuffer();
		ConsoleGL::SetActiveWindow( window2 );
		ConsoleGL::SwapWindowBuffer();
	}

	ConsoleGL::DestroyWindow( window0 );
	ConsoleGL::DestroyWindow( window1 );
	ConsoleGL::DestroyWindow( window2 );*/
}
