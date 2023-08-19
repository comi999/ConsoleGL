#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>

#include <ConsoleGL/Pixel.hpp>
#include <ConsoleGL/Colour.hpp>

#ifndef CONSOLE_DOCK_PROCESS_PATH
    #ifdef _DEBUG
        #define CONSOLE_DOCK_PROCESS_PATH L"Debug/ConsoleGL-ConsoleDock.exe"
    #else
        #define CONSOLE_DOCK_PROCESS_PATH L"Release/ConsoleGL-ConsoleDock.exe"
    #endif
#endif

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

class Window
{
private:

    typedef HANDLE      ConsoleHandle;
    typedef DWORD       ConsoleProcessID;
    typedef HWND        WindowHandle;
    typedef SMALL_RECT  WindowRegion;

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
    static Window* GetActive() { return s_Active; }
    static void SetTitle( const std::string& a_Title ); 
    static void SetColours( const Colour* a_Colours );
    static void SetColours( EColourSet a_ColourSet );
    static void SwapBuffer();
    static void SwapBuffer( uint32_t a_Index );

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetArea() const { return m_Width * m_Height; }
    uint32_t GetBufferIndex() const { return m_ActiveBuffer; }
    uint32_t GetBufferCount() const { return m_Buffers.size(); }
    Pixel* GetBuffer() { return m_Buffers[ m_ActiveBuffer ]; }
    const Pixel* GetBuffer() const { return m_Buffers[ m_ActiveBuffer ]; }
    Pixel* GetBuffer( uint32_t a_Index ) { return m_Buffers[ a_Index ]; }
    const Pixel* GetBuffer( uint32_t a_Index ) const { return m_Buffers[ a_Index ]; }
    void SetBuffer( Pixel a_Pixel );
    void SetRect( uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel );
    void SetPixel( uint32_t a_Index, Pixel a_Pixel );
    void SetPixel( uint32_t a_X, uint32_t a_Y, Pixel a_Pixel );
    void SetPixels( uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel );
    void SetPixels( uint32_t a_X, uint32_t a_Y, uint32_t a_Count, Pixel a_Pixel );

private:
    
    Window();
    ~Window();

    ConsoleHandle               m_ConsoleOutputHandle;
    ConsoleHandle               m_ConsoleInputHandle;
    WindowHandle                m_WindowHandle;
    WindowRegion                m_WindowRegion;
    STARTUPINFO                 m_StartupInfo;
    PROCESS_INFORMATION         m_ProcessInfo;
    ConsoleProcessID            m_ConsoleProcessID;
    uint32_t                    m_Width;
    uint32_t                    m_Height;
    uint32_t                    m_PixelWidth;
    uint32_t                    m_PixelHeight;
    std::vector< Pixel* >       m_Buffers;
    uint32_t                    m_ActiveBuffer;
    std::string                 m_Title;
    static Window*              s_Active;
};
} // namespace ConsoleGL