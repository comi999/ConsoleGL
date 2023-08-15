#include <fstream>
#include <Windows.h>

struct Colour { uint8_t r, g, b; };

class PixelMapGenerator
{
public:

	using PixelType = CHAR_INFO;

	void Init();
	const PixelType* GetData() const;

private:

	PixelType* m_Data;
};


void PixelMapGenerator::Init()
{
	m_Data = new PixelType[ 256u * 256u * 256u ];

	const auto PixelAt = [&]( Colour a_Colour ) -> PixelType&
	{
		return m_Data[ 256u * 256u * a_Colour.r + 256u * a_Colour.g + a_Colour.b ];
	};

	/*static const EConsoleColour ConsoleColourTable[ 16 ]
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
	};*/

	const auto SetFG = []( PixelType& a_Pixel, uint8_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_Colour );
	};

	const auto SetBG = []( PixelType& a_Pixel, uint8_t a_Colour )
	{
		( a_Pixel.Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_Colour ) << 4 );
	};

	// unique pairs: 15 + 14 + ... + 2 + 1 = 15/2 * ( 15 + 1 ) = 120
	// unique colours pairs = unique pairs * 3 variants = 120 * 3 = 360
	// solid colours = 16
	// total = 360 + 16 = 376.

	// Set initial colours
	Colour Seeds[ 376 ] =
	{
		{ 0,   0,   0   }, //Black          
		{ 255, 0,   0   }, //Dark_Blue      
		{ 0,   255, 0   }, //Dark_Green     
		{ 0,   0,   255 }, //Dark_Cyan      
		{ 255, 255, 0   }, //Dark_Red       
		{ 255, 0,   255 }, //Dark_Magenta   
		{ 0,   255, 255 }, //Dark_Yellow    
		{ 255, 255, 255 }, //Dark_White     
		{ 85,  85,  85  }, //Bright_Black   
		{ 170, 85,  85  }, //Bright_Blue    
		{ 85,  170, 85  }, //Bright_Green   
		{ 85,  85,  170 }, //Bright_Cyan    
		{ 170, 170, 85  }, //Bright_Red     
		{ 170, 85,  170 }, //Bright_Magenta 
		{ 85,  170, 170 }, //Bright_Yellow  
		{ 170, 170, 170 }  //White
	};

	// Set initial colours.
	for ( int i = 0; i < 16; ++i )
	{
		PixelType& Pixel = PixelAt( Seeds[ i ] );
		SetBG( Pixel, i );
		Pixel.Char.UnicodeChar = L'\x20';
	}
	
	size_t Index = 16;
	
	// Set remaining colours.
	for ( int i = 0; i < 16; ++i )
	{
		Colour Background = Seeds[ i ];
	
		for ( int j = i + 1; j < 16; ++j )
		{
			Colour Foreground = Seeds[ j ];
	
			for ( int k = 1; k < 4; ++k )
			{
				// Get alpha.
				float Alpha = ( ( k - 1 ) * 64 + 63 ) / 255.0f;
	
				// Create and set Colour Seed.
				Colour& SeedColour = Seeds[ Index++ ];

				// Combine.
				SeedColour.r = ( 1.0f - Alpha ) * Background.r + Alpha * Foreground.r;
				SeedColour.g = ( 1.0f - Alpha ) * Background.g + Alpha * Foreground.g;
				SeedColour.b = ( 1.0f - Alpha ) * Background.b + Alpha * Foreground.b;

				// Set pixel.
				PixelType& Pixel = PixelAt( SeedColour );
				SetBG( Pixel, i );
				SetFG( Pixel, j );
				Pixel.Char.UnicodeChar = L'\x2590' + k;
			}
		}
	}
}

const PixelMapGenerator::PixelType* PixelMapGenerator::GetData() const
{

}

int main()
{
#ifndef OUTPUT_PATH
	return 1;
#endif
	PixelMapGenerator s;
	std::ofstream Output( OUTPUT_PATH, std::ofstream::out );
	
	if ( !Output.is_open() )
	{
		return 1;
	}

	Output << "const int8_t ConsoleGL::Pixel::PixelMap[ PixelMapLength * sizeof( BaseType ) ] =\n";

	const auto WriteOutput = [&Output]( const void* a_Data, size_t a_Count )
	{
		Output << "\n    \"";
		Output.write( ( const char* )a_Data, a_Count );
		Output << "\"";
	};



	Output << ";";
	Output.close();
	return 0;
}