#include <fstream>

#include "ConsoleGL/Colour.hpp"

struct PixelMapGenerator
{

};

int main()
{
#ifndef OUTPUT_PATH
	return 1;
#endif

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