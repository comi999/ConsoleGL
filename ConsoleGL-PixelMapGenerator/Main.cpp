#include <fstream>
#include <string>
#include <iostream>
#include <Windows.h>

//Some handy defines magic, thanks overflow
#define IS_LITTLE_ENDIAN  ('ABCD'==0x41424344UL) //41 42 43 44 = 'ABCD' hex ASCII code
#define IS_BIG_ENDIAN     ('ABCD'==0x44434241UL) //44 43 42 41 = 'DCBA' hex ASCII code
#define IS_UNKNOWN_ENDIAN (IS_LITTLE_ENDIAN == IS_BIG_ENDIAN)

struct Colour { uint8_t a, r, g, b; };

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

	/*Black         */  { 0u, 0,   0,   0   },
	/*Dark_Blue     */  { 0u, 255, 0,   0   },
	/*Dark_Green    */  { 0u, 0,   255, 0   },
	/*Dark_Cyan     */  { 0u, 0,   0,   255 },
	/*Dark_Red      */  { 0u, 255, 255, 0   },
	/*Dark_Magenta  */  { 0u, 255, 0,   255 },
	/*Dark_Yellow   */  { 0u, 0,   255, 255 },
	/*Dark_White    */  { 0u, 255, 255, 255 },
	/*Bright_Black  */  { 0u, 85,  85,  85  },
	/*Bright_Blue   */  { 0u, 170, 85,  85  },
	/*Bright_Green  */  { 0u, 85,  170, 85  },
	/*Bright_Cyan   */  { 0u, 85,  85,  170 },
	/*Bright_Red    */  { 0u, 170, 170, 85  },
	/*Bright_Magenta*/  { 0u, 170, 85,  170 },
	/*Bright_Yellow */  { 0u, 85,  170, 170 },
	/*White         */  { 0u, 170, 170, 170 }
};

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
		if constexpr ( IS_BIG_ENDIAN )
		{

		}
		else
		{

		}

		/*return m_Data[ 
			( uint32_t )a_Colour.b * 256u * 256u + 
			( uint32_t )a_Colour.g * 256u + 
			( uint32_t )a_Colour.r
		];*/

		return m_Data[ *( uint32_t* )&a_Colour >> 8u ];
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
		SetBG( Pixel, i );
		SetFG( Pixel, i );
	}
	uint32_t Index = 16u;
	// Set remaining colours.
	for ( uint32_t i = 0u; i < 15u; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( uint32_t j = i + 1u; j < 16u; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( uint32_t k = 0u; k < 3u; ++k )
			{
				// Get alpha.
				float Alpha = k;
				Alpha *= 64;
				Alpha += 63;
				Alpha /= 255.0f;
				Colour& SeedColour = Seeds[ Index++ ];

				Alpha = 1.0f - Alpha;
				
				// Combine.
				SeedColour.r = ( 1.0f - Alpha ) * Background.r + Alpha * Foreground.r;
				SeedColour.g = ( 1.0f - Alpha ) * Background.g + Alpha * Foreground.g;
				SeedColour.b = ( 1.0f - Alpha ) * Background.b + Alpha * Foreground.b;

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetBG( Pixel, i );
				SetFG( Pixel, j );
				
				Pixel.Char.UnicodeChar = L'\x2591' + k; // ░▒▓
			}
		}
	}
	//printf( "\33[2K\r" );
	//printf( "Generating: %.1f%%", ( float )b / 2.55f );

	//for ( int32_t b = 0u; b < 256u; ++b )
	//	for ( int32_t g = 0u; g < 256u; ++g )
	//		for ( int32_t r = 0u; r < 256u; ++r )
	for ( uint32_t i = 0; i < 256u * 256u * 256u; ++i )
			{
				Colour c;
				uint32_t Index = i >> 8u;
				{
					c = *( Colour* )&Index;
					/*c.r = r;
					c.g = g;
					c.b = b;*/
				}

				int32_t r = c.r;
				int32_t g = c.g;
				int32_t b = c.b;

				uint32_t MinDistSqrd = -1;
				PixelType& Current = PixelAt( c );
				PixelType Closest = PixelType();

				for ( Colour Seed : Seeds )
				{
					int32_t DiffR = r - Seed.r;// * ( c.r - Seed.r );
					int32_t DiffG = g - Seed.g;// * ( c.g - Seed.g );
					int32_t DiffB = b - Seed.b;// * ( c.b - Seed.b );

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

				Current = Closest;
			}

	//printf( "\n" );
}

const int width = 80;
const int height = 80;

int main()
{
#ifndef OUTPUT_PATH
	return 1;
#endif

	std::ofstream Output( OUTPUT_PATH, std::ios::binary | std::ios::out );
	//std::ofstream Output( "pixels.map", std::ios::binary | std::ios::out );
	
	if ( !Output.is_open() )
	{
		return 1;
	}

	PixelMapGenerator* Generator = new PixelMapGenerator;
	Generator->Init();

#define SET_WORD_TYPE( Name ) using PacketType = Name; Output << "static const "#Name" PixelMapData[ 256u * 256u * 256u / sizeof( "#Name" ) * sizeof( CHAR_INFO ) ] = {"

	SET_WORD_TYPE( uint64_t );
	using PixelType = typename PixelMapGenerator::PixelType;
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

	//Output.write( ( const char* )Generator->Data(), Generator->Size() * sizeof( PixelType ) );

	Output.close();

	return 0;
}