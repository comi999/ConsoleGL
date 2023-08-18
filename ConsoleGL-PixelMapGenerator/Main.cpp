﻿#include <fstream>
#include <string>
#include <iostream>
#include <Windows.h>

struct Colour { uint8_t /*a, */r, g, b; };

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

	///*Black         */  { 0,   0,   0   },
	///*Dark_Blue     */  { 255, 0,   0   },
	///*Dark_Green    */  { 0,   255, 0   },
	///*Dark_Cyan     */  { 0,   0,   255 },
	///*Dark_Red      */  { 255, 255, 0   },
	///*Dark_Magenta  */  { 255, 0,   255 },
	///*Dark_Yellow   */  { 0,   255, 255 },
	///*Dark_White    */  { 255, 255, 255 },
	///*Bright_Black  */  { 85,  85,  85  },
	///*Bright_Blue   */  { 170, 85,  85  },
	///*Bright_Green  */  { 85,  170, 85  },
	///*Bright_Cyan   */  { 85,  85,  170 },
	///*Bright_Red    */  { 170, 170, 85  },
	///*Bright_Magenta*/  { 170, 85,  170 },
	///*Bright_Yellow */  { 85,  170, 170 },
	///*White         */  { 170, 170, 170 }
};

Colour CombineColour( Colour a_LHS, Colour a_RHS, uint8_t a_Alpha )
{
	static const float Denom = 1.0f / 255.0f;

	if ( a_Alpha == 255 )
	{
		return a_RHS;
	}

	if ( a_Alpha == 0 )
	{
		return a_LHS;
	}

	uint8_t NewA = 255 - ( 255 - a_Alpha ) * ( 1.0f - 255 * Denom );

	if ( 255 < 1.0e-6f )
	{
		return Colour{ 255, 255, 255 };
	}

	float PremultiplyLeft = float( a_Alpha ) / NewA;
	float PremultiplyRight = 255 * ( 1.0f - a_Alpha * Denom ) / NewA;

	a_RHS.r = a_RHS.r * PremultiplyLeft + a_LHS.r * PremultiplyRight;
	a_RHS.g = a_RHS.g * PremultiplyLeft + a_LHS.g * PremultiplyRight;
	a_RHS.b = a_RHS.b * PremultiplyLeft + a_LHS.b * PremultiplyRight;

	return a_RHS;
}

class PixelMapGenerator
{
public:

	using PixelType = CHAR_INFO;

	void Init();
	const PixelType* Data() const { return m_Data; }
	constexpr size_t Size() const { return 256u * 256u * 256u; }

private:

	PixelType m_Data[ 256u * 256u * 256u ];
};

void PixelMapGenerator::Init()
{
	const auto PixelAt = [&]( Colour a_Colour ) -> PixelType&
	{
		return m_Data[ 
			( uint32_t )a_Colour.b * 256u * 256u + 
			( uint32_t )a_Colour.g * 256u + 
			( uint32_t )a_Colour.r
		];
	};

	const auto SetFG = []( PixelType& a_Pixel, uint32_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFFFFFFF0 ) |= static_cast< WORD >( a_Colour );
	};

	const auto SetBG = []( PixelType& a_Pixel, uint32_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFFFFFF0F ) |= ( static_cast< WORD >( a_Colour ) << 4u );
	};

	// Set initial colours.
	for ( uint32_t i = 0; i < 16; ++i )
	{
		PixelType& Pixel = PixelAt( Seeds[ i ] );
		Pixel.Char.UnicodeChar = L' ';
		SetBG( Pixel, i );
	}
	
	// Set remaining colours.
	for ( uint32_t i = 0u, Index = 16u; i < 16u; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( uint32_t j = i + 1u; j < 16u; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( uint32_t k = 0u; k < 3u; ++k )
			{
				// Get alpha.
				//float Alpha = ( float )( k * 64u + 63u ) / 255.0f;
				Colour& SeedColour = Seeds[ Index++ ];

				// Combine.
				//SeedColour.r = ( 1.0f - Alpha ) * Background.r + Alpha * Foreground.r;
				//SeedColour.g = ( 1.0f - Alpha ) * Background.g + Alpha * Foreground.g;
				//SeedColour.b = ( 1.0f - Alpha ) * Background.b + Alpha * Foreground.b;

				//SeedColour.r = Background.r + Alpha * ( Foreground.r - Background.r );
				//SeedColour.g = Background.g + Alpha * ( Foreground.g - Background.g );
				//SeedColour.r = Background.b + Alpha * ( Foreground.b - Background.b );

				SeedColour = CombineColour( Foreground, Background, k * 64u + 63u );

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetFG( Pixel, j );
				SetBG( Pixel, i );
				
				Pixel.Char.UnicodeChar = L'\x2591' + k; // ░▒▓
			}
		}
	}

	for ( uint32_t b = 0u; b < 256u; ++b )
	{
		char ProgressBuffer[ 32 ];
		sprintf_s( ProgressBuffer, "Generating: %.1f%%", ( float )b / 2.55f );
		std::cout << ProgressBuffer << std::endl;


		for ( uint32_t g = 0u; g < 256u; ++g )
			for ( uint32_t r = 0u; r < 256u; ++r )
			{
				Colour c;
				{
					c.r = r;
					c.g = g;
					c.b = b;
				}

				uint32_t MinDistSqrd = 256u * 256u * 256u;
				PixelType& Current = PixelAt( c );
				PixelType Closest;

				for ( Colour Seed : Seeds )
				{
					int32_t DiffR = ( c.r - Seed.r ) * ( c.r - Seed.r );
					int32_t DiffG = ( c.g - Seed.g ) * ( c.g - Seed.g );
					int32_t DiffB = ( c.b - Seed.b ) * ( c.b - Seed.b );

					int32_t DistSqrd = DiffR + DiffG + DiffB;

					if ( DistSqrd == 0 )
					{
						Closest = Current;
						break;
					}

					if ( DistSqrd < MinDistSqrd )
					{
						MinDistSqrd = DistSqrd;
						Closest = PixelAt( Seed );
					}
				}

				Current = Closest;
			}
	}
}

const int width = 80;
const int height = 80;

int main()
{
#ifndef OUTPUT_PATH
	return 1;
#endif

	//std::ofstream Output( OUTPUT_PATH, std::ios::binary | std::ios::out );
	std::ofstream Output( "pixels.map", std::ios::binary | std::ios::out );
	
	if ( !Output.is_open() )
	{
		return 1;
	}

	PixelMapGenerator* Generator = new PixelMapGenerator;
	Generator->Init();

#define SET_WORD_TYPE( Name ) using PacketType = Name; Output << "static const "#Name" PixelMapData[ 256u * 256u * 256u / sizeof( "#Name" ) * sizeof( CHAR_INFO ) ] = {"

	//SET_WORD_TYPE( uint64_t );
	using PixelType = typename PixelMapGenerator::PixelType;
	//const PacketType* Packets = ( const PacketType* )Generator->Data();
	//const size_t PacketsTotal = Generator->Size() * sizeof( PixelType ) / sizeof( PacketType );
	//const size_t PacketsPerLine = 256u;
	//const size_t TotalBytes = PacketsTotal * sizeof( PacketType );
	//
	//// i here is packet index.
	//for ( size_t i = 0u; i < PacketsTotal; i += PacketsPerLine, Packets += PacketsPerLine )
	//{
	//	for ( size_t j = 0u; j < PacketsPerLine; ++j )
	//	{
	//		Output << std::to_string( Packets[ j ] ) << "u,";
	//	}

	//	Output << "\n";

	//	char ProgressBuffer[ 32 ];
	//	sprintf_s( ProgressBuffer, "Writing: %.1f%%", ( float )i * 100.0f / PacketsTotal );
	//	std::cout << ProgressBuffer << std::endl;
	//}

	//Output << "};";

	Output.write( ( const char* )Generator->Data(), Generator->Size() * sizeof( PixelType ) );


	Output.close();

	return 0;
}