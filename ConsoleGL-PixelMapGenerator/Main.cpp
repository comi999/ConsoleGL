#include <fstream>
#include <string>
#include <Windows.h>

struct Colour { uint8_t a, r, g, b; };

class PixelMapGenerator
{
public:

	using PixelType = CHAR_INFO;

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
		return m_Data[ 256u * 256u * a_Colour.r + 256u * a_Colour.g + a_Colour.b ];
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
	for ( int i = 0; i < 15; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( int j = i + 1; j < 16; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( int k = 1; k < 4; ++k )
			{
				// Get alpha.
				// ( ( k - 1 ) * 64 + 63 ) / 255.0f;
				float Alpha = k * 0.2509804f - 0.003921569f;
	
				// Create and set Colour Seed.
				Colour& SeedColour = Seeds[ Index++ ];

				// Combine.
				// = bg - alpha * bg + alpha * fg
				SeedColour.r = Background.r + Alpha * ( Foreground.r - Background.r );
				SeedColour.g = Background.g + Alpha * ( Foreground.g - Background.g );
				SeedColour.b = Background.b + Alpha * ( Foreground.b - Background.b );

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetBG( Pixel, i );
				SetFG( Pixel, j );
				Pixel.Char.UnicodeChar = L'\x2590' + k;
			}
		}
	}

	for ( uint32_t i = 0u; i < 256u * 256u * 256u; ++i )
	{
		Colour c;
		{
			uint32_t Index = i << 8u;
			c = *( Colour* )&Index; 
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

	for ( size_t i = 0; i < Size; i += Increment, Data += Increment )
	{
		Write( Data + Increment, Increment );
	}

	Output << "};";

	Output.close();
	return 0;
}