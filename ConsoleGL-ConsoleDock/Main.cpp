#include <iostream>

#include <Window.hpp>

int main( int a_ArgCount, const char** a_Args )
{
	if ( a_ArgCount != 2 )
	{
		return 1;
	}

	

	return ConsoleGL::WindowDock::RunListener( a_Args[ 1 ] );
}