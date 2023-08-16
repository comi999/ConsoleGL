#pragma once
#include <vector>
#include <Windows.h>
#include <ConsoleGL/Colour.hpp>

namespace ConsoleGL
{
	struct Pixel : protected CHAR_INFO
	{
		using BaseType = CHAR_INFO;
		using AsciiType = CHAR;
		using UnicodeType = WCHAR;
		using WordType = WORD;

		static const size_t PixelMapLength = 256u * 256u * 256u / sizeof( uint64_t ) * sizeof( BaseType );
		static const uint64_t PixelMap[ PixelMapLength ];

		static Pixel Empty;
		static Pixel OneQuarter;
		static Pixel Half;
		static Pixel ThreeQuarter;
		static Pixel Solid;

		Pixel() = default;
		Pixel( const Pixel& ) = default;
		Pixel( Pixel&& ) = default;
		Pixel& operator=( const Pixel& ) = default;
		Pixel& operator=( Pixel&& ) = default;
		Pixel( BaseType a_Base ) : BaseType( a_Base ) {}
		Pixel( EConsoleColour a_ConsoleColour ) { *this = Empty; SetBackground( a_ConsoleColour ); }
		Pixel( Colour a_Colour ) { *this = ( ( const Pixel* )PixelMap )[ *( uint32_t* )&a_Colour >> 8u ]; }
		AsciiType& Ascii() { return Char.AsciiChar; }
		UnicodeType& Unicode() { return Char.UnicodeChar; }
		WordType& Attributes() { return CHAR_INFO::Attributes; }
		EConsoleColour GetForeground() { return static_cast< EConsoleColour >( 0xF & CHAR_INFO::Attributes ); }
		EConsoleColour GetBackgroundColour() { return static_cast< EConsoleColour >( ( 0xF0 & CHAR_INFO::Attributes ) >> 4 ); }
		void SetForeground( EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFFF0 ) |= static_cast< WORD >( a_ConsoleColour ); }
		void SetBackground( EConsoleColour a_ConsoleColour ) { ( CHAR_INFO::Attributes &= 0xFF0F ) |= ( static_cast< WORD >( a_ConsoleColour ) << 4 ); }
		operator BaseType& () { return *this; }
		operator const BaseType& () const { return *this; }

	private:

		friend class ConsoleWindow;
	};

	//class PixelColourMap
	//{
	//public:

	//	static constexpr size_t Length = 256u * 256u * 256u;
	//	static const Pixel DefaultData[ Length ];

	//	PixelColourMap();
	//	PixelColourMap( const PixelColourMap& a_PixelColourMap );
	//	PixelColourMap( PixelColourMap&& a_PixelColourMap ) noexcept;
	//	~PixelColourMap();

	//	// Given an array of 16 RGBA colours, 
	//	void Init( const Colour* a_SeedColours )
	//	{

	//		//0.2989 * R + 0.5870 * G + 0.1140 * B
	//		//for ( int i = 0; i < 16; ++i )
	//		//{
	//		//	Colour& c = SeedColours[ i ];
	//		//	Vector3 crgb = { ( float )c.R / 255, ( float )c.G / 255, ( float )c.B / 255 };
	//		//	float weighting = 0.2989 * crgb.x + 0.5870 * crgb.y + 0.1140 * crgb.z;
	//		//	Vector3 n( weighting );
	//		//	c.R = 255 * n.x;
	//		//	c.G = 255 * n.y;
	//		//	c.B = 255 * n.z;
	//		//}

	//		// sepia
	//		//tr = 0.393R + 0.769G + 0.189B
	//		//tg = 0.349R + 0.686G + 0.168B
	//		//tb = 0.272R + 0.534G + 0.131B
	//		//for ( int i = 0; i < 16; ++i )
	//		//{
	//		//	Colour& c = SeedColours[ i ];
	//		//	Vector3 crgb = { ( float )c.R / 255, ( float )c.G / 255, ( float )c.B / 255 };
	//		//
	//		//	Vector3 n;
	//		//	n.x = 0.393 * crgb.x + 0.769 * crgb.y + 0.189 * crgb.z;
	//		//	n.y = 0.349 * crgb.x + 0.686 * crgb.y + 0.168 * crgb.z;
	//		//	n.z = 0.272 * crgb.x + 0.534 * crgb.y + 0.131 * crgb.z;
	//		//
	//		//	n = Math::Clamp( n, Vector3::Zero, Vector3::One );
	//		//
	//		//	c.R = 255 * n.x;
	//		//	c.G = 255 * n.y;
	//		//	c.B = 255 * n.z;
	//		//}
	//		//
	//		//if ( s_Active.Load() )
	//		//{
	//		//	return true;
	//		//}
	//		//
	//		//return s_Active.BuildAndSave();
	//	}

	//	//void Build()
	//	//{
	//	//	auto Convert = []( Colour a_Colour )
	//	//	{
	//	//		return
	//	//			static_cast< int >( a_Colour.R ) +
	//	//			static_cast< int >( a_Colour.G ) * 256 +
	//	//			static_cast< int >( a_Colour.B ) * 65536;
	//	//	};
	//	//
	//	//	// Set initial colours.
	//	//	for ( int i = 0; i < 16; ++i )
	//	//	{
	//	//		Pixel NewPixel;
	//	//		NewPixel.SetForegroundColour( ConsoleColours[ i ] );
	//	//		NewPixel.Unicode() = L'\x2588'; // Block 
	//	//		m_PixelMap[ Convert( SeedColours[ i ] ) ] = NewPixel;
	//	//	}
	//	//
	//	//	size_t Index = 16;
	//	//
	//	//	// Set remaining colours.
	//	//	for ( int i = 0; i < 16; ++i )
	//	//	{
	//	//		Colour Background = SeedColours[ i ];
	//	//		Pixel NewPixel;
	//	//
	//	//		for ( int j = i + 1; j < 16; ++j )
	//	//		{
	//	//			Colour Foreground = SeedColours[ j ];
	//	//
	//	//			for ( int k = 1; k < 4; ++k )
	//	//			{
	//	//				// Set alpha.
	//	//				Foreground.A = ( k - 1 ) * 64 + 63;
	//	//
	//	//				// Create and set Colour Seed.
	//	//				Colour& SeedColour = SeedColours[ Index++ ];
	//	//				SeedColour = Background + Foreground;
	//	//
	//	//				// Set Pixel.
	//	//				NewPixel.SetBackgroundColour( ConsoleColours[ i ] );
	//	//				NewPixel.SetForegroundColour( ConsoleColours[ j ] );
	//	//				NewPixel.Unicode() = L'\x2590' + k; // Dithering characters.
	//	//				m_PixelMap[ Convert( SeedColour ) ] = NewPixel;
	//	//			}
	//	//		}
	//	//	}
	//	//
	//	//	// Extrapolate to empty regions in cube.
	//	//	for ( int B = 0; B < 256; ++B )
	//	//	{
	//	//		for ( int G = 0; G < 256; ++G )
	//	//		{
	//	//			for ( int R = 0; R < 256; ++R )
	//	//			{
	//	//				Vector3Int DiffVector;
	//	//				int MinDistSqrd = 16777216;
	//	//				Pixel& Current = m_PixelMap[ Convert( Colour( R, G, B ) ) ];
	//	//				Pixel Closest;
	//	//
	//	//				for ( Colour Seed : SeedColours )
	//	//				{
	//	//					DiffVector.x = R - Seed.R;
	//	//					DiffVector.y = G - Seed.G;
	//	//					DiffVector.z = B - Seed.B;
	//	//					int DistSqrd = Math::LengthSqrd( DiffVector );
	//	//
	//	//					if ( DistSqrd == 0 )
	//	//					{
	//	//						Closest = Current;
	//	//						break;
	//	//					}
	//	//
	//	//					if ( DistSqrd < MinDistSqrd )
	//	//					{
	//	//						MinDistSqrd = DistSqrd;
	//	//						Closest = m_PixelMap[ Convert( Seed ) ];
	//	//					}
	//	//				}
	//	//
	//	//				Current = Closest;
	//	//			}
	//	//		}
	//	//	}
	//	//}
	//	//
	//	//bool BuildAndSave()
	//	//{
	//	//	Build();
	//	//	return Save();
	//	//}
	//	//
	//	//bool Load()
	//	//{
	//	//	std::fstream File;
	//	//	File.open( "./Resources/colours.map", std::ios::binary | std::ios::in );
	//	//
	//	//	if ( !File.is_open() )
	//	//	{
	//	//		return false;
	//	//	}
	//	//
	//	//	File.read( reinterpret_cast< char* >( m_PixelMap ), 16777216 * sizeof( Pixel ) );
	//	//	File.close();
	//	//	return true;
	//	//}
	//	//
	//	//bool Save()
	//	//{
	//	//	std::fstream File;
	//	//	File.open( "./Resources/colours.map", std::ios::binary | std::ios::out );
	//	//
	//	//	if ( !File.is_open() )
	//	//	{
	//	//		return false;
	//	//	}
	//	//
	//	//	File.write( reinterpret_cast< char* >( m_PixelMap ), 16777216 * sizeof( Pixel ) );
	//	//	File.close();
	//	//	return true;
	//	//}
	//	//
	//	//Pixel ConvertColour( Colour a_Colour ) const
	//	//{
	//	//	return m_PixelMap[
	//	//		static_cast< int >( a_Colour.R ) +
	//	//			static_cast< int >( a_Colour.G ) * 256 +
	//	//			static_cast< int >( a_Colour.B ) * 65536 ];
	//	//}
	//	//
	//	//static const PixelColourMap& Get()
	//	//{
	//	//	return s_Active;
	//	//}
	//	//
	//	//inline static Colour SeedColours[ 376 ] =
	//	//{
	//	//	{ 0,   0,   0   }, //Black          
	//	//	{ 255, 0,   0   }, //Dark_Blue      
	//	//	{ 0,   255, 0   }, //Dark_Green     
	//	//	{ 0,   0,   255 }, //Dark_Cyan      
	//	//	{ 255, 255, 0   }, //Dark_Red       
	//	//	{ 255, 0,   255 }, //Dark_Magenta   
	//	//	{ 0,   255, 255 }, //Dark_Yellow    
	//	//	{ 255, 255, 255 }, //Dark_White     
	//	//	{ 85,  85,  85  }, //Bright_Black   
	//	//	{ 170, 85,  85  }, //Bright_Blue    
	//	//	{ 85,  170, 85  }, //Bright_Green   
	//	//	{ 85,  85,  170 }, //Bright_Cyan    
	//	//	{ 170, 170, 85  }, //Bright_Red     
	//	//	{ 170, 85,  170 }, //Bright_Magenta 
	//	//	{ 85,  170, 170 }, //Bright_Yellow  
	//	//	{ 170, 170, 170 }  //White
	//	//};

	//private:

	//	Pixel* m_Data;
	//};
} // namespace ConsoleGL