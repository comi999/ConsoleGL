#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include <ConsoleGL.hpp>
#include <Event.hpp>
#include <FileMap.hpp>

namespace ConsoleGL
{
class WindowDock
{
    enum class ECommand
    {
        Release,
        Attach,
        Terminate
    };
    
    struct CommandBuffer
    {
        ECommand Command;
        uint32_t Value;
    };
    
    WindowDock();
    WindowDock( const WindowDock& ) = delete;

public:

    WindowDock( WindowDock&& a_WindowDock ) noexcept;
    WindowDock& operator=( WindowDock&& a_WindowDock ) noexcept;
    ~WindowDock();

private:

    WindowDock& operator=( const WindowDock& ) = delete;

    static WindowDock* Create();
    static bool Destroy( const WindowDock* a_WindowDock );

public:

    static int RunListener( const std::string& a_Name );

private:

    bool IsBorrowed() const;
    bool Borrow() const;
    bool Return() const;
    bool IsValid() const { return m_InternalInfo.get(); }

    friend class Window;
    struct InternalInfo;

    std::shared_ptr< InternalInfo > m_InternalInfo;
    FileMap                         m_CommandBuffer;
	Event                           m_CommandReady;
    Event                           m_CommandComplete;
    Event                           m_ProcessStarted;
    Window*                         m_Docked;

    static const WindowDock*        s_CurrentlyBorrowed;
    static std::list< WindowDock >  s_WindowDocks;
};

#define IN_PLACE_PIXEL +[]( uint32_t, uint32_t, void* a_FragmentFnPayload ) { return *( const Pixel* )a_FragmentFnPayload; }, const_cast< Pixel* >( &a_Pixel )

class PixelBuffer
{
public:

    using FragmentFn = Pixel( * )( const uint32_t a_PosX, const uint32_t a_PosY, void* a_Payload );

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
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const Pixel a_Pixel ) { DrawRect( a_X, a_Y, a_Width, a_Height, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel ) { DrawRectFilled( a_X, a_Y, a_Width, a_Height, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, const Pixel a_Pixel ) { DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_Radians, IN_PLACE_PIXEL ); }
    void DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircle( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircle( a_X, a_Y, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircle( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawCircleFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, const Pixel a_Pixel ) { DrawCircleFilled( a_X, a_Y, a_Radius, IN_PLACE_PIXEL ); }
    void DrawCircleFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    void DrawEllipse( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const Pixel a_Pixel ) { DrawEllipse( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, IN_PLACE_PIXEL ); }
    void DrawEllipse( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );
    
    void DrawEllipseFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, const Pixel a_Pixel ) { DrawEllipseFilled( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, IN_PLACE_PIXEL ); }
    void DrawEllipseFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload );

    // Text operations
    
private:

    Pixel* m_Pixels;
    uint32_t m_Width;
    uint32_t m_Height;
};

#undef IN_PLACE_PIXEL

class Window
{
public:

    static const Colour ColourSetDefault[ 16 ];
    static const Colour ColourSetGreyscale[ 16 ];
    static const Colour ColourSetSepia[ 16 ];

    Window( const Window& ) = delete;
    Window( Window&& ) = delete;
    Window& operator=( const Window& ) = delete;
    Window& operator=( Window&& ) = delete;

    static Window* Create( const std::string& a_Title, uint32_t a_Width, uint32_t a_Height, uint32_t a_PixelWidth = 8, uint32_t a_PixelHeight = 8, uint32_t a_BufferCount = 2 );
    static void Destroy( Window* a_Window );
    static void SetActive( Window* a_ConsoleWindow );
    static Window* GetActive();
    static void SetTitle( const std::string& a_Title );
    static void SetColours( const Colour* a_Colours );
    static void SetColours( const EColourSet a_ColourSet );
    static void SwapBuffer();
    static void SwapBuffer( const uint32_t a_Index );

    const std::string& GetTitle() const { return m_Title; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetArea() const { return m_Width * m_Height; }
    uint32_t GetBufferIndex() const { return m_ActiveBuffer; }
    uint32_t GetBufferCount() const { return ( uint32_t )m_Buffers.size(); }
    const PixelBuffer* GetBuffer() const { return &m_Buffers[ m_ActiveBuffer ]; }
    PixelBuffer* GetBuffer() { return &m_Buffers[ m_ActiveBuffer ]; }
    PixelBuffer* GetBuffer( const uint32_t a_Index ) { return &m_Buffers[ a_Index ]; }
    const PixelBuffer* GetBuffer( const uint32_t a_Index ) const { return &m_Buffers[ a_Index ]; }

private:

    friend class WindowDock;
    
    Window( WindowDock& a_Dock );
    ~Window();

    uint32_t                   m_Width;
    uint32_t                   m_Height;
    uint32_t                   m_PixelWidth;
    uint32_t                   m_PixelHeight;
    std::vector< PixelBuffer > m_Buffers;
    uint32_t                   m_ActiveBuffer;
    std::string                m_Title;
    WindowDock*                m_Dock;
};
} // namespace ConsoleGL