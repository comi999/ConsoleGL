#include <ConsoleGL/Colour.hpp>

ConsoleGL::Colour ConsoleGL::Colour::operator+( Colour a_Colour ) const
{
	return Colour(
		
	);
}

ConsoleGL::Colour& ConsoleGL::Colour::operator+=( Colour a_Colour )
{
	return *this;
}

ConsoleGL::Colour ConsoleGL::Colour::operator-( Colour a_Colour ) const
{
	return Colour();
}

ConsoleGL::Colour& ConsoleGL::Colour::operator-=( Colour a_Colour )
{
	return* this;
}

ConsoleGL::Colour ConsoleGL::Colour::operator*( Colour a_Colour ) const
{

	return Colour();
}

ConsoleGL::Colour ConsoleGL::Colour::operator*( float a_Scalar ) const
{
	return Colour();
}

ConsoleGL::Colour& ConsoleGL::Colour::operator*=( Colour a_Colour )
{

	return*this;
}

ConsoleGL::Colour& ConsoleGL::Colour::operator*=( float a_Scalar )
{
	return*this;
}

ConsoleGL::Colour operator*( float a_Scalar, ConsoleGL::Colour a_Colour )
{
	return ConsoleGL::Colour();
}