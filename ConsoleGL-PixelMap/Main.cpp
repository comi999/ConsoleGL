#include <string>
#include <iostream>
#include <fstream>

#include <PixelMap.hpp>

int main(int argc, char* argv[])
{
	if ( argc == 1 )
	{
		std::ofstream Output( "PixelMap", std::ios::binary | std::ios::out );
	
		if ( !Output.is_open() )
		{
			return 1;
		}

		Output.write( ( const char* )ConsoleGL::Map.Data, sizeof( ConsoleGL::Map.Data ) );
	}
    else if ( argc == 2 )
    {
		std::string outputFile = argv[1];
		std::ofstream Output( outputFile, std::ios::binary | std::ios::out );
	
		if ( !Output.is_open() )
		{
			return 1;
		}

		Output.write( ( const char* )ConsoleGL::Map.Data, sizeof( ConsoleGL::Map.Data ) );
    }
	else
	{ 
        std::cerr << "Usage: " << argv[0] << " <output_file>" << std::endl;
        return 1;
    }

    return 0;
}