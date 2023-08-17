#include <fstream>
#include <string>
#include <iostream>
#include <Windows.h>

struct Colour { uint8_t a, r, g, b; };

class PixelMapGenerator
{
public:

	using PixelType = CHAR_INFO;

	PixelMapGenerator() = default;
	~PixelMapGenerator() { delete m_Data; }

	void Init();
	const PixelType* Data() const { return m_Data; }
	constexpr size_t Size() const { return 256u * 256u * 256u; }

private:

	PixelType* m_Data;
};

void PixelMapGenerator::Init()
{
	m_Data = new PixelType[ Size() ];

	const auto PixelAt = [&]( Colour a_Colour ) -> PixelType&
	{
		return m_Data[ ( ( uint32_t )a_Colour.r * 256u * 256u ) + ( ( uint32_t )a_Colour.g * 256u ) + a_Colour.b ];
		//return m_Data[ *( uint32_t* )&a_Colour >> 8u ];
	};

	const auto SetFG = []( PixelType& a_Pixel, uint8_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_Colour );
	};

	const auto SetBG = []( PixelType& a_Pixel, uint8_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_Colour ) << 4 );
	};

	// unique pairs = 16 nCr 2 = 16! / (16 - 2)! / 2! = 360
	// solid colours = 16
	// total variations = 16 + 360 * 3 = 376 total colours

	// Console colours
	// -    = 0  // BLACK
	// b    = 1  // BLUE
	// g    = 2  // GREEN
	// bg   = 3  // CYAN
	// r    = 4  // RED
	// br   = 5  // MAGENTA
	// gr   = 6  // YELLOW
	// bgr  = 7  // BRIGHT_GREY
	// i    = 8  // GREY
	// ib   = 9  // BRIGHT_BLUE
	// ig   = 10 // BRIGHT_GREEN
	// ibg  = 11 // BRIGHT_CYAN
	// ir   = 12 // BRIGHT_RED
	// ibr  = 13 // BRIGHT_MAGENTA
	// igr  = 14 // BRIGHT_YELLOW
	// ibgr = 15 // WHITE

	// Set initial colours
	Colour Seeds[ 376 ] =
	{
		{ 0,   0,   0   }, // BLACK
		{ 0,   0,   255 }, // BLUE
		{ 0,   255, 0   }, // GREEN
		{ 0,   128, 128 }, // CYAN
		{ 255, 0,   0   }, // RED
		{ 128, 0,   128 }, // MAGENTA
		{ 128, 128, 0   }, // YELLOW
		{ 170, 170, 170 }, // BRIGHT_GREY
		{ 85,  85,  85  }, // GREY
		{ 128, 128, 255 }, // BRIGHT_BLUE
		{ 128, 255, 128 }, // BRIGHT_GREEN
		{ 128, 192, 192 }, // BRIGHT_CYAN
		{ 255, 128, 128 }, // BRIGHT_RED
		{ 192, 128, 192 }, // BRIGHT_MAGENTA
		{ 192, 192, 128 }, // BRIGHT_YELLOW
		{ 255, 255, 255 }, // WHITE
	};

	// Set initial colours.
	for ( int i = 0; i < 16; ++i )
	{
		PixelType& Pixel = PixelAt( Seeds[ i ] );
		SetBG( Pixel, i );
		SetFG( Pixel, i );
		Pixel.Char.UnicodeChar = L'\x20';
	}
	
	size_t Index = 16;

	// Set remaining colours.
	for ( uint32_t i = 0; i < 15; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( uint32_t j = i + 1; j < 16; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( uint32_t k = 1; k < 4; ++k )
			{
				// Get alpha.
				// ( ( k - 1 ) * 64 + 63 ) / 255.0f;
				float Alpha = k * 0.2509804f - 0.003921569f;
				//float Alpha = ( float )( ( k - 1u ) * 64u + 63u ) / 255.0f;
	
				// Create and set Colour Seed.
				Colour& SeedColour = Seeds[ Index++ ];

				// Combine.
				// = bg - alpha * bg + alpha * fg
				SeedColour.r = Background.r + Alpha * ( Foreground.r - Background.r );
				SeedColour.g = Background.g + Alpha * ( Foreground.g - Background.g );
				SeedColour.b = Background.b + Alpha * ( Foreground.b - Background.b );
				//SeedColour.r = ( 1.0f - Alpha ) * ( float )Background.r + Alpha * ( float )Foreground.r;
				//SeedColour.g = ( 1.0f - Alpha ) * ( float )Background.g + Alpha * ( float )Foreground.g;
				//SeedColour.b = ( 1.0f - Alpha ) * ( float )Background.b + Alpha * ( float )Foreground.b;

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetBG( Pixel, i );
				SetFG( Pixel, j );
				Pixel.Char.UnicodeChar = L'\x2590' + k;
			}
		}
	}

	for ( uint32_t r = 0u; r < 256u; ++r )
	for ( uint32_t g = 0u; g < 256u; ++g )
	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		Colour c;
		{
			//uint32_t Index = i << 8u;
			//c = *( Colour* )&Index;
			c.r = r;
			c.g = g;
			c.b = b;
		}
		
		uint32_t MinDistSqrd = -1;
		PixelType Closest;
		
		for ( Colour Seed : Seeds )
		{
			int32_t DiffR = c.r - Seed.r;
			int32_t DiffG = c.g - Seed.g;
			int32_t DiffB = c.b - Seed.b;

			DiffR *= DiffR;
			DiffG *= DiffG;
			DiffB *= DiffB;

			int32_t DistSqrd = DiffR + DiffG + DiffB;
		
			if ( DistSqrd == 0 )
			{
				Closest = PixelAt( Seed );
				break;
			}
		
			if ( DistSqrd < MinDistSqrd )
			{
				MinDistSqrd = DistSqrd;
				Closest = PixelAt( Seed );
			}
		}

		PixelAt( c ) = Closest;
	}
}

using PixelType = typename PixelMapGenerator::PixelType;

const int width = 80;
const int height = 80;

void WriteOutput( const PixelType* a_Pixels, size_t a_Count )
{
	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = width - 1;
	rect.Bottom = height - 1;

	WriteConsoleOutput(
		GetStdHandle( STD_OUTPUT_HANDLE ),
		a_Pixels,
		{ ( short )width, ( short )height },
		{ 0, 0 },
		&rect );
}

void SetupWindow()
{
	STARTUPINFO si;
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof( pi ) );

	auto output = GetStdHandle( STD_OUTPUT_HANDLE );
	auto input = GetStdHandle( STD_INPUT_HANDLE );
	auto window = GetConsoleWindow();

	// Set console font.
	CONSOLE_FONT_INFOEX FontInfo;
	FontInfo.cbSize = sizeof( FontInfo );
	FontInfo.nFont = 0;
	FontInfo.dwFontSize = { ( short )8, ( short )8 };
	FontInfo.FontFamily = FF_DONTCARE;
	FontInfo.FontWeight = FW_NORMAL;
	wcscpy_s( FontInfo.FaceName, L"Terminal" );
	SetCurrentConsoleFontEx( output, false, &FontInfo );

	// Get screen buffer info object.
	CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
	ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
	GetConsoleScreenBufferInfoEx( output, &ScreenBufferInfo );
	//// set colours here
	SetConsoleScreenBufferInfoEx( output, &ScreenBufferInfo );

	// Get largest possible window size that can fit on screen.
	COORD LargestWindow = GetLargestConsoleWindowSize( output );

	// Set window region rect.
	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = width - 1;
	rect.Bottom = height - 1;

	// Set console attributes.
	SetConsoleScreenBufferSize( output, LargestWindow );
	SetConsoleWindowInfo( output, true, &rect );
	GetConsoleScreenBufferInfoEx( output, &ScreenBufferInfo );
	SetConsoleScreenBufferSize( output, { ( short )width, ( short )height } );

	// Set cursor attributes.
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo( output, &CursorInfo );
	CursorInfo.bVisible = false;
	SetConsoleCursorInfo( output, &CursorInfo );

	// Set window attributes.
	SetWindowLong( window, GWL_STYLE, WS_CAPTION | DS_MODALFRAME | WS_MINIMIZEBOX | WS_SYSMENU );
	SetWindowPos( window, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW );
	SetConsoleMode( input, /*ENABLE_EXTENDED_FLAGS |*/ ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT );
}

int main()
{
#ifndef OUTPUT_PATH
	return 1;
#endif

	PixelMapGenerator s;
	std::ofstream Output( OUTPUT_PATH, std::ios::binary | std::ios::out );
	
	if ( !Output.is_open() )
	{
		return 1;
	}

	PixelMapGenerator Generator;
	Generator.Init();
	auto Data = ( const uint8_t* )Generator.Data();
	auto Size = Generator.Size() * sizeof( PixelMapGenerator::PixelType );

	SetupWindow();
	static PixelType Buffer[ width * height ];

	for ( uint32_t r = 0; r < 256u; ++r )
	{
		for ( uint32_t g = 0; g < 256u; ++g )
		{
			for ( uint32_t b = 0; b < 256u; ++b )
			{
				float x = ( float )b / 255.0f;
				float y = ( float )g / 255.0f;
				x *= ( width - 1 );
				y *= ( height - 1 );
				uint32_t index = y * width + x;
				Buffer[ index ] = ( ( const PixelType* )Data )[ r * 256u * 256u + g * 256u + b ];
			}
		}

		WriteOutput( ( const PixelType* )Buffer, Generator.Size() );
	}


	const auto Write = [ &Output ]( const void* a_Data, size_t a_Length )
	{
		a_Length /= sizeof( uint64_t );
		const uint64_t* Data = ( const uint64_t* )a_Data;

		for ( size_t i = 0; i < a_Length; ++i )
		{
			Output << std::to_string( Data[ i ] ) << "u,";
		}

		Output << "\n";
	};

	Output << "const uint64_t ConsoleGL::Pixel::PixelMap[ PixelMapLength ] = {";

	const size_t Increment = 256u * sizeof( PixelMapGenerator::PixelType );

	for ( size_t i = 0u; i < Size; i += Increment, Data += Increment )
	{
		Write( Data, Increment );
	}

	Output << "};";

	Output.close();
	return 0;
}