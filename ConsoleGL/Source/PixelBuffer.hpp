#pragma once
#include <ConsoleGL.hpp>

namespace ConsoleGL
{
#define IN_PLACE_PIXEL +[]( const Coord a_Coord, void* a_Stage ) { return *( const Pixel* )a_Stage; }, const_cast< Pixel* >( &a_Pixel )

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
    void SetPixel( const Coord a_Position, const Pixel a_Pixel ) { SetPixel( a_Position.y * m_Width + a_Position.x, a_Pixel ); }

    void SetPixels( const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel ) { for ( Pixel* Begin = m_Pixels + a_Index, *End = Begin + a_Count; Begin != End; ++Begin ) *Begin = a_Pixel; }
    void SetPixels( const Coord a_Position, const uint32_t a_Count, const Pixel a_Pixel ) { SetPixels( a_Position.y * m_Width + a_Position.x, a_Count, a_Pixel ); }

    // Region operations
    void SetBuffer( const Pixel a_Pixel ) { SetPixels( 0u, GetSize(), a_Pixel ); }
    void SetBuffer( const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawLine( const Seg& a_Seg, const Pixel a_Pixel ) { DrawLine( a_Seg, IN_PLACE_PIXEL ); }
    void DrawLine( const Seg& a_Seg, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawHorizontalLine( const Coord a_Begin, const uint32_t a_Length, const Pixel a_Pixel ) { SetPixels( a_Begin, a_Length, a_Pixel ); }
    void DrawHorizontalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawVerticalLine( const Coord a_Begin, const uint32_t a_Length, const Pixel a_Pixel ) { DrawVerticalLine( a_Begin, a_Length, IN_PLACE_PIXEL ); }
    void DrawVerticalLine( const Coord a_Begin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawTriangle( const Tri& a_Tri, const Pixel a_Pixel ) { DrawTriangle( a_Tri, IN_PLACE_PIXEL ); }
    void DrawTriangle( const Tri& a_Tri, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawTriangleFilled( const Tri& a_Tri, const Pixel a_Pixel ) { DrawTriangleFilled( a_Tri, IN_PLACE_PIXEL ); }
    void DrawTriangleFilled( const Tri& a_Tri, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawRect( const Rect& a_Rect, const Pixel a_Pixel ) { DrawRect( a_Rect, IN_PLACE_PIXEL ); }
    void DrawRect( const Rect& a_Rect, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRect( const Rect& a_Rect, const float a_Radians, const Pixel a_Pixel ) { DrawRect( a_Rect, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRect( const Rect& a_Rect, const float a_Radians, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawRectFilled( const Rect& a_Rect, const Pixel a_Pixel ) { DrawRectFilled( a_Rect, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const Rect& a_Rect, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRectFilled( const Rect& a_Rect, const float a_Radians, const Pixel a_Pixel ) { DrawRectFilled( a_Rect, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const Rect& a_Rect, const float a_Radians, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircle( const Coord a_Centre, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircle( a_Centre, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircle( const Coord a_Centre, const uint32_t a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircleFilled( const Coord a_Centre, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircleFilled( a_Centre, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircleFilled( const Coord a_Centre, const uint32_t a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawEllipse( const Coord a_Centre, const Coord a_Radius, const Pixel a_Pixel ) { DrawEllipse( a_Centre, a_Radius, IN_PLACE_PIXEL ); }
    void DrawEllipse( const Coord a_Centre, const Coord a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawEllipseFilled( const Coord a_Centre, const Coord a_Radius, const Pixel a_Pixel ) { DrawEllipseFilled( a_Centre, a_Radius, IN_PLACE_PIXEL ); }
    void DrawEllipseFilled( const Coord a_Centre, const Coord a_Radius, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    // Text operations
    //...

    // Mapping functions
   void DrawOnto( PixelBuffer* a_Buffer, const Coord a_Origin );
    
private:

    Pixel* m_Pixels;
    uint32_t m_Width;
    uint32_t m_Height;
};

#undef IN_PLACE_PIXEL
}