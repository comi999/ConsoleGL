#include <memory>
#include <algorithm>

#include <glm/gtx/rotate_vector.hpp>
#include <PixelBuffer.hpp>

ConsoleGL::PixelBuffer::PixelBuffer( const uint32_t a_Width, const uint32_t a_Height )
    : m_Pixels( a_Width * a_Height > 0u ? new Pixel[ a_Width * a_Height ] : nullptr )
    , m_Width( a_Width )
    , m_Height( a_Height )
{}

ConsoleGL::PixelBuffer::PixelBuffer( const PixelBuffer& a_PixelBuffer )
    : m_Pixels( a_PixelBuffer.m_Pixels ? new Pixel[ a_PixelBuffer.m_Width * a_PixelBuffer.m_Height ] : nullptr )
    , m_Width( a_PixelBuffer.m_Width )
    , m_Height( a_PixelBuffer.m_Height )
{
    if ( m_Pixels )
    {
        ( void )memcpy( m_Pixels, a_PixelBuffer.m_Pixels, sizeof( Pixel ) * a_PixelBuffer.m_Width * a_PixelBuffer.m_Height );
    }
}

ConsoleGL::PixelBuffer::PixelBuffer( PixelBuffer&& a_PixelBuffer ) noexcept
    : m_Pixels( a_PixelBuffer.m_Pixels )
    , m_Width( a_PixelBuffer.m_Width )
    , m_Height( a_PixelBuffer.m_Height )
{
    a_PixelBuffer.m_Pixels = nullptr;
    a_PixelBuffer.m_Width = 0u;
    a_PixelBuffer.m_Height = 0u;
}

ConsoleGL::PixelBuffer& ConsoleGL::PixelBuffer::operator=( const PixelBuffer& a_PixelBuffer )
{
    m_Pixels = a_PixelBuffer.m_Pixels ? new Pixel[ a_PixelBuffer.m_Width * a_PixelBuffer.m_Height ] : nullptr;
    m_Width = a_PixelBuffer.m_Width;
    m_Height = a_PixelBuffer.m_Height;

    if ( m_Pixels )
    {
        ( void )memcpy( m_Pixels, a_PixelBuffer.m_Pixels, sizeof( Pixel ) * a_PixelBuffer.m_Width * a_PixelBuffer.m_Height );
    }

    return *this;
}

ConsoleGL::PixelBuffer& ConsoleGL::PixelBuffer::operator=( PixelBuffer&& a_PixelBuffer ) noexcept
{
    m_Pixels = a_PixelBuffer.m_Pixels;
    m_Width = a_PixelBuffer.m_Width;
    m_Height = a_PixelBuffer.m_Height;
    a_PixelBuffer.m_Pixels = nullptr;
    a_PixelBuffer.m_Width = 0u;
    a_PixelBuffer.m_Height = 0u;
    return *this;
}

ConsoleGL::PixelBuffer::~PixelBuffer()
{
    if ( m_Pixels )
    {
        delete m_Pixels;
        m_Pixels = nullptr;
        m_Width = 0u;
        m_Height = 0u;
    }
}

void ConsoleGL::PixelBuffer::SetBuffer( const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    Pixel* Buffer = m_Pixels;
    for ( uint32_t y = 0u; y < m_Height; ++y )
        for ( uint32_t x = 0u; x < m_Width; ++x, ++Buffer )
           *Buffer = a_FragmentFn( x, y, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawLine( const Coord a_Begin, const Coord a_End, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	int32_t x, y;
    int32_t dx = a_End.x - a_Begin.x;
    int32_t dy = a_End.y - a_Begin.y;
    int32_t dx1 = abs( dx );
    int32_t dy1 = abs( dy );
    int32_t px = 2 * dy1 - dx1;
    int32_t py = 2 * dx1 - dy1;
    int32_t xe, ye, i;

	if ( dy1 <= dx1 )
	{
		if ( dx >= 0 )
        { 
            x = a_Begin.x; 
            y = a_Begin.y; 
            xe = a_End.x; 
        }
		else
		{ 
            x = a_End.x; 
            y = a_End.y; 
            xe = a_Begin.x; 
        }

        SetPixel( Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );
		
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

            SetPixel( Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );
		}
	}
	else
	{
		if ( dy >= 0 )
		{ 
            x = a_Begin.x; 
            y = a_Begin.y; 
            ye = a_End.y; 
        }
		else
		{ 
            x = a_End.x;
            y = a_End.y; 
            ye = a_Begin.y; 
        }

        SetPixel( Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );

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
			
            SetPixel( Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );
		}
	}
}

void ConsoleGL::PixelBuffer::DrawHorizontalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t YMin = a_Begin.y;
    const uint32_t YMax = a_Begin.y + a_Length;

    for ( uint32_t y = YMin; y < YMax; ++y )
    {
        a_FragmentFn( a_Begin.x, y, a_FragmentFnPayload );
    }
}

void ConsoleGL::PixelBuffer::DrawVerticalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t XMin = a_Begin.x;
    const uint32_t XMax = a_Begin.x + a_Length;

    for ( uint32_t x = XMin; x < XMax; ++x )
    {
        a_FragmentFn( x, a_Begin.y, a_FragmentFnPayload );
    }
}

void ConsoleGL::PixelBuffer::DrawTriangle( const Coord a_P0, const Coord a_P1, const Coord a_P2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    DrawLine( a_P0, a_P1, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( a_P1, a_P2, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( a_P2, a_P0, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawTriangleFilled( const Coord a_P0, const Coord a_P1, const Coord a_P2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    // Using code from: https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
    // All credit goes to author.

    // 28.4 fixed-point coordinates
    const int Y1 = (int)(16.0f * a_P0.y);
    const int Y2 = (int)(16.0f * a_P1.y);
    const int Y3 = (int)(16.0f * a_P2.y);
    const int X1 = (int)(16.0f * a_P0.x);
    const int X2 = (int)(16.0f * a_P1.x);
    const int X3 = (int)(16.0f * a_P2.x);

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
    Pixel* Buffer = m_Pixels + miny * m_Width;

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
                Buffer[x] = a_FragmentFn( x, y, a_FragmentFnPayload );
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

void ConsoleGL::PixelBuffer::DrawRect( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
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

	Pixel* Top = m_Pixels + a_Rect.y * m_Width;
	Pixel* Bot = Top + ( a_Rect.h - 1u ) * m_Width;

    const uint32_t YTop = a_Rect.y;
    const uint32_t YBottom = a_Rect.y + a_Rect.h - 1u;
    const uint32_t XLeft = a_Rect.x;
    const uint32_t XRight = a_Rect.x + a_Rect.w - 1u;

    // Draw top and bottom line.
    for ( uint32_t x = XLeft; x <= XRight; ++x )
    {
	    Top[ x ] = a_FragmentFn( x, YTop, a_FragmentFnPayload );
	    Bot[ x ] = a_FragmentFn( x, YBottom, a_FragmentFnPayload );
    }

    Pixel* Buffer = Top += m_Width;

    // Draw left and right line.
    for ( uint32_t y = YTop + 1u; y < YBottom; ++y, Buffer += m_Width )
    {
	    Buffer[ XLeft ] = a_FragmentFn( XLeft, y, a_FragmentFnPayload );
	    Buffer[ XRight ] = a_FragmentFn( XRight, y, a_FragmentFnPayload );
    }
}

void ConsoleGL::PixelBuffer::DrawRect( const Rect& a_Rect, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
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

    DrawLine( Coord( Rotated0.x, Rotated0.y ), Coord( Rotated1.x, Rotated1.y ), a_FragmentFn, a_FragmentFnPayload );
    DrawLine( Coord( Rotated1.x, Rotated1.y ), Coord( Rotated2.x, Rotated2.y ), a_FragmentFn, a_FragmentFnPayload );
    DrawLine( Coord( Rotated2.x, Rotated2.y ), Coord( Rotated3.x, Rotated3.y ), a_FragmentFn, a_FragmentFnPayload );
    DrawLine( Coord( Rotated3.x, Rotated3.y ), Coord( Rotated0.x, Rotated0.y ), a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawRectFilled( const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    const uint32_t YTop = a_Rect.y;
    const uint32_t YBottom = a_Rect.y + a_Rect.h - 1u;
    const uint32_t XLeft = a_Rect.x;
    const uint32_t XRight = a_Rect.x + a_Rect.w - 1u;

	Pixel* Buffer = m_Pixels + a_Rect.y * m_Width;

    for ( uint32_t y = YTop; y <= YBottom; ++y, Buffer += m_Width )
		for ( uint32_t x = XLeft; x <= XRight; ++x )
            Buffer[ x ] = a_FragmentFn( x, y, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawRectFilled( const Rect& a_Rect, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
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

    DrawTriangleFilled( Coord( Rotated0.x, Rotated0.y ), Coord( Rotated1.x, Rotated1.y ), Coord( Rotated2.x, Rotated2.y ), a_FragmentFn, a_FragmentFnPayload );
    DrawTriangleFilled( Coord( Rotated0.x, Rotated0.y ), Coord( Rotated2.x, Rotated2.y ), Coord( Rotated3.x, Rotated3.y ), a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawCircle( const Coord a_Centre, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO("This needs work.");
    auto setPixel = [ & ]( int x, int y )
    {
	    SetPixel( Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );
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

void ConsoleGL::PixelBuffer::DrawCircleFilled( const Coord a_Centre, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO( "this can be optimised to only calculate a quarter and flip coordinates for other 3 quadrants.");
	// Scan over bounding box.
    const uint32_t MinX = a_Centre.x - a_Radius;
    const uint32_t MaxX = a_Centre.x + a_Radius;
    const uint32_t MinY = a_Centre.y - a_Radius;
    const uint32_t MaxY = a_Centre.y + a_Radius;
    const int32_t CentreX = a_Centre.x;
    const int32_t CentreY = a_Centre.y;

    Pixel* Buffer = m_Pixels + MinY * m_Width;
    TODO( "Is the failure to use <= here failing the calcs?" );
    for ( int32_t y = MinY; y < MaxY; ++y, Buffer += m_Width )
        for ( int32_t x = MinX; x < MaxX; ++x )
            if ( ( x - CentreX ) * ( x - CentreX ) + ( y - CentreY ) * ( y - CentreY ) < a_Radius * a_Radius )
                // If pixel inside radius, draw.
                Buffer[ x ] = a_FragmentFn( x, y, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawEllipse( const Coord a_Centre, const Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	TODO("This needs work.");
    auto setPixel = [ & ]( int x, int y )
    {
	    SetPixel(Coord( x, y ), a_FragmentFn( x, y, a_FragmentFnPayload ) );
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
}

void ConsoleGL::PixelBuffer::DrawEllipseFilled( const Coord a_Centre, const Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    TODO( "this can be optimised to only calculate a quarter and flip coordinates for other 3 quadrants.");
	// Scan over bounding box.
    const uint32_t MinX = a_Centre.x - a_Radius.x;
    const uint32_t MaxX = a_Centre.x + a_Radius.x;
    const uint32_t MinY = a_Centre.y - a_Radius.y;
    const uint32_t MaxY = a_Centre.y + a_Radius.y;
    const int32_t LDen = a_Radius.x * a_Radius.x;
    const int32_t RDen = a_Radius.y * a_Radius.y;

    Pixel* Buffer = m_Pixels + MinY * m_Width;

    for ( int32_t y = MinY; y < MaxY; ++y, Buffer += m_Width )
        for ( int32_t x = MinX; x < MaxX; ++x )
        {
            const int32_t LNum = ( x - a_Centre.x ) * ( x - a_Centre.x );
            const int32_t RNum = ( y - a_Centre.y ) * ( y - a_Centre.y );

	        if ( RDen * LNum + LDen * RNum < LDen * RDen )
                // If pixel inside radius, draw.
                Buffer[ x ] = a_FragmentFn( x, y, a_FragmentFnPayload );
        }
}
