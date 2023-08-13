#pragma once
#include <Windows.h>
#include <ConsoleGL/ConsoleColour.hpp>

namespace ConsoleGL
{
	struct Pixel : protected CHAR_INFO
	{
		using BaseType = CHAR_INFO;
		using AsciiType = CHAR;
		using UnicodeType = WCHAR;
		using WordType = WORD;

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
		Pixel( ConsoleColour a_ConsoleColour )
		{
			BaseType::Char.UnicodeChar = 0x20; // space
			SetBackground( a_ConsoleColour );
		}

		AsciiType& Ascii() { return Char.AsciiChar; }
		UnicodeType& Unicode() { return Char.UnicodeChar; }
		WordType& Attributes() { return CHAR_INFO::Attributes; }
		ConsoleColour GetForeground() { return static_cast< ConsoleColour >( 0xF & CHAR_INFO::Attributes ); }
		ConsoleColour GetBackgroundColour() { return static_cast< ConsoleColour >( ( 0xF0 & CHAR_INFO::Attributes ) >> 4 ); }
		void SetForeground( ConsoleColour a_ConsoleColour ) { CHAR_INFO::Attributes &= 0xFFF0; CHAR_INFO::Attributes |= static_cast< WORD >( a_ConsoleColour ); }
		void SetBackground( ConsoleColour a_ConsoleColour ) { CHAR_INFO::Attributes &= 0xFF0F; CHAR_INFO::Attributes |= static_cast< WORD >( a_ConsoleColour ) << 4; }
		operator BaseType& () { return *this; }
		operator const BaseType& () const { return *this; }

	private:

		friend class ConsoleWindow;
	};
} // namespace ConsoleGL