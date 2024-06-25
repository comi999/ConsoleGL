#include <fstream>
#include <string>
#include <iostream>
#include <Windows.h>

#include <Pixel.hpp>
#include <Colour.hpp>

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

ConsoleGL::Colour Seeds[ 376 ] =
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

namespace ConsoleGL
{
class PixelMapGenerator
{
public:

	using PixelType = CHAR_INFO;

	void Init();
	const PixelType* Data() const { return m_Data; }
	constexpr size_t Size() const { return 256u * 256u * 256u; }

private:

	PixelType m_Data[ 256u * 256u * 256u ] {};
};

void PixelMapGenerator::Init()
{
	const auto PixelAt = [&]( Colour a_Colour ) -> PixelType&
	{
		return m_Data[ *( uint32_t* )&a_Colour ];
	};

	const auto SetFG = []( PixelType& a_Pixel, uint32_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_Colour );
	};

	const auto SetBG = []( PixelType& a_Pixel, uint32_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_Colour ) << 4u );
	};

	// Set initial colours.
	for ( uint32_t i = 0; i < 16; ++i )
	{
		PixelType& Pixel = PixelAt( Seeds[ i ] );
		Pixel.Char.UnicodeChar = L'\x20';
		SetFG( Pixel, i );
	}

	// Set remaining colours.
	for ( uint32_t i = 0u, Index = 16u; i < 15u; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( uint32_t j = i + 1u; j < 16u; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( uint32_t k = 0u; k < 3u; ++k )
			{
				// Get alpha.
				float Alpha = ( k * 64u + 63u ) / 255.0f;
				Colour& SeedColour = Seeds[ Index++ ];

				// Combine.
				SeedColour.r = Alpha * Background.r + ( 1.0f - Alpha ) * Foreground.r;
				SeedColour.g = Alpha * Background.g + ( 1.0f - Alpha ) * Foreground.g;
				SeedColour.b = Alpha * Background.b + ( 1.0f - Alpha ) * Foreground.b;

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetBG( Pixel, i );
				SetFG( Pixel, j );
				
				Pixel.Char.UnicodeChar = L'\x2591' + k; // ░▒▓
			}
		}
	}

	for ( uint32_t i = 0; i < 256u * 256u * 256u; ++i )
	{
		Colour c = *( Colour* )&i;
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

			if ( DistSqrd <= MinDistSqrd )
			{
				MinDistSqrd = DistSqrd;
				Closest = PixelAt( Seed );
			}
		}

		PixelAt( c ) = Closest;
	}
}
}

const int width = 80;
const int height = 80;

int main()
{
	std::ofstream Output( "PixelMap.inl", std::ios::binary | std::ios::out );
	
	if ( !Output.is_open() )
	{
		return 1;
	}

	ConsoleGL::PixelMapGenerator* Generator = new ConsoleGL::PixelMapGenerator;
	Generator->Init();

#define SET_WORD_TYPE( Name ) using PacketType = Name; Output << "static const "#Name" PixelMapData[ 256u * 256u * 256u / sizeof( "#Name" ) * sizeof( CHAR_INFO ) ] = {"

	SET_WORD_TYPE( uint64_t );
	using PixelType = typename ConsoleGL::PixelMapGenerator::PixelType;
	const PacketType* Packets = ( const PacketType* )Generator->Data();
	const size_t PacketsTotal = Generator->Size() * sizeof( PixelType ) / sizeof( PacketType );
	const size_t PacketsPerLine = 256u;
	const size_t TotalBytes = PacketsTotal * sizeof( PacketType );
	
	// i here is packet index.
	for ( size_t i = 0u; i < PacketsTotal; i += PacketsPerLine, Packets += PacketsPerLine )
	{
		for ( size_t j = 0u; j < PacketsPerLine; ++j )
		{
			Output << std::to_string( Packets[ j ] ) << "u,";
		}

		Output << "\n";

		printf( "\33[2K\r" );
		printf( "Writing: %.1f%%", ( float )i * 100.0f / PacketsTotal );
	}

	Output << "};";
	Output.close();

	return 0;
}