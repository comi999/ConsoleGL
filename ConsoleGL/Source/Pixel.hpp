#pragma once
#include <Windows.h>
#include <Colour.hpp>

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
		Pixel( const BaseType a_Base ) : BaseType( a_Base ) {}
		Pixel( const EConsoleColour a_ConsoleColour ) { *this = Empty; SetBackground( a_ConsoleColour ); }
		Pixel( const Colour a_Colour ) { *this = PixelMap[ *( uint32_t* )&a_Colour ]; }
		AsciiType& Ascii() { return Char.AsciiChar; }
		UnicodeType& Unicode() { return Char.UnicodeChar; }
		WordType& Attributes() { return CHAR_INFO::Attributes; }
		EConsoleColour GetForeground() const { return static_cast< EConsoleColour >( 0x0F & CHAR_INFO::Attributes ); }
		EConsoleColour GetBackground() const { return static_cast< EConsoleColour >( ( 0xF0 & CHAR_INFO::Attributes ) >> 4u ); }
		void SetForeground( const EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_ConsoleColour ); }
		void SetBackground( const EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_ConsoleColour ) << 4u ); }
		operator BaseType& () { return *this; }
		operator const BaseType& () const { return *this; }

	private:

		friend class ConsoleWindow;
	};
} // namespace ConsoleGL