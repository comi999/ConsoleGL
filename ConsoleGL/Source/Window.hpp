#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include <Pixel.hpp>
#include <Colour.hpp>
#include <Event.hpp>
#include <FileMap.hpp>

namespace ConsoleGL
{
enum class EColourSet
{
    DEFAULT,
    NEW,
    LEGACY,
    GREYSCALE,
    SEPIA,
};

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

    WindowDock( WindowDock&& ) = default;
    WindowDock& operator=( WindowDock&& ) = default;
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

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetArea() const { return m_Width * m_Height; }
    uint32_t GetBufferIndex() const { return m_ActiveBuffer; }
    uint32_t GetBufferCount() const { return ( uint32_t )m_Buffers.size(); }
    Pixel* GetBuffer() { return m_Buffers[ m_ActiveBuffer ]; }
    const Pixel* GetBuffer() const { return m_Buffers[ m_ActiveBuffer ]; }
    Pixel* GetBuffer( const uint32_t a_Index ) { return m_Buffers[ a_Index ]; }
    const Pixel* GetBuffer( const uint32_t a_Index ) const { return m_Buffers[ a_Index ]; }
    void SetBuffer( const Pixel a_Pixel );
    void SetRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel );
    void SetPixel( const uint32_t a_Index, const Pixel a_Pixel );
    void SetPixel( const uint32_t a_X, const uint32_t a_Y, const Pixel a_Pixel );
    void SetPixels( const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel );
    void SetPixels( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Count, const Pixel a_Pixel );

private:

    friend class WindowDock;
    
    Window( WindowDock& a_Dock );
    ~Window();

    uint32_t                        m_Width;
    uint32_t                        m_Height;
    uint32_t                        m_PixelWidth;
    uint32_t                        m_PixelHeight;
    std::vector< Pixel* >           m_Buffers;
    uint32_t                        m_ActiveBuffer;
    std::string                     m_Title;
    WindowDock*                     m_Dock;
};
} // namespace ConsoleGL