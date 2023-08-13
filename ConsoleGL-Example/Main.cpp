#include <iostream>

#include <ConsoleGL/ConsoleWindow.hpp>
#include <ConsoleGL/ConsoleColour.hpp>

using Window = ConsoleGL::Window;

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	//auto window1 = ConsoleGL::Window::Create( "Test_Title1", 100, 100, 8, 8 );
	//auto window2 = ConsoleGL::Window::Create( "Test Title2", 50, 50, 8, 8 );
	//auto window3 = ConsoleGL::Window::Create( "Test Title3", 75, 75, 8, 8 );
	//auto window4 = ConsoleGL::Window::Create( "Test Title4", 125, 125, 8, 8 );
	//ConsoleGL::Window::SetActive( window3 );
	//std::cout << "Hi there3!" << std::endl;
	//ConsoleGL::Window::SetActive( window1 );
	//std::cout << "Hi there1!" << std::endl;
	//
	//ConsoleGL::Window::Destroy( window1 );
	//ConsoleGL::Window::Destroy( window2 );
	//ConsoleGL::Window::Destroy( window3 );
	//ConsoleGL::Window::Destroy( window4 );

	auto window = Window::Create( "Window", 40, 40 );
	Window::SetActive( window );
	Window::SetBuffer( ConsoleGL::ConsoleColour::BLUE );
	Window::SwapBuffer();
	Window::Destroy();

	return 0;
}