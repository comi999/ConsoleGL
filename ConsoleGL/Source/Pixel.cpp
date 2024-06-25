#include <Pixel.hpp>
#include <Colour.hpp>

ConsoleGL::Pixel ConsoleGL::Pixel::Empty = CHAR_INFO{ L'\x20', ( ConsoleGL::Pixel::WordType )ConsoleGL::EConsoleColour::WHITE };
ConsoleGL::Pixel ConsoleGL::Pixel::OneQuarter = CHAR_INFO{ L'\x2591', ( ConsoleGL::Pixel::WordType )ConsoleGL::EConsoleColour::WHITE };
ConsoleGL::Pixel ConsoleGL::Pixel::Half = CHAR_INFO{ L'\x2592', ( ConsoleGL::Pixel::WordType )ConsoleGL::EConsoleColour::WHITE };
ConsoleGL::Pixel ConsoleGL::Pixel::ThreeQuarter = CHAR_INFO{ L'\x2593', ( ConsoleGL::Pixel::WordType )ConsoleGL::EConsoleColour::WHITE };
ConsoleGL::Pixel ConsoleGL::Pixel::Solid = CHAR_INFO{ L'\x2588', ( ConsoleGL::Pixel::WordType )ConsoleGL::EConsoleColour::WHITE };

#if __has_include( "PixelMap.inl" )
	#include "PixelMap.inl"
#else
	static constexpr ConsoleGL::Pixel* PixelMapData = nullptr;
#endif
const ConsoleGL::Pixel* ConsoleGL::Pixel::PixelMap = ( const Pixel* )PixelMapData;
