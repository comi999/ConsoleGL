#pragma once
#include <ConsoleGL.hpp>

namespace ConsoleGL
{
#define IN_PLACE_PIXEL +[]( uint32_t, uint32_t, void* a_FragmentFnPayload ) { return *( const Pixel* )a_FragmentFnPayload; }, const_cast< Pixel* >( &a_Pixel )

class PixelBuffer
{
public:

    PixelBuffer( const uint32_t a_Width, const uint32_t a_Height );
    PixelBuffer( const PixelBuffer& a_PixelBuffer );
    PixelBuffer( PixelBuffer&& a_PixelBuffer ) noexcept;
    PixelBuffer& operator=( const PixelBuffer& a_PixelBuffer );
    PixelBuffer& operator=( PixelBuffer&& a_PixelBuffer ) noexcept;
    ~PixelBuffer();

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetSize() const { return m_Width * m_Height; }
    const Pixel* GetPixels() const { return m_Pixels; }
    Pixel* GetPixels() { return m_Pixels; }
    
    // Pixel operations.
    void SetPixel( const uint32_t a_Index, const Pixel a_Pixel ) { m_Pixels[ a_Index ] = a_Pixel; }
    void SetPixel( const uint32_t a_X, const uint32_t a_Y, const Pixel a_Pixel ) { SetPixel( a_Y * m_Width + a_X, a_Pixel ); }

    void SetPixels( const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel ) { for ( Pixel* Begin = m_Pixels + a_Index, *End = Begin + a_Count; Begin != End; ++Begin ) *Begin = a_Pixel; }
    void SetPixels( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Count, const Pixel a_Pixel ) { SetPixels( a_Y * m_Width + a_X, a_Count, a_Pixel ); }

    // Region operations
    void SetBuffer( const Pixel a_Pixel ) { SetPixels( 0u, GetSize(), a_Pixel ); }
    void SetBuffer( const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawLine( const uint32_t a_XBegin, const uint32_t a_XEnd, const uint32_t a_YBegin, const uint32_t a_YEnd, const Pixel a_Pixel ) { DrawLine( a_XBegin, a_XEnd, a_YBegin, a_YEnd, IN_PLACE_PIXEL ); }
    void DrawLine( const uint32_t a_XBegin, const uint32_t a_XEnd, const uint32_t a_YBegin, const uint32_t a_YEnd, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawHorizontalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const Pixel a_Pixel ) { SetPixels( a_XBegin, a_YBegin, a_Length, a_Pixel ); }
    void DrawHorizontalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawVerticalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const Pixel a_Pixel ) { DrawVerticalLine( a_XBegin, a_YBegin, a_Length, IN_PLACE_PIXEL ); }
    void DrawVerticalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawTriangle( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const Pixel a_Pixel ) { DrawTriangle( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, IN_PLACE_PIXEL ); }
    void DrawTriangle( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawTriangleFilled( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const Pixel a_Pixel ) { DrawTriangleFilled( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, IN_PLACE_PIXEL ); }
    void DrawTriangleFilled( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel ) { DrawRect( a_X, a_Y, a_Width, a_Height, IN_PLACE_PIXEL ); }
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const Pixel a_Pixel ) { DrawRect( a_X, a_Y, a_Width, a_Height, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel ) { DrawRectFilled( a_X, a_Y, a_Width, a_Height, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const Pixel a_Pixel ) { DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircle( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircle( a_X, a_Y, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircle( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircleFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircleFilled( a_X, a_Y, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircleFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawEllipse( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const Pixel a_Pixel ) { DrawEllipse( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, IN_PLACE_PIXEL ); }
    void DrawEllipse( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawEllipseFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const Pixel a_Pixel ) { DrawEllipseFilled( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, IN_PLACE_PIXEL ); }
    void DrawEllipseFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    // Text operations

    // Mapping functions
   void DrawOnto( PixelBuffer* a_Buffer, const uint32_t a_OffsetX, const uint32_t a_OffsetY, const uint32_t a_Width, const uint32_t a_Height );
    
private:

    Pixel* m_Pixels;
    uint32_t m_Width;
    uint32_t m_Height;
};

#undef IN_PLACE_PIXEL
}