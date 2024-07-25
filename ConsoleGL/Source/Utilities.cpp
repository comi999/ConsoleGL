#include <Utilities.hpp>

void ConsoleGL::ScanTriangle( const glm::vec4* a_P, const ScanFn a_ScanFn, void* a_State )
{
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

				a_ScanFn( Frag, Barycentric, a_State );
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

void ConsoleGL::RasterizeFlat( const glm::vec4* a_P, const float** a_Data, const uint32_t a_Stride, const RasterFn a_RasterFn, void* a_State )
{
    struct RasterState
    {
        uint32_t Stride;
        const float* D0;
        RasterFn RasterFn;
        void* State;
    } State;

	State.Stride = a_Stride;
	State.D0 = a_Data[ 0u ];
    State.RasterFn = a_RasterFn;
    State.State = a_State;

    const ScanFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
    {
	    const RasterState& State = *( RasterState* )a_InternalState;
        State.RasterFn( a_Position, State.D0, State.Stride, State.State );
    };

    ScanTriangle( a_P, Scan, &State );
}

void ConsoleGL::RasterizeAffine( const glm::vec4* a_P, const float** a_Data, const uint32_t a_Stride, const RasterFn a_RasterFn, void* a_State )
{
    struct RasterState
    {
		float Data[ 512u ];
        uint32_t Stride;
        const float* D0;
		const float* D1;
		const float* D2;
        RasterFn RasterFn;
        void* State;
    } State;

	State.Stride = a_Stride;
	State.D0 = a_Data[ 0u ];
	State.D1 = a_Data[ 1u ];
	State.D2 = a_Data[ 2u ];
    State.RasterFn = a_RasterFn;
    State.State = a_State;

    const ScanFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
    {
	    RasterState& State = *( RasterState* )a_InternalState;

        for ( uint32_t i = 0u; i < State.Stride; ++i )
        {
            State.Data[ i ] = 
                State.D0[ i ] * a_BarycentricCoord[ 0u ] +
                State.D1[ i ] * a_BarycentricCoord[ 1u ] + 
                State.D2[ i ] * a_BarycentricCoord[ 2u ];
        }
        
    	State.RasterFn( a_Position, State.Data, State.Stride, State.State );
    };

    ScanTriangle( a_P, Scan, &State );
}

void ConsoleGL::RasterizePerspective( const glm::vec4* a_P, const float** a_Data, const uint32_t a_Stride, const RasterFn a_RasterFn, void* a_State )
{
    float Data0[ 512 ]; memcpy( Data0, a_Data[ 0u ], a_Stride * sizeof( float ) );
    float Data1[ 512 ]; memcpy( Data1, a_Data[ 1u ], a_Stride * sizeof( float ) );
    float Data2[ 512 ]; memcpy( Data2, a_Data[ 2u ], a_Stride * sizeof( float ) );

    for ( uint32_t i = 0u; i < a_Stride; ++i )
    {
	    Data0[ i ] *= a_P[ 0u ].w;
	    Data1[ i ] *= a_P[ 1u ].w;
	    Data2[ i ] *= a_P[ 2u ].w;
    }

    struct RasterState
    {
		float Data[ 512u ];
        uint32_t Stride;
        const float* D0;
		const float* D1;
		const float* D2;
        RasterFn RasterFn;
        void* State;
    } State;

	State.Stride = a_Stride;
	State.D0 = Data0;
	State.D1 = Data1;
	State.D2 = Data2;
    State.RasterFn = a_RasterFn;
    State.State = a_State;

    const ScanFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
    {
	    RasterState& State = *( RasterState* )a_InternalState;
        const float InvW = 1.0f / a_Position.w;

        for ( uint32_t i = 0u; i < State.Stride; ++i )
        {
            State.Data[ i ] = (
                State.D0[ i ] * a_BarycentricCoord[ 0u ] +
                State.D1[ i ] * a_BarycentricCoord[ 1u ] + 
                State.D2[ i ] * a_BarycentricCoord[ 2u ] ) * InvW;
        }
        
    	State.RasterFn( a_Position, State.Data, State.Stride, State.State );
    };

    ScanTriangle( a_P, Scan, &State );
}

void ConsoleGL::Rasterize( const glm::vec4* a_P, const float** a_Data, const uint32_t a_FlatStride, const uint32_t a_AffineStride, const uint32_t a_PerspectiveStride, const RasterFn a_RasterFn, void* a_State )
{
    // We only need to create copies of the perspective portion.
    float Data0[ 512 ]; memcpy( Data0, a_Data[ 0u ] + a_FlatStride + a_AffineStride, a_PerspectiveStride * sizeof( float ) );
    float Data1[ 512 ]; memcpy( Data1, a_Data[ 1u ] + a_FlatStride + a_AffineStride, a_PerspectiveStride * sizeof( float ) );
    float Data2[ 512 ]; memcpy( Data2, a_Data[ 2u ] + a_FlatStride + a_AffineStride, a_PerspectiveStride * sizeof( float ) );

    for ( uint32_t i = 0u; i < a_PerspectiveStride; ++i )
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

	State.FlatStride = a_FlatStride;
    State.AffineStride = a_AffineStride;
    State.PerspectiveStride = a_PerspectiveStride;
    State.D = a_Data;
	State.D0 = Data0;
	State.D1 = Data1;
	State.D2 = Data2;
    State.RasterFn = a_RasterFn;
    State.State = a_State;

    const ScanFn Scan = +[]( const glm::vec4& a_Position, const glm::vec3& a_BarycentricCoord, void* a_InternalState )
    {
	    RasterState& State = *( RasterState* )a_InternalState;

        // Copy through from provoking vertex for flat.
        memcpy( State.Data, State.D[ 0u ], State.FlatStride * sizeof( float ) );

        // Interpolate affine, without perspective correction.
        for ( uint32_t i = State.FlatStride; i < State.FlatStride + State.AffineStride; ++i )
        {
            State.Data[ i ] =
                State.D[ 0u ][ i ] * a_BarycentricCoord[ 0u ] +
                State.D[ 1u ][ i ] * a_BarycentricCoord[ 1u ] +
                State.D[ 2u ][ i ] * a_BarycentricCoord[ 2u ];
        }

        const float InvW = 1.0f / a_Position.w;

        for ( uint32_t i = State.FlatStride + State.AffineStride; i < State.FlatStride + State.AffineStride + State.PerspectiveStride; ++i )
        {
            State.Data[ i ] = (
                State.D0[ i ] * a_BarycentricCoord[ 0u ] +
                State.D1[ i ] * a_BarycentricCoord[ 1u ] + 
                State.D2[ i ] * a_BarycentricCoord[ 2u ] ) * InvW;
        }

        State.RasterFn( a_Position, State.Data, State.FlatStride + State.AffineStride + State.PerspectiveStride, State.State );
    };

    ScanTriangle( a_P, Scan, &State );
}