#include <ConsoleGL/Colour.hpp>
#include <algorithm>

//ConsoleGL::Colour ConsoleGL::Colour::operator*( Colour a_Colour ) const
//{
//	return Colour(
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( r ) * a_Colour.r / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( g ) * a_Colour.g / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( b ) * a_Colour.b / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a ) * a_Colour.a / 255u ) )
//	);
//}
//
//ConsoleGL::Colour ConsoleGL::Colour::operator*( float a_Scalar ) const
//{
//	a_Scalar = std::max( 0.0f, a_Scalar );
//	return Colour(
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( r ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( g ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( b ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a ) * a_Scalar ) )
//	);
//}
//
//ConsoleGL::Colour& ConsoleGL::Colour::operator*=( Colour a_Colour )
//{
//	return *this = Colour(
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( r ) * a_Colour.r / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( g ) * a_Colour.g / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( b ) * a_Colour.b / 255u ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a ) * a_Colour.a / 255u ) )
//	);
//}
//
//ConsoleGL::Colour& ConsoleGL::Colour::operator*=( float a_Scalar )
//{
//	a_Scalar = std::max( 0.0f, a_Scalar );
//	return *this = Colour(
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( r ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( g ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( b ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a ) * a_Scalar ) )
//	);
//}
//
//ConsoleGL::Colour operator*( float a_Scalar, ConsoleGL::Colour a_Colour )
//{
//	a_Scalar = std::max( 0.0f, a_Scalar );
//	return ConsoleGL::Colour(
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a_Colour.r ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a_Colour.g ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a_Colour.b ) * a_Scalar ) ),
//		std::min( 255u, ( uint32_t )( static_cast< uint32_t >( a_Colour.a ) * a_Scalar ) )
//	);
//}