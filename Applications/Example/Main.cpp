#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#undef CreateWindow
#include <ConsoleGL.hpp>

#include <stb_image/stb_image.h>

int Width = 100, Height = 100;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	ConsoleGL::Window* window0 = ConsoleGL::CreateWindow( "window0", Width, Height, 8, 8, 2 );
	ConsoleGL::SetActiveWindow( window0 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::DEFAULT );
	/*auto pix = *ConsoleGL::MapColourToPixel( { 255, 0, 0, 255 } );
	ConsoleGL::DrawTriangleFilled( buff0, 0, 45, 90, 23, 43, 12, pix );
	pix = *ConsoleGL::MapColourToPixel( { 0, 128, 25, 255 } );
	ConsoleGL::DrawTriangle( buff0, 0, 45, 90, 23, 43, 12, pix );
	pix = *ConsoleGL::MapColourToPixel( { 0, 0, 255, 255 } );*/

	float i = 0.0f;

	auto background = *ConsoleGL::MapColourToPixel( { 255, 255, 200, 255 } );
	auto pix0 = *ConsoleGL::MapColourToPixel( { 255, 0, 0, 255 } );
	auto pix1 = *ConsoleGL::MapColourToPixel( { 0, 255, 0, 255 } );

	while ( true )
	{
		auto buff0 = ConsoleGL::GetWindowBuffer( window0 );

		struct Tri
		{
			ConsoleGL::Coord P0 = { 0, 0 };
			ConsoleGL::Coord P1 = { ( uint32_t )( Width/2 ), ( uint32_t )( Height-1 ) };
			ConsoleGL::Coord P2 = { ( uint32_t )( Width-1 ), ( uint32_t )( 0 ) };
		} tri;
		
		auto fn = +[]( const uint32_t X, const uint32_t Y, void* a_Tri ) -> ConsoleGL::Pixel
		{
			auto P0 = static_cast< Tri* >( a_Tri )->P0;
			auto P1 = static_cast< Tri* >( a_Tri )->P1;
			auto P2 = static_cast< Tri* >( a_Tri )->P2;

			float x = X, y = Y, x1 = P0.x, y1 = P0.y, x2 = P1.x, y2 = P1.y, x3 = P2.x, y3 = P2.y;

			float L1 = ((y2-y3)*(x-x3)+(x3-x2)*(y-y3))/((y2-y3)*(x1-x3)+(x3-x2)*(y1-y3));
			float L2 = ((y3-y1)*(x-x3)+(x1-x3)*(y-y3))/((y2-y3)*(x1-x3)+(x3-x2)*(y1-y3));
			float L3 = 1.0f - L1 - L2;

			ConsoleGL::Colour c{ L1, L2, L3 };
			auto pixel = *ConsoleGL::MapColourToPixel( c );
			return pixel;
		};
		
		ConsoleGL::DrawTriangleFilledFn( buff0, tri.P0, tri.P1, tri.P2, fn, &tri );
		ConsoleGL::SwapWindowBuffer();
	}

	/*ConsoleGL::Window* window1 = ConsoleGL::CreateWindow( "window1", Width, Height, 8, 8, 2 );
	ConsoleGL::SetActiveWindow( window1 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::SEPIA );
	auto buff1 = ConsoleGL::GetWindowBuffer( window1 );
	pix = *ConsoleGL::MapColourToPixel( { 123, 234, 23, 255 } );
	ConsoleGL::DrawTriangleFilled( buff1, 12, 57, 21, 23, 43, 12, pix );
	pix = *ConsoleGL::MapColourToPixel( { 0, 128, 25, 255 } );
	ConsoleGL::DrawTriangle( buff1, 12, 57, 21, 23, 43, 12, pix );
	pix = *ConsoleGL::MapColourToPixel( { 0, 0, 255, 255 } );
	ConsoleGL::DrawRect( buff1, 10, 10, 12, 54, pix );
	ConsoleGL::SwapWindowBuffer();
	system("pause");*/
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
