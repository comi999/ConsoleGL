#pragma once
#include <type_traits>

#include <glm/glm.hpp>

namespace ConsoleGL
{
	// Provides easy-to-use interface for setting/unsetting bitfield parameters.
	// Use this if enum is laid out in a bitfield layout. ie. Each value is a power of 2.
	template < typename _Enum >
	class EnumBitfield
	{
		static_assert( std::is_enum_v< _Enum >, "Type must be an enum or enum class." );
	
	public:
	
		using EnumType = _Enum;
		using UnderlyingType = std::underlying_type_t< EnumType >;
	
		constexpr EnumBitfield() = default;
		constexpr EnumBitfield( const EnumBitfield& ) = default;
		constexpr EnumBitfield( EnumBitfield&& ) = default;
		constexpr EnumBitfield( const EnumType a_Enum ) : m_Enum{ a_Enum } {}
		constexpr EnumBitfield( const UnderlyingType a_Enum ) : m_Enum{ static_cast< EnumType >( a_Enum ) } {}
		constexpr EnumBitfield( const std::initializer_list< EnumType > a_Enums );
		constexpr EnumBitfield( const std::initializer_list< UnderlyingType > a_Enums );
		constexpr EnumBitfield( const std::initializer_list< bool > a_Enums );
		constexpr EnumBitfield& operator=( const EnumBitfield& ) = default;
		constexpr EnumBitfield& operator=( EnumBitfield&& ) = default;
		constexpr EnumBitfield& operator=( const EnumType a_Enum ) { m_Enum = a_Enum; return *this; }
		constexpr EnumBitfield& operator=( const UnderlyingType a_Enum ) { m_Enum = static_cast< EnumType >( a_Enum ); return *this; }
		constexpr EnumBitfield& operator=( const std::initializer_list< EnumType > a_Enums );
		constexpr EnumBitfield& operator=( const std::initializer_list< UnderlyingType > a_Enums );
		constexpr EnumBitfield& operator=( const std::initializer_list< bool > a_Enums );
	
		constexpr bool IsSet( const EnumType a_Enum ) const { return IsSet( static_cast< UnderlyingType >( a_Enum ) ); }
		constexpr bool IsSet( const UnderlyingType a_Enum ) const { return static_cast< bool >( a_Enum & static_cast< UnderlyingType >( m_Enum ) ); }
		constexpr void Set( const EnumType a_Enum ) { Set( static_cast< UnderlyingType >( a_Enum ) ); }
		constexpr void Set( const UnderlyingType a_Enum ) { m_Enum = static_cast< EnumType >( static_cast< UnderlyingType >( m_Enum ) | a_Enum ); }
		constexpr void Set( const EnumType a_Enum, const bool a_Enable ) { Set( static_cast< UnderlyingType >( a_Enum ), a_Enable ); }
		constexpr void Set( const UnderlyingType a_Enum, const bool a_Enable ) { if ( a_Enable ) Set( a_Enum ); else Unset( a_Enum ); }
		constexpr void Unset( const EnumType a_Enum ) { Unset( static_cast< UnderlyingType >( a_Enum ) ); }
		constexpr void Unset( const UnderlyingType a_Enum ) { m_Enum = static_cast< EnumType >( static_cast< UnderlyingType >( m_Enum ) & ~a_Enum ); }
		constexpr void Reset() { m_Enum = static_cast< EnumType >( 0 ); }
		constexpr operator EnumType() const { return m_Enum; }
		constexpr operator UnderlyingType() const { return static_cast< UnderlyingType >( m_Enum ); }
	
	private:
	
		EnumType m_Enum{};
	};


	using ScanFn = void(*)( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_State );
	using RasterFn = void(*)( const glm::vec4& a_Position, const float* a_Data, uint32_t a_Stride, void* a_State );

	// Scan a triangle with a scan function.
	// a_P is glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	void ScanTriangle( const glm::vec4* a_P, ScanFn a_ScanFn, void* a_State );

	// Rasterize a triangle with a raster function.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	void RasterizeFlat( const glm::vec4* a_P, const float** a_Data, uint32_t a_Stride, RasterFn a_RasterFn, void* a_State );

	// Rasterize a triangle with a raster function.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	void RasterizeAffine( const glm::vec4* a_P, const float** a_Data, uint32_t a_Stride, RasterFn a_RasterFn, void* a_State );

	// Rasterize a triangle with a raster function with perspective.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	void RasterizePerspective( const glm::vec4* a_P, const float** a_Data, uint32_t a_Stride, RasterFn a_RasterFn, void* a_State );

	// Rasterize a triangle with a raster function with flat, affine, and perspective mixed.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	// a_FlatStride is how many elements from 0th index are to be flat interpolated.
	// a_AffineStride is how many elements after a_FlatStride are to be affine interpolated.
	// a_PerspectiveStride is how many elements after a_AFfineStride are to be perspective interpolated.
	void Rasterize( const glm::vec4* a_P, const float** a_Data, uint32_t a_FlatStride, uint32_t a_AffineStride, uint32_t a_PerspectiveStride, RasterFn a_RasterFn, void* a_State );
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >::EnumBitfield( const std::initializer_list< EnumType > a_Enums )
{
	for ( EnumType Enum : a_Enums ) Set( Enum );
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >::EnumBitfield( const std::initializer_list< UnderlyingType > a_Enums )
{
	for ( EnumType Enum : a_Enums ) Set( Enum );
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >::EnumBitfield( const std::initializer_list< bool > a_Enums )
{
	for ( size_t i = 0u; i < a_Enums.size(); ++i ) Set( static_cast< EnumType >( 1u << i ), a_Enums.begin()[ i ] );
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >& ConsoleGL::EnumBitfield< _Enum >::operator=( const std::initializer_list< EnumType > a_Enums )
{
	for ( EnumType Enum : a_Enums ) Set( Enum ); return *this;
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >& ConsoleGL::EnumBitfield< _Enum >::operator=( const std::initializer_list< UnderlyingType > a_Enums )
{
	for ( EnumType Enum : a_Enums ) Set( Enum ); return *this;
}

template < typename _Enum >
constexpr ConsoleGL::EnumBitfield< _Enum >& ConsoleGL::EnumBitfield< _Enum >::operator=( const std::initializer_list< bool > a_Enums )
{
	for ( size_t i = 0u; i < a_Enums.size(); ++i ) Set( static_cast< EnumType >( 1u << i ), a_Enums.begin()[ i ] ); return *this;
}