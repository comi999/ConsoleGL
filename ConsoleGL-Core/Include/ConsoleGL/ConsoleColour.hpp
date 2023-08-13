#pragma once
#include <cstdint>

namespace ConsoleGL
{
enum class ConsoleColour : uint8_t
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

static const ConsoleColour ConsoleColourTable[ 16 ]
{
	ConsoleGL::ConsoleColour::BLACK,
	ConsoleGL::ConsoleColour::DARK_BLUE,
	ConsoleGL::ConsoleColour::DARK_GREEN,
	ConsoleGL::ConsoleColour::DARK_CYAN,
	ConsoleGL::ConsoleColour::DARK_RED,
	ConsoleGL::ConsoleColour::DARK_MAGENTA,
	ConsoleGL::ConsoleColour::DARK_YELLOW,
	ConsoleGL::ConsoleColour::GREY,
	ConsoleGL::ConsoleColour::DARK_GREY,
	ConsoleGL::ConsoleColour::BLUE,
	ConsoleGL::ConsoleColour::GREEN,
	ConsoleGL::ConsoleColour::CYAN,
	ConsoleGL::ConsoleColour::RED,
	ConsoleGL::ConsoleColour::MAGENTA,
	ConsoleGL::ConsoleColour::YELLOW,
	ConsoleGL::ConsoleColour::WHITE
};
} // namespace ConsoleGL