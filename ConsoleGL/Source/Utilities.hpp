#pragma once
#include <type_traits>

#include <glm/glm.hpp>
#include <ConsoleGL.hpp>

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

	template < typename _Element >
	class DrawBuffer
	{
	public:
#define IN_PLACE_ELEMENT +[]( Coord a_Coord, void* a_State ) { return *( const ElementType* )a_State; }, const_cast< ElementType* >( &a_Element )
		using ElementType = _Element;

        DrawBuffer() = default;
	    DrawBuffer( uint32_t a_Width, uint32_t a_Height );
	    DrawBuffer( const DrawBuffer& a_DrawBuffer );
	    DrawBuffer( DrawBuffer&& a_DrawBuffer ) noexcept;
	    DrawBuffer& operator=( const DrawBuffer& a_DrawBuffer );
	    DrawBuffer& operator=( DrawBuffer&& a_DrawBuffer ) noexcept;
	    ~DrawBuffer();
	
	    uint32_t GetWidth() const { return m_Width; }
	    uint32_t GetHeight() const { return m_Height; }
	    uint32_t GetSize() const { return m_Width * m_Height; }
	    const ElementType* GetElements() const { return m_Elements; }
	    ElementType* GetElements() { return m_Elements; }
	    
	    // ElementType operations.
		void SetElement( const uint32_t a_Index, const ElementType a_Element ) { m_Elements[ a_Index ] = a_Element; }
	    void SetElement( const Coord a_Position, const ElementType a_Element ) { SetElement( a_Position.y * m_Width + a_Position.x, a_Element ); }
	
	    void SetElements( const uint32_t a_Index, const uint32_t a_Count, const ElementType a_Element ) { for ( ElementType* Begin = m_Elements + a_Index, *End = Begin + a_Count; Begin != End; ++Begin ) *Begin = a_Element; }
	    void SetElements( const Coord a_Position, const uint32_t a_Count, const ElementType a_Element ) { SetElements( a_Position.y * m_Width + a_Position.x, a_Count, a_Element ); }
	
	    // Region operations
	    void SetBuffer( const ElementType a_Element ) { SetElements( 0u, GetSize(), a_Element ); }
	    void SetBuffer( FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawLine( const Seg& a_Seg, const ElementType a_Element ) { DrawLine( a_Seg, IN_PLACE_ELEMENT ); }
	    void DrawLine( const Seg& a_Seg, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawHorizontalLine( const Coord a_Begin, const uint32_t a_Length, const ElementType a_Element ) { SetElements( a_Begin, a_Length, a_Element ); }
	    void DrawHorizontalLine( Coord a_Begin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawVerticalLine( const Coord a_Begin, const uint32_t a_Length, const ElementType a_Element ) { DrawVerticalLine( a_Begin, a_Length, IN_PLACE_ELEMENT ); }
	    void DrawVerticalLine( Coord a_Begin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawTriangle( const Tri& a_Tri, const ElementType a_Element ) { DrawTriangle( a_Tri, IN_PLACE_ELEMENT ); }
	    void DrawTriangle( const Tri& a_Tri, FragmentFn a_FragmentFn, void* a_State );
	    
	    void DrawTriangleFilled( const Tri& a_Tri, const ElementType a_Element ) { DrawTriangleFilled( a_Tri, IN_PLACE_ELEMENT ); }
	    void DrawTriangleFilled( const Tri& a_Tri, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawRect( const Rect& a_Rect, const ElementType a_Element ) { DrawRect( a_Rect, IN_PLACE_ELEMENT ); }
	    void DrawRect( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_State );
	    void DrawRect( const Rect& a_Rect, const float a_Radians, const ElementType a_Element ) { DrawRect( a_Rect, a_Radians, IN_PLACE_ELEMENT ); }
	    void DrawRect( const Rect& a_Rect, float a_Radians, FragmentFn a_FragmentFn, void* a_State );
	    
	    void DrawRectFilled( const Rect& a_Rect, const ElementType a_Element ) { DrawRectFilled( a_Rect, IN_PLACE_ELEMENT ); }
	    void DrawRectFilled( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_State );
	    void DrawRectFilled( const Rect& a_Rect, const float a_Radians, const ElementType a_Element ) { DrawRectFilled( a_Rect, a_Radians, IN_PLACE_ELEMENT ); }
	    void DrawRectFilled( const Rect& a_Rect, float a_Radians, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawCircle( const Coord a_Centre, const uint32_t a_Radius, const ElementType a_Element ) { DrawCircle( a_Centre, a_Radius, IN_PLACE_ELEMENT ); }
	    void DrawCircle( Coord a_Centre, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawCircleFilled( const Coord a_Centre, const uint32_t a_Radius, const ElementType a_Element ) { DrawCircleFilled( a_Centre, a_Radius, IN_PLACE_ELEMENT ); }
	    void DrawCircleFilled( Coord a_Centre, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_State );
	
	    void DrawEllipse( const Coord a_Centre, const Coord a_Radius, const ElementType a_Element ) { DrawEllipse( a_Centre, a_Radius, IN_PLACE_ELEMENT ); }
	    void DrawEllipse( Coord a_Centre, Coord a_Radius, FragmentFn a_FragmentFn, void* a_State );
	    
	    void DrawEllipseFilled( const Coord a_Centre, const Coord a_Radius, const ElementType a_Element ) { DrawEllipseFilled( a_Centre, a_Radius, IN_PLACE_ELEMENT ); }
	    void DrawEllipseFilled( Coord a_Centre, Coord a_Radius, FragmentFn a_FragmentFn, void* a_State );
	
	    // Text operations
	    //...
	
	    // Mapping functions
	   void DrawOnto( DrawBuffer* a_Buffer, Coord a_Origin );
	    
	private:
	
	    ElementType* m_Elements;
	    uint32_t m_Width;
	    uint32_t m_Height;
#undef IN_PLACE_ELEMENT
	};

#define DEFINE_BUFFER( Name, Type ) \
    class Name : public DrawBuffer< Type > \
    { \
    public: \
		Name() = default; \
        Name( const uint32_t a_Width, const uint32_t a_Height ) : DrawBuffer( a_Width, a_Height ) {}; \
        Name( const Name& ) = default; \
        Name( Name&& ) = default; \
        Name& operator=( const Name& ) = default; \
        Name& operator=( Name&& ) = default; \
        ~Name() = default; \
    };

    DEFINE_BUFFER( PixelBuffer, Pixel );
    DEFINE_BUFFER( ColourBuffer, Colour );
    DEFINE_BUFFER( DepthBuffer, float );

#undef DEFINE_BUFFER

	using ScanLineFn = void(*)( const glm::vec4& a_Position, float a_T, void* a_State );
	using ScanTriangleFn = void(*)( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_State );

	using RasterFn = void(*)( const glm::vec4& a_Position, const float* a_Data, uint32_t a_Stride, void* a_State );

	// Rasterize a line with a raster function with flat, affine, and perspective mixed.
	// a_P is a glm::vec4[2] of the 2 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	// a_FlatStride is how many elements from 0th index are to be flat interpolated.
	// a_AffineStride is how many elements after a_FlatStride are to be affine interpolated.
	// a_PerspectiveStride is how many elements after a_AFfineStride are to be perspective interpolated.
	inline void RasterizeLine( const glm::vec4* a_P, const float** a_Data, const uint32_t a_FlatStride, const uint32_t a_AffineStride, const uint32_t a_PerspectiveStride, const RasterFn a_RasterFn, void* a_State )
    {
        const uint32_t FlatStride = a_FlatStride / sizeof( float );
        const uint32_t AffineStride = a_AffineStride / sizeof( float );
        const uint32_t PerspectiveStride = a_PerspectiveStride / sizeof( float );
    
        // We only need to create copies of the perspective portion.
        float Data0[ 512 ]; memcpy( Data0, a_Data[ 0u ] + FlatStride + AffineStride, a_PerspectiveStride );
        float Data1[ 512 ]; memcpy( Data1, a_Data[ 1u ] + FlatStride + AffineStride, a_PerspectiveStride );
    
        for ( uint32_t i = 0u; i < PerspectiveStride; ++i )
        {
    	    Data0[ i ] *= a_P[ 0u ].w;
    	    Data1[ i ] *= a_P[ 1u ].w;
        }
    
        struct RasterState
        {
    		float Data[ 512u ];
            uint32_t FlatStride;
            uint32_t AffineStride;
            uint32_t PerspectiveStride;
            const float** D;
            const float* D0;
    		const float* D1;
            RasterFn RasterFn;
            void* State;
        } State;
    
    	State.FlatStride = FlatStride;
        State.AffineStride = AffineStride;
        State.PerspectiveStride = PerspectiveStride;
        State.D = a_Data;
    	State.D0 = Data0;
    	State.D1 = Data1;
        State.RasterFn = a_RasterFn;
        State.State = a_State;
    
        const ScanLineFn Scan = +[]( const glm::vec4& a_Position, const float a_T, void* a_InternalState )
        {
    	    RasterState& State = *( RasterState* )a_InternalState;
    
            // Copy through from provoking vertex for flat.
            memcpy( State.Data, State.D[ 0u ], State.FlatStride * sizeof( float ) );
    
            // Interpolate affine, without perspective correction.
            const uint32_t OffsetAffine = State.FlatStride;
            
            for ( uint32_t i = 0u; i < State.AffineStride; ++i )
            {
                State.Data[ i + OffsetAffine ] =
                    State.D[ 0u ][ i ] * a_T +
                    State.D[ 1u ][ i ] * ( 1.0f - a_T );
            }
    
            const float InvW = 1.0f / a_Position.w;
            const uint32_t OffsetPerspective = State.FlatStride + State.AffineStride;
    
            for ( uint32_t i = 0u; i < State.PerspectiveStride; ++i )
            {
                State.Data[ i + OffsetPerspective ] = (
                    State.D0[ i ] * a_T +
                    State.D1[ i ] * ( 1.0f - a_T ) ) * InvW;
            }
    
            State.RasterFn( a_Position, State.Data, State.FlatStride + State.AffineStride + State.PerspectiveStride, State.State );
        };

        int32_t x, y;
        int32_t dx = a_P[ 1u ].x - a_P[ 0u ].x;
        int32_t dy = a_P[ 1u ].y - a_P[ 0u ].y;
        const int32_t dx1 = abs( dx );
        const int32_t dy1 = abs( dy );
        int32_t px = 2 * dy1 - dx1;
        int32_t py = 2 * dx1 - dy1;
        int32_t xe, ye, i;
    
    	if ( dy1 <= dx1 )
    	{
    		if ( dx >= 0 )
            { 
                x = a_P[ 0u ].x; 
                y = a_P[ 0u ].y; 
                xe = a_P[ 1u ].x; 
            }
    		else
    		{ 
                x = a_P[ 1u ].x; 
                y = a_P[ 1u ].y; 
                xe = a_P[ 0u ].x; 
            }

            Scan( glm::vec4( x, y, 1.0f, 1.0f ), ( float )( dx1 - x ), &State );
    		
    		for ( i = 0; x < xe; ++i )
    		{
    			x = x + 1;
    
                if ( px < 0 )
                {
                    px = px + 2 * dy1;
                }
    			else
    			{
                    if ( ( dx < 0 && dy < 0 ) || ( dx > 0 && dy > 0 ) )
                    {
                        y = y + 1; 
                    }
                    else
                    {
                        y = y - 1;
                    }
    
    				px = px + 2 * ( dy1 - dx1 );
    			}
    
    			Scan( glm::vec4( x, y, 1.0f, 1.0f ), ( float )( dx1 - x ), &State );
    		}
    	}
    	else
    	{
    		if ( dy >= 0 )
    		{ 
                x = a_P[ 0u ].x; 
                y = a_P[ 0u ].y; 
                ye = a_P[ 1u ].y; 
            }
    		else
    		{ 
                x = a_P[ 1u ].x;
                y = a_P[ 1u ].y; 
                ye = a_P[ 0u ].y; 
            }
    
            Scan( glm::vec4( x, y, 1.0f, 1.0f ), ( float )( dx1 - x ), &State );
    
    		for ( i = 0; y < ye; ++i )
    		{
    			y = y + 1;
    
                if ( py <= 0 )
                {
                    py = py + 2 * dx1;
                }
    			else
    			{
                    if ( ( dx < 0 && dy < 0 ) || ( dx > 0 && dy > 0 ) )
                    {
                        x = x + 1;
                    }
                    else
                    {
                        x = x - 1;
                    }
    
    				py = py + 2 * ( dx1 - dy1 );
    			}

                TODO("CLean all this up and make z's valid." );
    			Scan( glm::vec4( x, y, 1.0f, 1.0f ), ( float )( dx1 - x ), &State );
    		}
    	}
    }

	// Rasterize a triangle with a raster function with flat, affine, and perspective mixed.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	// a_FlatStride is how many elements from 0th index are to be flat interpolated.
	// a_AffineStride is how many elements after a_FlatStride are to be affine interpolated.
	// a_PerspectiveStride is how many elements after a_AFfineStride are to be perspective interpolated.
	inline void RasterizeTriangle( const glm::vec4* a_P, const float** a_Data, const uint32_t a_FlatStride, const uint32_t a_AffineStride, const uint32_t a_PerspectiveStride, const RasterFn a_RasterFn, void* a_State )
    {
        const uint32_t FlatStride = a_FlatStride / sizeof( float );
        const uint32_t AffineStride = a_AffineStride / sizeof( float );
        const uint32_t PerspectiveStride = a_PerspectiveStride / sizeof( float );
    
        // We only need to create copies of the perspective portion.
        float Data0[ 512 ]; memcpy( Data0, a_Data[ 0u ] + FlatStride + AffineStride, a_PerspectiveStride );
        float Data1[ 512 ]; memcpy( Data1, a_Data[ 1u ] + FlatStride + AffineStride, a_PerspectiveStride );
        float Data2[ 512 ]; memcpy( Data2, a_Data[ 2u ] + FlatStride + AffineStride, a_PerspectiveStride );
    
        for ( uint32_t i = 0u; i < PerspectiveStride; ++i )
        {
    	    Data0[ i ] *= a_P[ 0u ].w;
    	    Data1[ i ] *= a_P[ 1u ].w;
    	    Data2[ i ] *= a_P[ 2u ].w;
        }
    
        struct RasterState
        {
    		float Data[ 512u ];
            uint32_t FlatStride;
            uint32_t AffineStride;
            uint32_t PerspectiveStride;
            const float** D;
            const float* D0;
    		const float* D1;
    		const float* D2;
            RasterFn RasterFn;
            void* State;
        } State;
    
    	State.FlatStride = FlatStride;
        State.AffineStride = AffineStride;
        State.PerspectiveStride = PerspectiveStride;
        State.D = a_Data;
    	State.D0 = Data0;
    	State.D1 = Data1;
    	State.D2 = Data2;
        State.RasterFn = a_RasterFn;
        State.State = a_State;
    
        const ScanTriangleFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
        {
    	    RasterState& State = *( RasterState* )a_InternalState;
    
            // Copy through from provoking vertex for flat.
            memcpy( State.Data, State.D[ 0u ], State.FlatStride * sizeof( float ) );
    
            // Interpolate affine, without perspective correction.
            const uint32_t OffsetAffine = State.FlatStride;
            
            for ( uint32_t i = 0u; i < State.AffineStride; ++i )
            {
                State.Data[ i + OffsetAffine ] =
                    State.D[ 0u ][ i ] * a_BarycentricCoord[ 0u ] +
                    State.D[ 1u ][ i ] * a_BarycentricCoord[ 1u ] +
                    State.D[ 2u ][ i ] * a_BarycentricCoord[ 2u ];
            }
    
            const float InvW = 1.0f / a_Position.w;
            const uint32_t OffsetPerspective = State.FlatStride + State.AffineStride;
    
            for ( uint32_t i = 0u; i < State.PerspectiveStride; ++i )
            {
                State.Data[ i + OffsetPerspective ] = (
                    State.D0[ i ] * a_BarycentricCoord[ 0u ] +
                    State.D1[ i ] * a_BarycentricCoord[ 1u ] + 
                    State.D2[ i ] * a_BarycentricCoord[ 2u ] ) * InvW;
            }
    
            State.RasterFn( a_Position, State.Data, State.FlatStride + State.AffineStride + State.PerspectiveStride, State.State );
        };

        // Using code from: https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
        // All credit goes to author.
    
    	const float BarycentricDenom = 1.0f / (
    		( a_P[ 1u ].y - a_P[ 2u ].y ) * ( a_P[ 0u ].x - a_P[ 2u ].x ) + 
    		( a_P[ 2u ].x - a_P[ 1u ].x ) * ( a_P[ 0u ].y - a_P[ 2u ].y ) );
    
        // 28.4 fixed-point coordinates
    	const int32_t Y2 = ( int32_t )( 16.0f * a_P[ 1u ].y );
    	const int32_t Y1 = ( int32_t )( 16.0f * a_P[ 0u ].y );
    	const int32_t Y3 = ( int32_t )( 16.0f * a_P[ 2u ].y );
        const int32_t X1 = ( int32_t )( 16.0f * a_P[ 0u ].x );
        const int32_t X2 = ( int32_t )( 16.0f * a_P[ 1u ].x );
        const int32_t X3 = ( int32_t )( 16.0f * a_P[ 2u ].x );
    
        // Deltas
        const int32_t DX12 = X1 - X2;
        const int32_t DX23 = X2 - X3;
        const int32_t DX31 = X3 - X1;
    
        const int32_t DY12 = Y1 - Y2;
        const int32_t DY23 = Y2 - Y3;
        const int32_t DY31 = Y3 - Y1;
    
        // Fixed-point deltas
        const int32_t FDX12 = DX12 << 4;
        const int32_t FDX23 = DX23 << 4;
        const int32_t FDX31 = DX31 << 4;
    
        const int32_t FDY12 = DY12 << 4;
        const int32_t FDY23 = DY23 << 4;
        const int32_t FDY31 = DY31 << 4;
    
        // Bounding rectangle
    	const int32_t MaxX = std::max( { X1, X2, X3 } ) + 0xF >> 4u;
    	const int32_t MinX = std::min( { X1, X2, X3 } ) + 0xF >> 4u;
    	const int32_t MinY = std::min( { Y1, Y2, Y3 } ) + 0xF >> 4u;
        const int32_t MaxY = std::max( { Y1, Y2, Y3 } ) + 0xF >> 4u;
    
        // Half-edge constants
        int32_t C1 = DY12 * X1 - DX12 * Y1;
        int32_t C2 = DY23 * X2 - DX23 * Y2;
        int32_t C3 = DY31 * X3 - DX31 * Y3;
    
        // Correct for fill convention
        if( DY12 < 0 || ( DY12 == 0 && DX12 > 0 ) ) C1++;
        if( DY23 < 0 || ( DY23 == 0 && DX23 > 0 ) ) C2++;
        if( DY31 < 0 || ( DY31 == 0 && DX31 > 0 ) ) C3++;
    
        int32_t CY1 = C1 + DX12 * ( MinY << 4 ) - DY12 * ( MinX << 4 );
        int32_t CY2 = C2 + DX23 * ( MinY << 4 ) - DY23 * ( MinX << 4 );
        int32_t CY3 = C3 + DX31 * ( MinY << 4 ) - DY31 * ( MinX << 4 );
    
        for( int32_t y = MinY; y < MaxY; ++y )
        {
            int32_t CX1 = CY1;
            int32_t CX2 = CY2;
            int32_t CX3 = CY3;
       
            for( int32_t x = MinX; x < MaxX; ++x )
            {
                if( CX1 > 0 && CX2 > 0 && CX3 > 0 )
                {
                    glm::vec3 Barycentric;
    
    				// calculate barycentric
    				Barycentric.x = ( 
    					( a_P[ 1u ].y - a_P[ 2u ].y ) * ( ( float )x - a_P[ 2u ].x ) + 
    					( a_P[ 2u ].x - a_P[ 1u ].x ) * ( ( float )y - a_P[ 2u ].y ) ) * BarycentricDenom;
    
    				Barycentric.y = ( 
    					( a_P[ 2u ].y - a_P[ 0u ].y ) * ( ( float )x - a_P[ 2u ].x ) + 
    					( a_P[ 0u ].x - a_P[ 2u ].x ) * ( ( float )y - a_P[ 2u ].y ) ) * BarycentricDenom;
    
    				Barycentric.z = 1.0f - Barycentric.x - Barycentric.y;
    
    				const glm::vec4 Frag = {
    					x, y,
    					Barycentric.x * a_P[ 0u ].z + Barycentric.y * a_P[ 1u ].z + Barycentric.z * a_P[ 2u ].z,
    					Barycentric.x * a_P[ 0u ].w + Barycentric.y * a_P[ 1u ].w + Barycentric.z * a_P[ 2u ].w
    				};
    
    				Scan( Frag, Barycentric, &State );
                }
    
                CX1 -= FDY12;
                CX2 -= FDY23;
                CX3 -= FDY31;
            }
    
            CY1 += FDX12;
            CY2 += FDX23;
            CY3 += FDX31;
        }
    }

	// Rasterize a triangle with a raster function with flat, affine, and perspective mixed.
    // Provide a bounding rect to clip the triangle to.
	// a_P is a glm::vec4[3] of the 3 corner points. Each point should be of the form (x/w, y/w, z/w, 1/w)
	// a_FlatStride is how many elements from 0th index are to be flat interpolated.
	// a_AffineStride is how many elements after a_FlatStride are to be affine interpolated.
	// a_PerspectiveStride is how many elements after a_AFfineStride are to be perspective interpolated.
    inline void RasterizeTriangleClipped( const Rect& a_Rect, const glm::vec4* a_P, const float** a_Data, const uint32_t a_FlatStride, const uint32_t a_AffineStride, const uint32_t a_PerspectiveStride, const RasterFn a_RasterFn, void* a_State )
    {
        const uint32_t FlatStride = a_FlatStride / sizeof( float );
        const uint32_t AffineStride = a_AffineStride / sizeof( float );
        const uint32_t PerspectiveStride = a_PerspectiveStride / sizeof( float );
    
        // We only need to create copies of the perspective portion.
        float Data0[ 512 ]; memcpy( Data0, a_Data[ 0u ] + FlatStride + AffineStride, a_PerspectiveStride );
        float Data1[ 512 ]; memcpy( Data1, a_Data[ 1u ] + FlatStride + AffineStride, a_PerspectiveStride );
        float Data2[ 512 ]; memcpy( Data2, a_Data[ 2u ] + FlatStride + AffineStride, a_PerspectiveStride );
    
        for ( uint32_t i = 0u; i < PerspectiveStride; ++i )
        {
    	    Data0[ i ] *= a_P[ 0u ].w;
    	    Data1[ i ] *= a_P[ 1u ].w;
    	    Data2[ i ] *= a_P[ 2u ].w;
        }
    
        struct RasterState
        {
    		float Data[ 512u ];
            uint32_t FlatStride;
            uint32_t AffineStride;
            uint32_t PerspectiveStride;
            const float** D;
            const float* D0;
    		const float* D1;
    		const float* D2;
            RasterFn RasterFn;
            void* State;
        } State;
    
    	State.FlatStride = FlatStride;
        State.AffineStride = AffineStride;
        State.PerspectiveStride = PerspectiveStride;
        State.D = a_Data;
    	State.D0 = Data0;
    	State.D1 = Data1;
    	State.D2 = Data2;
        State.RasterFn = a_RasterFn;
        State.State = a_State;
    
        const ScanTriangleFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
        {
    	    RasterState& State = *( RasterState* )a_InternalState;
    
            // Copy through from provoking vertex for flat.
            memcpy( State.Data, State.D[ 0u ], State.FlatStride * sizeof( float ) );
    
            // Interpolate affine, without perspective correction.
            const uint32_t OffsetAffine = State.FlatStride;
            
            for ( uint32_t i = 0u; i < State.AffineStride; ++i )
            {
                State.Data[ i + OffsetAffine ] =
                    State.D[ 0u ][ i ] * a_BarycentricCoord[ 0u ] +
                    State.D[ 1u ][ i ] * a_BarycentricCoord[ 1u ] +
                    State.D[ 2u ][ i ] * a_BarycentricCoord[ 2u ];
            }
    
            const float InvW = 1.0f / a_Position.w;
            const uint32_t OffsetPerspective = State.FlatStride + State.AffineStride;
    
            for ( uint32_t i = 0u; i < State.PerspectiveStride; ++i )
            {
                State.Data[ i + OffsetPerspective ] = (
                    State.D0[ i ] * a_BarycentricCoord[ 0u ] +
                    State.D1[ i ] * a_BarycentricCoord[ 1u ] + 
                    State.D2[ i ] * a_BarycentricCoord[ 2u ] ) * InvW;
            }
    
            State.RasterFn( a_Position, State.Data, State.FlatStride + State.AffineStride + State.PerspectiveStride, State.State );
        };

        // Using code from: https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
        // All credit goes to author.
    
    	const float BarycentricDenom = 1.0f / (
    		( a_P[ 1u ].y - a_P[ 2u ].y ) * ( a_P[ 0u ].x - a_P[ 2u ].x ) + 
    		( a_P[ 2u ].x - a_P[ 1u ].x ) * ( a_P[ 0u ].y - a_P[ 2u ].y ) );
    
        // 28.4 fixed-point coordinates
    	const int32_t Y2 = ( int32_t )( 16.0f * a_P[ 1u ].y );
    	const int32_t Y1 = ( int32_t )( 16.0f * a_P[ 0u ].y );
    	const int32_t Y3 = ( int32_t )( 16.0f * a_P[ 2u ].y );
        const int32_t X1 = ( int32_t )( 16.0f * a_P[ 0u ].x );
        const int32_t X2 = ( int32_t )( 16.0f * a_P[ 1u ].x );
        const int32_t X3 = ( int32_t )( 16.0f * a_P[ 2u ].x );
    
        // Deltas
        const int32_t DX12 = X1 - X2;
        const int32_t DX23 = X2 - X3;
        const int32_t DX31 = X3 - X1;
    
        const int32_t DY12 = Y1 - Y2;
        const int32_t DY23 = Y2 - Y3;
        const int32_t DY31 = Y3 - Y1;
    
        // Fixed-point deltas
        const int32_t FDX12 = DX12 << 4;
        const int32_t FDX23 = DX23 << 4;
        const int32_t FDX31 = DX31 << 4;
    
        const int32_t FDY12 = DY12 << 4;
        const int32_t FDY23 = DY23 << 4;
        const int32_t FDY31 = DY31 << 4;
    
        // Bounding rectangle
        const int32_t MaxX = a_Rect.x + a_Rect.w;
        const int32_t MinX = a_Rect.x;
        const int32_t MaxY = a_Rect.y + a_Rect.h;
        const int32_t MinY = a_Rect.y;
    
        // Half-edge constants
        int32_t C1 = DY12 * X1 - DX12 * Y1;
        int32_t C2 = DY23 * X2 - DX23 * Y2;
        int32_t C3 = DY31 * X3 - DX31 * Y3;
    
        // Correct for fill convention
        if( DY12 < 0 || ( DY12 == 0 && DX12 > 0 ) ) C1++;
        if( DY23 < 0 || ( DY23 == 0 && DX23 > 0 ) ) C2++;
        if( DY31 < 0 || ( DY31 == 0 && DX31 > 0 ) ) C3++;
    
        int32_t CY1 = C1 + DX12 * ( MinY << 4 ) - DY12 * ( MinX << 4 );
        int32_t CY2 = C2 + DX23 * ( MinY << 4 ) - DY23 * ( MinX << 4 );
        int32_t CY3 = C3 + DX31 * ( MinY << 4 ) - DY31 * ( MinX << 4 );
    
        for( int32_t y = MinY; y < MaxY; ++y )
        {
            int32_t CX1 = CY1;
            int32_t CX2 = CY2;
            int32_t CX3 = CY3;
       
            for( int32_t x = MinX; x < MaxX; ++x )
            {
                if( CX1 > 0 && CX2 > 0 && CX3 > 0 )
                {
                    glm::vec3 Barycentric;
    
    				// calculate barycentric
    				Barycentric.x = ( 
    					( a_P[ 1u ].y - a_P[ 2u ].y ) * ( ( float )x - a_P[ 2u ].x ) + 
    					( a_P[ 2u ].x - a_P[ 1u ].x ) * ( ( float )y - a_P[ 2u ].y ) ) * BarycentricDenom;
    
    				Barycentric.y = ( 
    					( a_P[ 2u ].y - a_P[ 0u ].y ) * ( ( float )x - a_P[ 2u ].x ) + 
    					( a_P[ 0u ].x - a_P[ 2u ].x ) * ( ( float )y - a_P[ 2u ].y ) ) * BarycentricDenom;
    
    				Barycentric.z = 1.0f - Barycentric.x - Barycentric.y;
    
    				const glm::vec4 Frag = {
    					x, y,
    					Barycentric.x * a_P[ 0u ].z + Barycentric.y * a_P[ 1u ].z + Barycentric.z * a_P[ 2u ].z,
    					Barycentric.x * a_P[ 0u ].w + Barycentric.y * a_P[ 1u ].w + Barycentric.z * a_P[ 2u ].w
    				};
    
    				Scan( Frag, Barycentric, &State );
                }
    
                CX1 -= FDY12;
                CX2 -= FDY23;
                CX3 -= FDY31;
            }
    
            CY1 += FDX12;
            CY2 += FDX23;
            CY3 += FDX31;
        }
    }

    // a_P is a glm::vec4[4]. The first three should be set to the triangle positions, and these will be changed.
    // a Fourth point can be generated and placed in the last index. Will return number of points.
    // a_Plane represents the clipping plane.
    // a_T is the parameters along .
    inline uint32_t ClipTriangle( const glm::vec4* a_P, const glm::vec4& a_Plane, glm::vec2& o_T )
	{
		return 0u;
	}
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

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >::DrawBuffer( const uint32_t a_Width, const uint32_t a_Height )
    : m_Elements( a_Width * a_Height > 0u ? new ElementType[ a_Width * a_Height ] : nullptr )
    , m_Width( a_Width )
    , m_Height( a_Height )
{}

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >::DrawBuffer( const DrawBuffer& a_DrawBuffer )
    : m_Elements( a_DrawBuffer.m_Elements ? new ElementType[ a_DrawBuffer.m_Width * a_DrawBuffer.m_Height ] : nullptr )
    , m_Width( a_DrawBuffer.m_Width )
    , m_Height( a_DrawBuffer.m_Height )
{
    if ( m_Elements )
    {
        ( void )memcpy( m_Elements, a_DrawBuffer.m_Elements, sizeof( ElementType ) * a_DrawBuffer.m_Width * a_DrawBuffer.m_Height );
    }
}

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >::DrawBuffer( DrawBuffer&& a_DrawBuffer ) noexcept
    : m_Elements( a_DrawBuffer.m_Elements )
    , m_Width( a_DrawBuffer.m_Width )
    , m_Height( a_DrawBuffer.m_Height )
{
    a_DrawBuffer.m_Elements = nullptr;
    a_DrawBuffer.m_Width = 0u;
    a_DrawBuffer.m_Height = 0u;
}

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >& ConsoleGL::DrawBuffer< _Element >::operator=( const DrawBuffer& a_DrawBuffer )
{
    m_Elements = a_DrawBuffer.m_Elements ? new ElementType[ a_DrawBuffer.m_Width * a_DrawBuffer.m_Height ] : nullptr;
    m_Width = a_DrawBuffer.m_Width;
    m_Height = a_DrawBuffer.m_Height;

    if ( m_Elements )
    {
        ( void )memcpy( m_Elements, a_DrawBuffer.m_Elements, sizeof( ElementType ) * a_DrawBuffer.m_Width * a_DrawBuffer.m_Height );
    }

    return *this;
}

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >& ConsoleGL::DrawBuffer< _Element >::operator=( DrawBuffer&& a_DrawBuffer ) noexcept
{
    m_Elements = a_DrawBuffer.m_Elements;
    m_Width = a_DrawBuffer.m_Width;
    m_Height = a_DrawBuffer.m_Height;
    a_DrawBuffer.m_Elements = nullptr;
    a_DrawBuffer.m_Width = 0u;
    a_DrawBuffer.m_Height = 0u;
    return *this;
}

template < typename _Element >
ConsoleGL::DrawBuffer< _Element >::~DrawBuffer()
{
    if ( m_Elements )
    {
        delete m_Elements;
        m_Elements = nullptr;
        m_Width = 0u;
        m_Height = 0u;
    }
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::SetBuffer( const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    ElementType* Buffer = m_Elements;
    for ( uint32_t y = 0u; y < m_Height; ++y )
        for ( uint32_t x = 0u; x < m_Width; ++x, ++Buffer )
            *Buffer = a_FragmentFn( { x, y }, a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawLine( const Seg& a_Seg, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	int32_t x, y;
    int32_t dx = a_Seg.p1.x - a_Seg.p0.x;
    int32_t dy = a_Seg.p1.y - a_Seg.p0.y;
    int32_t dx1 = abs( dx );
    int32_t dy1 = abs( dy );
    int32_t px = 2 * dy1 - dx1;
    int32_t py = 2 * dx1 - dy1;
    int32_t xe, ye, i;

	if ( dy1 <= dx1 )
	{
		if ( dx >= 0 )
        { 
            x = a_Seg.p0.x; 
            y = a_Seg.p0.y; 
            xe = a_Seg.p1.x; 
        }
		else
		{ 
            x = a_Seg.p1.x; 
            y = a_Seg.p1.y; 
            xe = a_Seg.p0.x; 
        }

        SetElement( Coord( x, y ), a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );
		
		for ( i = 0; x < xe; ++i )
		{
			x = x + 1;

            if ( px < 0 )
            {
                px = px + 2 * dy1;
            }
			else
			{
                if ( ( dx < 0 && dy < 0 ) || ( dx > 0 && dy > 0 ) )
                {
                    y = y + 1; 
                }
                else
                {
                    y = y - 1;
                }

				px = px + 2 * ( dy1 - dx1 );
			}

            SetElement( Coord( x, y ), a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );
		}
	}
	else
	{
		if ( dy >= 0 )
		{ 
            x = a_Seg.p0.x; 
            y = a_Seg.p0.y; 
            ye = a_Seg.p1.y; 
        }
		else
		{ 
            x = a_Seg.p1.x;
            y = a_Seg.p1.y; 
            ye = a_Seg.p0.y; 
        }

        SetElement( Coord( x, y ), a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );

		for ( i = 0; y < ye; ++i )
		{
			y = y + 1;

            if ( py <= 0 )
            {
                py = py + 2 * dx1;
            }
			else
			{
                if ( ( dx < 0 && dy < 0 ) || ( dx > 0 && dy > 0 ) )
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }

				py = py + 2 * ( dx1 - dy1 );
			}
			
            SetElement( Coord( x, y ), a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );
		}
	}
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawHorizontalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t YMin = a_Begin.y;
    const uint32_t YMax = a_Begin.y + a_Length;

    for ( uint32_t y = YMin; y < YMax; ++y )
    {
        a_FragmentFn( { a_Begin.x, y }, a_FragmentFnPayload );
    }
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawVerticalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t XMin = a_Begin.x;
    const uint32_t XMax = a_Begin.x + a_Length;

    for ( uint32_t x = XMin; x < XMax; ++x )
    {
        a_FragmentFn( { x, a_Begin.y }, a_FragmentFnPayload );
    }
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawTriangle( const Tri& a_Tri, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    DrawLine( { a_Tri.p0, a_Tri.p1 }, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( { a_Tri.p1, a_Tri.p2 }, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( { a_Tri.p2, a_Tri.p0 }, a_FragmentFn, a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawTriangleFilled( const Tri& a_Tri, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    // Using code from: https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
    // All credit goes to author.

    // 28.4 fixed-point coordinates
    const int Y1 = (int)(16.0f * a_Tri.p0.y);
    const int Y2 = (int)(16.0f * a_Tri.p1.y);
    const int Y3 = (int)(16.0f * a_Tri.p2.y);
    const int X1 = (int)(16.0f * a_Tri.p0.x);
    const int X2 = (int)(16.0f * a_Tri.p1.x);
    const int X3 = (int)(16.0f * a_Tri.p2.x);

    // Deltas
    const int DX12 = X1 - X2;
    const int DX23 = X2 - X3;
    const int DX31 = X3 - X1;

    const int DY12 = Y1 - Y2;
    const int DY23 = Y2 - Y3;
    const int DY31 = Y3 - Y1;

    // Fixed-point deltas
    const int FDX12 = DX12 << 4;
    const int FDX23 = DX23 << 4;
    const int FDX31 = DX31 << 4;

    const int FDY12 = DY12 << 4;
    const int FDY23 = DY23 << 4;
    const int FDY31 = DY31 << 4;

    // Bounding rectangle
    int minx = (std::min({X1, X2, X3}) + 0xF) >> 4;
    int maxx = (std::max({X1, X2, X3}) + 0xF) >> 4;
    int miny = (std::min({Y1, Y2, Y3}) + 0xF) >> 4;
    int maxy = (std::max({Y1, Y2, Y3}) + 0xF) >> 4;

    //(char*&)colorBuffer += miny * stride;
    ElementType* Buffer = m_Elements + miny * m_Width;

    // Half-edge constants
    int C1 = DY12 * X1 - DX12 * Y1;
    int C2 = DY23 * X2 - DX23 * Y2;
    int C3 = DY31 * X3 - DX31 * Y3;

    // Correct for fill convention
    if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
    if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
    if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

    int CY1 = C1 + DX12 * (miny << 4) - DY12 * (minx << 4);
    int CY2 = C2 + DX23 * (miny << 4) - DY23 * (minx << 4);
    int CY3 = C3 + DX31 * (miny << 4) - DY31 * (minx << 4);

    for(int y = miny; y < maxy; y++)
    {
        int CX1 = CY1;
        int CX2 = CY2;
        int CX3 = CY3;
   
        for(int x = minx; x < maxx; x++)
        {
            if(CX1 > 0 && CX2 > 0 && CX3 > 0)
            {
                Buffer[x] = a_FragmentFn( Coord( x, y ), a_FragmentFnPayload );
            }

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
        }

        CY1 += FDX12;
        CY2 += FDX23;
        CY3 += FDX31;

        //(char*&)colorBuffer += stride;
        Buffer += m_Width;
    }

}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawRect( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    // If 0 thickness, nothing to draw.
    if ( !a_Rect.w || !a_Rect.h )
    {
	    return;
    }

    // If 1 thickness in x direction, draw horizontal line.
    if ( a_Rect.w == 1u )
        return DrawVerticalLine( a_Rect.Origin, a_Rect.h, a_FragmentFn, a_FragmentFnPayload );

    if ( a_Rect.h == 1u )
        return DrawHorizontalLine( a_Rect.Origin, a_Rect.w, a_FragmentFn, a_FragmentFnPayload );

	Pixel* Top = m_Elements + a_Rect.y * m_Width;
	Pixel* Bot = Top + ( a_Rect.h - 1u ) * m_Width;

    const uint32_t YTop = a_Rect.y;
    const uint32_t YBottom = a_Rect.y + a_Rect.h - 1u;
    const uint32_t XLeft = a_Rect.x;
    const uint32_t XRight = a_Rect.x + a_Rect.w - 1u;

    // Draw top and bottom line.
    for ( uint32_t x = XLeft; x <= XRight; ++x )
    {
        Top[ x ] = a_FragmentFn( { x, YTop }, a_FragmentFnPayload );
        Bot[ x ] = a_FragmentFn( { x, YBottom }, a_FragmentFnPayload );
    }

    ElementType* Buffer = Top += m_Width;

    // Draw left and right line.
    for ( uint32_t y = YTop + 1u; y < YBottom; ++y, Buffer += m_Width )
    {
        Buffer[ XLeft ] = a_FragmentFn( { XLeft, y }, a_FragmentFnPayload );
        Buffer[ XRight ] = a_FragmentFn( { XRight, y }, a_FragmentFnPayload );
    }
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawRect( const Rect& a_Rect, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const int32_t HalfX = a_Rect.w >> 1;
    const int32_t HalfY = a_Rect.h >> 1;

	const float Cos = cos( a_Radians );
	const float Sin = sin( a_Radians );

    const glm::vec2 Offset0 = { -HalfX, -HalfY };
    const glm::vec2 Offset1 = { -HalfX, -HalfY + a_Rect.h };

    glm::vec2 Rotated0 = { Offset0.x * Cos - Offset0.y * Sin, Offset0.x * Sin + Offset0.y * Cos };
    glm::vec2 Rotated1 = { Offset1.x * Cos - Offset1.y * Sin, Offset1.x * Sin + Offset1.y * Cos };
    glm::vec2 Rotated2 = { -Rotated0.x, -Rotated0.y };
    glm::vec2 Rotated3 = { -Rotated1.x, -Rotated1.y };

    const int32_t OffsetX = HalfX + a_Rect.x;
    const int32_t OffsetY = HalfY + a_Rect.y;

    Rotated0 += glm::vec2{ OffsetX, OffsetY };
    Rotated1 += glm::vec2{ OffsetX, OffsetY };
    Rotated2 += glm::vec2{ OffsetX, OffsetY };
    Rotated3 += glm::vec2{ OffsetX, OffsetY };

    DrawLine( { Coord{ ( uint32_t )Rotated0.x, ( uint32_t )Rotated0.y }, Coord{ ( uint32_t )Rotated1.x, ( uint32_t )Rotated1.y } }, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( { Coord{ ( uint32_t )Rotated1.x, ( uint32_t )Rotated1.y }, Coord{ ( uint32_t )Rotated2.x, ( uint32_t )Rotated2.y } }, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( { Coord{ ( uint32_t )Rotated2.x, ( uint32_t )Rotated2.y }, Coord{ ( uint32_t )Rotated3.x, ( uint32_t )Rotated3.y } }, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( { Coord{ ( uint32_t )Rotated3.x, ( uint32_t )Rotated3.y }, Coord{ ( uint32_t )Rotated0.x, ( uint32_t )Rotated0.y } }, a_FragmentFn, a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawRectFilled( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t YTop = a_Rect.y;
    const uint32_t YBottom = a_Rect.y + a_Rect.h - 1u;
    const uint32_t XLeft = a_Rect.x;
    const uint32_t XRight = a_Rect.x + a_Rect.w - 1u;

	Pixel* Buffer = m_Elements + a_Rect.y * m_Width;

    for ( uint32_t y = YTop; y <= YBottom; ++y, Buffer += m_Width )
		for ( uint32_t x = XLeft; x <= XRight; ++x )
            Buffer[ x ] = a_FragmentFn( { x, y }, a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawRectFilled( const Rect& a_Rect, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	const int32_t HalfX = a_Rect.w >> 1;
    const int32_t HalfY = a_Rect.h >> 1;

	const float Cos = cos( a_Radians );
	const float Sin = sin( a_Radians );

    const glm::vec2 Offset0 = { -HalfX, -HalfY };
    const glm::vec2 Offset1 = { -HalfX, -HalfY + a_Rect.h };

    glm::vec2 Rotated0 = { Offset0.x * Cos - Offset0.y * Sin, Offset0.x * Sin + Offset0.y * Cos };
    glm::vec2 Rotated1 = { Offset1.x * Cos - Offset1.y * Sin, Offset1.x * Sin + Offset1.y * Cos };
    glm::vec2 Rotated2 = { -Rotated0.x, -Rotated0.y };
    glm::vec2 Rotated3 = { -Rotated1.x, -Rotated1.y };

    const int32_t OffsetX = HalfX + a_Rect.x;
    const int32_t OffsetY = HalfY + a_Rect.y;

    Rotated0 += glm::vec2{ OffsetX, OffsetY };
    Rotated1 += glm::vec2{ OffsetX, OffsetY };
    Rotated2 += glm::vec2{ OffsetX, OffsetY };
    Rotated3 += glm::vec2{ OffsetX, OffsetY };

    // 0 ------ 3
    // |        |
    // |        |
    // 1 ------ 2

    DrawTriangleFilled( { Coord{ ( uint32_t )Rotated0.x, ( uint32_t )Rotated0.y }, Coord{ ( uint32_t )Rotated1.x, ( uint32_t )Rotated1.y }, Coord{ ( uint32_t )Rotated2.x, ( uint32_t )Rotated2.y } }, a_FragmentFn, a_FragmentFnPayload );
    DrawTriangleFilled( { Coord{ ( uint32_t )Rotated0.x, ( uint32_t )Rotated0.y }, Coord{ ( uint32_t )Rotated2.x, ( uint32_t )Rotated2.y }, Coord{ ( uint32_t )Rotated3.x, ( uint32_t )Rotated3.y } }, a_FragmentFn, a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawCircle( const Coord a_Centre, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO("This needs work.");
    auto setPixel = [ & ]( int x, int y )
    {
        SetElement( Coord{ ( uint32_t )x, ( uint32_t )y }, a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );
    };

    auto drawCircle = [ & ](int xc, int yc, int x, int y)
    {
        setPixel( xc+x, yc+y );
        setPixel( xc-x, yc+y );
        setPixel( xc+x, yc-y );
        setPixel( xc-x, yc-y );
        setPixel( xc+y, yc+x );
        setPixel( xc-y, yc+x );
        setPixel( xc+y, yc-x );
        setPixel( xc-y, yc-x );
    };

    int x = 0, y = a_Radius;
    int d = 3 - 2 * a_Radius;
    drawCircle( a_Centre.x, a_Centre.y, x, y);
    while (y >= x)
    {
        // for each pixel we will
        // draw all eight pixels
        
        x++;

        // check for decision parameter
        // and correspondingly 
        // update d, x, y
        if (d > 0)
        {
            y--; 
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        drawCircle(a_Centre.x, a_Centre.y, x, y);
    }
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawCircleFilled( const Coord a_Centre, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO( "this can be optimised to only calculate a quarter and flip coordinates for other 3 quadrants.");
	// Scan over bounding box.
    const int32_t MinX = a_Centre.x - a_Radius;
    const int32_t MaxX = a_Centre.x + a_Radius;
    const int32_t MinY = a_Centre.y - a_Radius;
    const int32_t MaxY = a_Centre.y + a_Radius;
    const int32_t CentreX = a_Centre.x;
    const int32_t CentreY = a_Centre.y;

    ElementType* Buffer = m_Elements + MinY * m_Width;
    TODO( "Is the failure to use <= here failing the calcs?" );
    for ( int32_t y = MinY; y < MaxY; ++y, Buffer += m_Width )
        for ( int32_t x = MinX; x < MaxX; ++x )
            if ( ( x - CentreX ) * ( x - CentreX ) + ( y - CentreY ) * ( y - CentreY ) < ( int32_t )( a_Radius * a_Radius ) )
                // If pixel inside radius, draw.
                Buffer[ x ] = a_FragmentFn( Coord( x, y ), a_FragmentFnPayload );
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawEllipse( const Coord a_Centre, const Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
#pragma warning( push )
#pragma warning( disable : 4244)

	TODO("This needs work.");
    auto setPixel = [ & ]( int x, int y )
    {
	    SetElement(Coord( x, y ), a_FragmentFn( Coord( x, y ), a_FragmentFnPayload ) );
    };

	[&](int rx, int ry, int xc, int yc) -> void
    {
        float dx, dy, d1, d2, x, y;
        x = 0;
        y = ry;
     
        // Initial decision parameter of region 1
        d1 = (ry * ry) - (rx * rx * ry) + 
                         (0.25 * rx * rx);
        dx = 2 * ry * ry * x;
        dy = 2 * rx * rx * y;
     
        // For region 1
        while (dx < dy) 
        {
            // Print points based on 4-way symmetry
            //cout << x + xc << " , " << y + yc << endl;
            //cout << -x + xc << " , " << y + yc << endl;
            //cout << x + xc << " , " << -y + yc << endl;
            //cout << -x + xc << " , " << -y + yc << endl;
            setPixel( x+xc, y+yc );
            setPixel( -x+xc, y+yc );
            setPixel( x+xc, -y+yc );
            setPixel( -x+xc, -y+yc );

            // Checking and updating value of
            // decision parameter based on algorithm
            if (d1 < 0)
            {
                x++;
                dx = dx + (2 * ry * ry);
                d1 = d1 + dx + (ry * ry);
            }
            else
            {
                x++;
                y--;
                dx = dx + (2 * ry * ry);
                dy = dy - (2 * rx * rx);
                d1 = d1 + dx - dy + (ry * ry);
            }
        }
     
        // Decision parameter of region 2
        d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) + 
             ((rx * rx) * ((y - 1) * (y - 1))) -
              (rx * rx * ry * ry);
     
        // Plotting points of region 2
        while (y >= 0)
        {
            setPixel( x+xc, y+yc );
            setPixel( -x+xc, y+yc );
            setPixel( x+xc, -y+yc );
            setPixel( -x+xc, -y+yc );

            // Checking and updating parameter
            // value based on algorithm
            if (d2 > 0) 
            {
                y--;
                dy = dy - (2 * rx * rx);
                d2 = d2 + (rx * rx) - dy;
            }
            else
            {
                y--;
                x++;
                dx = dx + (2 * ry * ry);
                dy = dy - (2 * rx * rx);
                d2 = d2 + dx - dy + (rx * rx);
            }
        }
    }( a_Radius.x, a_Radius.y, a_Centre.x, a_Centre.y );

#pragma warning( pop ) 
}

template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawEllipseFilled( const Coord a_Centre, const Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO( "this can be optimised to only calculate a quarter and flip coordinates for other 3 quadrants.");
	// Scan over bounding box.
    const int32_t MinX = a_Centre.x - a_Radius.x;
    const int32_t MaxX = a_Centre.x + a_Radius.x;
    const int32_t MinY = a_Centre.y - a_Radius.y;
    const int32_t MaxY = a_Centre.y + a_Radius.y;
    const int32_t LDen = a_Radius.x * a_Radius.x;
    const int32_t RDen = a_Radius.y * a_Radius.y;

    ElementType* Buffer = m_Elements + MinY * m_Width;

    for ( int32_t y = MinY; y < MaxY; ++y, Buffer += m_Width )
        for ( int32_t x = MinX; x < MaxX; ++x )
        {
            const int32_t LNum = ( x - a_Centre.x ) * ( x - a_Centre.x );
            const int32_t RNum = ( y - a_Centre.y ) * ( y - a_Centre.y );

	        if ( RDen * LNum + LDen * RNum < LDen * RDen )
                // If pixel inside radius, draw.
                Buffer[ x ] = a_FragmentFn( Coord( x, y ), a_FragmentFnPayload );
        }
}


template < typename _Element >
void ConsoleGL::DrawBuffer< _Element >::DrawOnto( DrawBuffer* a_Buffer, const Coord a_Origin )
{
	Pixel* Destination = a_Buffer->m_Elements + a_Origin.y * a_Buffer->m_Width + a_Origin.x;
    ElementType* Source = m_Elements;

    for ( uint32_t y = 0; y < m_Height; ++y, Destination += a_Buffer->m_Width, Source += m_Width )
    {
	    memcpy( Destination, Source, sizeof( ElementType ) * m_Width );
    }
}