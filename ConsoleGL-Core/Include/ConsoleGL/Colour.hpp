#pragma once
#include <cstdint>
#include <type_traits>

namespace ConsoleGL
{
	enum class EConsoleColour : uint8_t
	{
		BLACK = 0x0000,
		DARK_BLUE = 0x0001,
		DARK_GREEN = 0x0002,
		DARK_RED = 0x0004,
		DARK_GREY = 0x0008,
		DARK_CYAN = DARK_BLUE | DARK_GREEN,
		DARK_MAGENTA = DARK_BLUE | DARK_RED,
		DARK_YELLOW = DARK_GREEN | DARK_RED,
		BLUE = DARK_GREY | DARK_BLUE,
		GREEN = DARK_GREY | DARK_GREEN,
		RED = DARK_GREY | DARK_RED,
		GREY = DARK_CYAN | DARK_RED,
		CYAN = DARK_GREY | DARK_CYAN,
		MAGENTA = DARK_GREY | DARK_MAGENTA,
		YELLOW = DARK_GREY | DARK_YELLOW,
		WHITE = BLUE | DARK_YELLOW
	};

	static const EConsoleColour ConsoleColourTable[ 16 ]
	{
		ConsoleGL::EConsoleColour::BLACK,
		ConsoleGL::EConsoleColour::DARK_BLUE,
		ConsoleGL::EConsoleColour::DARK_GREEN,
		ConsoleGL::EConsoleColour::DARK_CYAN,
		ConsoleGL::EConsoleColour::DARK_RED,
		ConsoleGL::EConsoleColour::DARK_MAGENTA,
		ConsoleGL::EConsoleColour::DARK_YELLOW,
		ConsoleGL::EConsoleColour::GREY,
		ConsoleGL::EConsoleColour::DARK_GREY,
		ConsoleGL::EConsoleColour::BLUE,
		ConsoleGL::EConsoleColour::GREEN,
		ConsoleGL::EConsoleColour::CYAN,
		ConsoleGL::EConsoleColour::RED,
		ConsoleGL::EConsoleColour::MAGENTA,
		ConsoleGL::EConsoleColour::YELLOW,
		ConsoleGL::EConsoleColour::WHITE
	};

	struct Colour
	{
		uint8_t /*a, */r, g, b;

		//constexpr Colour() : r( 0u ), g( 0u ), b( 0u ), a( 255u ) {}

		//template < typename T, typename = std::enable_if_t< std::is_arithmetic_v< T > > >
		//constexpr Colour( T a_R, T a_G, T a_B, T a_A = []() { if constexpr ( std::is_integral_v< T > ) return 255u; else return 1.0f; }( ) )
		//	: r( []() { if constexpr ( std::is_integral_v< T > ) return 1u; else return 255u; }() * a_R )
		//	, g( []() { if constexpr ( std::is_integral_v< T > ) return 1u; else return 255u; }() * a_G )
		//	, b( []() { if constexpr ( std::is_integral_v< T > ) return 1u; else return 255u; }() * a_B )
		//	, a( []() { if constexpr ( std::is_integral_v< T > ) return 1u; else return 255u; }() * a_A )
		//{}

		//Colour operator*( Colour a_Colour ) const;
		//Colour operator*( float a_Scalar ) const;
		//Colour& operator*=( Colour a_Colour );
		//Colour& operator*=( float a_Scalar );
	};
} // namespace ConsoleGL

//ConsoleGL::Colour operator*( float a_Scalar, ConsoleGL::Colour a_Colour );