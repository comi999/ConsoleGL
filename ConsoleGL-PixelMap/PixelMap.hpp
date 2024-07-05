#pragma once
#include <ConsoleGL.hpp>

namespace ConsoleGL
{
	// Char represents character that gives the mixing percentage.
	// Percentage represents foreground intensity.
	struct CharMixInfo
	{
		Pixel::CharType Symbol;
		float Percentage;
	};

	// Mixing info array.
	constexpr CharMixInfo CharMixInfos[]
	{
		{ L'\x2593', 0.25f },
		{ L'\x2592', 0.50f },
		{ L'\x2591', 0.75f },
	};

	struct PixelMap
	{
		PixelMap()
		{
			// Seed colours are those that can be created from a foreground colour, a background colour and a character to mix them.
			// There are 16 possible true colours that are set from a space (no foreground) and a background colour.

			// There are then 15! possible colour combinations. For each of these combinations, there can be n variants, where n
			// is the number of mixing chars that are defined.

			constexpr size_t NumMixingChars = sizeof( CharMixInfos ) / sizeof( CharMixInfo );
			constexpr size_t NumColourCombinations = 16 * 15 / ( 2 * 1 ); // nCr = 16C2 = 16!/(2!(16 - 2)!)
			constexpr size_t NumTotalColours = NumColourCombinations * NumMixingChars + 16;

			// An array in 256x256x256 RGB space of mixed colours. These will
			// be placed inside the seed cube at their RGB locations.
			// There is a colour placed at all 8 corners of the cube, and then another 8 colours
			// placed inside to describe an internal cube that is half the width, and centred.
			Colour Seeds[ NumTotalColours ] =
			{
				/*Dark_Black    */  { 0,   0,   0   },
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
	
			// Set initial seed colours first.
			for ( size_t i = 0u; i < 16u; ++i )
			{
				Data[ ToIndex( Seeds[ i ] ) ] = Pixel{ L'\x20', static_cast< EConsoleColour >( i ) };
			}

			// Set mixed seed colours.
			for ( size_t i = 0u, Index = 16u; i < 15u; ++i )
			{
				const Colour Background = Seeds[ i ];
					
				for ( size_t j = i + 1u; j < 16u; ++j )
				{
					const Colour Foreground = Seeds[ j ];
			
					for ( size_t k = 0u; k < NumMixingChars; ++k )
					{
						// Get alpha.
						//const float Alpha = ( k * 64u + 63u ) / 255.0f;
						const float Alpha = CharMixInfos[ k ].Percentage;
						Colour& SeedColour = Seeds[ Index++ ];
						
						// Combine.
						SeedColour.r = ( uint8_t )( Alpha * Background.r + ( 1.0f - Alpha ) * Foreground.r );
						SeedColour.g = ( uint8_t )( Alpha * Background.g + ( 1.0f - Alpha ) * Foreground.g );
						SeedColour.b = ( uint8_t )( Alpha * Background.b + ( 1.0f - Alpha ) * Foreground.b );
						
						// Set pixel.
						Data[ ToIndex( SeedColour ) ] = Pixel{
							CharMixInfos[ k ].Symbol,
							static_cast< EConsoleColour >( i ),
							static_cast< EConsoleColour >( j )
						};
					}
				}
			}

			uint32_t Index = 0;

			for ( uint32_t r = 0u; r < PIXEL_MAP_SIZE; ++r )
			for ( uint32_t g = 0u; g < PIXEL_MAP_SIZE; ++g )
			for ( uint32_t b = 0u; b < PIXEL_MAP_SIZE; ++b )
			{

				const Colour Corrected = Colour{
					static_cast< uint8_t >( r << PIXEL_MAP_MIP_LEVEL ),
					static_cast< uint8_t >( g << PIXEL_MAP_MIP_LEVEL ),
					static_cast< uint8_t >( b << PIXEL_MAP_MIP_LEVEL ),
					static_cast< uint8_t >( 0 )
				};

				int32_t MinDistSqrd = PIXEL_MAP_VOLUME;
				Pixel Closest;

				for ( const Colour Seed : Seeds )
				{
					int32_t DiffR = Corrected.r - Seed.r;
					int32_t DiffG = Corrected.g - Seed.g;
					int32_t DiffB = Corrected.b - Seed.b;
	
					DiffR *= DiffR;
					DiffG *= DiffG;
					DiffB *= DiffB;
	
					const int32_t DistSqrd = DiffR + DiffG + DiffB;
	
					if ( DistSqrd == 0 )
					{
						Closest = Data[ ToIndex( Seed ) ];
						break;
					}
	
					if ( DistSqrd <= MinDistSqrd )
					{
						MinDistSqrd = DistSqrd;
						Closest = Data[ ToIndex( Seed ) ];
					}
				}

				Data[ ToIndex( Corrected ) ] = Closest;
			}
		}

		// Converts colour to an index into the pixel map.
		constexpr static size_t ToIndex( const Colour a_Colour )
		{
			const uint8_t R = a_Colour.r >> PIXEL_MAP_MIP_LEVEL;
			const uint8_t G = a_Colour.g >> PIXEL_MAP_MIP_LEVEL;
			const uint8_t B = a_Colour.b >> PIXEL_MAP_MIP_LEVEL;

			return PIXEL_MAP_SIZE * PIXEL_MAP_SIZE * R + PIXEL_MAP_SIZE * G + B;
		}

		constexpr Pixel operator[]( const Colour a_Colour ) const { return Data[ ToIndex( a_Colour ) ]; }
		constexpr Pixel& operator[]( const Colour a_Colour ) { return Data[ ToIndex( a_Colour ) ]; }
		constexpr Pixel operator[]( const size_t a_Index ) const { return Data[ a_Index ]; }
		constexpr Pixel& operator[]( const size_t a_Index ) { return Data[ a_Index ]; }

		Pixel Data[ PIXEL_MAP_VOLUME ];
	};

	static const PixelMap Map;
}