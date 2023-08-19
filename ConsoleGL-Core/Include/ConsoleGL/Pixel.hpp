#pragma once
#include <vector>
#include <Windows.h>
#include <ConsoleGL/Colour.hpp>

#define IS_LITTLE_ENDIAN  ('ABCD'==0x41424344UL)
#define IS_BIG_ENDIAN     ('ABCD'==0x44434241UL)
#define IS_UNKNOWN_ENDIAN (IS_LITTLE_ENDIAN == IS_BIG_ENDIAN)

namespace ConsoleGL
{
	struct Pixel : protected CHAR_INFO
	{
		using BaseType = CHAR_INFO;
		using AsciiType = CHAR;
		using UnicodeType = WCHAR;
		using WordType = WORD;

		static const Pixel* PixelMap;

		static Pixel Empty;
		static Pixel OneQuarter;
		static Pixel Half;
		static Pixel ThreeQuarter;
		static Pixel Solid;

		Pixel() = default;
		Pixel( const Pixel& ) = default;
		Pixel( Pixel&& ) = default;
		Pixel& operator=( const Pixel& ) = default;
		Pixel& operator=( Pixel&& ) = default;
		Pixel( BaseType a_Base ) : BaseType( a_Base ) {}
		Pixel( EConsoleColour a_ConsoleColour ) { *this = Empty; SetBackground( a_ConsoleColour ); }
		Pixel( Colour a_Colour ) {
			////*this = PixelMap[ ( uint32_t )a_Colour.b * 256u * 256u + ( uint32_t )a_Colour.g * 256u + ( uint32_t )a_Colour.r ];
			//uint32_t Index = *( uint32_t* )&a_Colour >> 8u;
			//*this = PixelMap[ Index ];

			if constexpr ( IS_BIG_ENDIAN )
			{
			}
			else
			{

			}
		}
		AsciiType& Ascii() { return Char.AsciiChar; }
		UnicodeType& Unicode() { return Char.UnicodeChar; }
		WordType& Attributes() { return CHAR_INFO::Attributes; }
		EConsoleColour GetForeground() { return static_cast< EConsoleColour >( 0x0F & CHAR_INFO::Attributes ); }
		EConsoleColour GetBackground() { return static_cast< EConsoleColour >( ( 0xF0 & CHAR_INFO::Attributes ) >> 4u ); }
		void SetForeground( EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_ConsoleColour ); }
		void SetBackground( EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_ConsoleColour ) << 4u ); }
		operator BaseType& () { return *this; }
		operator const BaseType& () const { return *this; }

	private:

		friend class ConsoleWindow;
	};
} // namespace ConsoleGL

#undef IS_LITTLE_ENDIAN
#undef IS_BIG_ENDIAN
#undef IS_UNKNOWN_ENDIAN