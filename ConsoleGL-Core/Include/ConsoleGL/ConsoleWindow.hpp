//#pragma once
//#include <Windows.h>
//#include <thread>
//#include <condition_variable>
//#include "Math.hpp"
//#include "Colour.hpp"
//#include "ScreenBuffer.hpp"
//#include "PixelColourMap.hpp"
//
//class ConsoleWindow
//{
//public:
//
//    typedef HANDLE      ConsoleHandle;
//    typedef HWND        WindowHandle;
//    typedef SMALL_RECT  WindowRegion;
//    typedef std::thread Thread;
//
//    void SetTitle( const char* a_Title )
//    {
//        size_t Length = strlen( a_Title ) + 1;
//        Length = Length > 64 ? 64 : Length;
//        mbstowcs_s( nullptr, m_TitleBuffer, Length, a_Title, Length );
//        SetConsoleTitleW( m_TitleBuffer );
//    }
//
//    inline Vector2Int GetSize()
//    {
//        return m_ScreenBuffer.GetSize();
//    }
//
//    inline short GetArea()
//    {
//        return m_ScreenBuffer.GetArea();
//    }
//
//    inline short GetWidth()
//    {
//        return m_ScreenBuffer.GetWidth();
//    }
//
//    inline short GetHeight()
//    {
//        return m_ScreenBuffer.GetHeight();
//    }
//
//    inline Vector< short, 2 > GetPixelSize()
//    {
//        return m_PixelSize;
//    }
//
//    inline short GetPixelWidth()
//    {
//        return m_PixelSize.x;
//    }
//
//    inline short GetPixelHeight()
//    {
//        return m_PixelSize.y;
//    }
//
//    inline ConsoleHandle GetConsoleHandle()
//    {
//        return m_ConsoleHandle;
//    }
//
//    inline WindowHandle GetWindowHandle()
//    {
//        return m_WindowHandle;
//    }
//
//    static ConsoleWindow* Create( const char* a_Title, Vector< short, 2 > a_Size, Vector< short, 2 > a_PixelSize )
//    {
//        ConsoleWindow* NewWindow = new ConsoleWindow();
//
//        // Retrieve handles for console window.
//        NewWindow->m_ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
//
//        if ( NewWindow->m_ConsoleHandle == INVALID_HANDLE_VALUE )
//        {
//            AllocConsole();
//            NewWindow->m_ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
//        }
//
//        NewWindow->m_WindowHandle = GetConsoleWindow();
//
//        // Set console font.
//        a_PixelSize.x = Math::Min( a_PixelSize.x, static_cast< short >( 8 ) );
//        a_PixelSize.y = Math::Min( a_PixelSize.y, static_cast< short >( 8 ) );
//        CONSOLE_FONT_INFOEX FontInfo;
//        FontInfo.cbSize = sizeof( FontInfo );
//        FontInfo.nFont = 0;
//        FontInfo.dwFontSize = { a_PixelSize.x, a_PixelSize.y };
//        FontInfo.FontFamily = FF_DONTCARE;
//        FontInfo.FontWeight = FW_NORMAL;
//        wcscpy_s( FontInfo.FaceName, L"Terminal" );
//        SetCurrentConsoleFontEx( NewWindow->m_ConsoleHandle, false, &FontInfo );
//        NewWindow->m_PixelSize = a_PixelSize;
//
//        // Get screen buffer info object.
//        CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
//        ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
//        GetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleHandle, &ScreenBufferInfo );
//
//        for ( int i = 0; i < 16; ++i )
//        {
//            COLORREF& ColourRef = ScreenBufferInfo.ColorTable[ i ];
//            Colour SeedColour = PixelColourMap::SeedColours[ i ];
//            ColourRef =
//                SeedColour.B << 16 |
//                SeedColour.G << 8  |
//                SeedColour.R;
//        }
//
//        SetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleHandle, &ScreenBufferInfo );
//
//        // Get largest possible window size that can fit on screen.
//        COORD LargestWindow = GetLargestConsoleWindowSize( NewWindow->m_ConsoleHandle );
//
//        // If smaller than requested size, exit.
//        if ( LargestWindow.X < a_Size.x ||
//             LargestWindow.Y < a_Size.y )
//        {
//            return nullptr;
//        }
//
//        // Set screen buffer.
//        NewWindow->m_ScreenBuffer.Initialize( a_Size );
//        COORD WindowSize = { NewWindow->GetWidth(), NewWindow->GetHeight() };
//
//        // Set window region rect.
//        NewWindow->m_WindowRegion.Left = 0;
//        NewWindow->m_WindowRegion.Top = 0;
//        NewWindow->m_WindowRegion.Right = WindowSize.X - 1;
//        NewWindow->m_WindowRegion.Bottom = WindowSize.Y - 1;
//
//        // Set console attributes.
//        SetConsoleScreenBufferSize( NewWindow->m_ConsoleHandle, { a_Size.x, a_Size.y } );
//        SetConsoleWindowInfo( NewWindow->m_ConsoleHandle, true, &NewWindow->m_WindowRegion );
//        GetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleHandle, &ScreenBufferInfo );
//        SetConsoleScreenBufferSize( NewWindow->m_ConsoleHandle, { a_Size.x, a_Size.y } );
//        
//        // Set cursor attributes.
//        CONSOLE_CURSOR_INFO CursorInfo;
//        GetConsoleCursorInfo( NewWindow->m_ConsoleHandle, &CursorInfo );
//        CursorInfo.bVisible = false;
//        SetConsoleCursorInfo( NewWindow->m_ConsoleHandle, &CursorInfo );
//
//        // Set window attributes.
//        SetWindowLong( NewWindow->m_WindowHandle, GWL_STYLE, WS_CAPTION | DS_MODALFRAME | WS_MINIMIZEBOX | WS_SYSMENU );
//        SetWindowPos( NewWindow->m_WindowHandle, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW );
//        NewWindow->SetTitle( a_Title );
//        NewWindow->m_Thread = new Thread( []( ConsoleWindow* a_ConsoleWindow )
//                                          {
//                                              while ( true )
//                                              {
//                                                  std::unique_lock< std::mutex > Locker( a_ConsoleWindow->m_Mutex );
//                                                  a_ConsoleWindow->m_ConditionVariable.wait( Locker );
//                                                  
//                                                  if ( a_ConsoleWindow->m_BufferReady )
//                                                  {
//                                                      a_ConsoleWindow->WriteBuffer();
//                                                      a_ConsoleWindow->m_BufferReady = false;
//                                                  }
//                                              }
//                                          }, NewWindow );
//        return NewWindow;
//    }
//
//    ScreenBuffer& GetScreenBuffer()
//    {
//        return m_ScreenBuffer;
//    }
//
//    static ConsoleWindow* GetCurrentContext()
//    {
//        return s_ActiveWindow;
//    }
//
//    static void MakeContextCurrent( ConsoleWindow* a_Window )
//    {
//        s_ActiveWindow = a_Window;
//    }
//
//    static void SwapBuffers( ConsoleWindow* a_Window )
//    {
//        a_Window->m_ScreenBuffer.SwapPixelBuffer();
//        a_Window->DrawBuffer();
//    }
//
//private:
//
//    void DrawBuffer()
//    {
//        m_BufferReady = true;
//        m_ConditionVariable.notify_one();
//
//        //while ( m_BufferReady );
//    }
//
//    void WriteBuffer()
//    {
//        WriteConsoleOutput(
//            m_ConsoleHandle,
//            m_ScreenBuffer.GetPixelBuffer(),
//            { m_ScreenBuffer.GetWidth(), m_ScreenBuffer.GetHeight() },
//            { 0, 0 },
//            &m_WindowRegion );
//    }
//
//    bool                         m_BufferReady;
//    ConsoleHandle                m_ConsoleHandle;
//    WindowHandle                 m_WindowHandle;
//    WindowRegion                 m_WindowRegion;
//    Vector< short, 2 >           m_PixelSize;
//    wchar_t                      m_TitleBuffer[ 64 ];
//    std::string                  m_Title;
//    ScreenBuffer                 m_ScreenBuffer;
//    Thread*                      m_Thread;
//    std::condition_variable      m_ConditionVariable;
//    std::mutex                   m_Mutex;
//    inline static ConsoleWindow* s_ActiveWindow;
//};

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>

#include <ConsoleGL/Pixel.hpp>

#ifndef CONSOLE_DOCK_PROCESS_PATH
#ifdef _DEBUG
    #define CONSOLE_DOCK_PROCESS_PATH L"Release/ConsoleGL-ConsoleDock.exe"
#else
    #define CONSOLE_DOCK_PROCESS_PATH L"Release/ConsoleGL-ConsoleDock.exe"
#endif
#endif

namespace ConsoleGL
{
class Window
{
private:

    typedef HANDLE      ConsoleHandle;
    typedef DWORD       ConsoleProcessID;
    typedef HWND        WindowHandle;
    typedef SMALL_RECT  WindowRegion;

public:

    Window( const Window& ) = delete;
    Window( Window&& ) = delete;
    Window& operator=( const Window& ) = delete;
    Window& operator=( Window&& ) = delete;

    static Window* Create( const std::string& a_Title, uint32_t a_Width, uint32_t a_Height, uint32_t a_PixelWidth = 8, uint32_t a_PixelHeight = 8, uint32_t a_BufferCount = 2 );
    static void Destroy();
    static void Destroy( Window* a_Window );
    static void SetActive( Window* a_ConsoleWindow );
    static Window* GetActive() { return s_Active; }
    static void SetTitle( const std::string& a_Title );
    static void SetTitle( Window* a_Window, const std::string& a_Title );
    static void SwapBuffer();
    static void SetBuffer( Pixel a_Pixel );
    static void SetRect( uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel );
    static void SetPixel( uint32_t a_Index, Pixel a_Pixel );
    static void SetPixel( uint32_t a_X, uint32_t a_Y, Pixel a_Pixel ); 
    static void SetPixels( uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel );
    static void SetPixels( uint32_t a_X, uint32_t a_Y, uint32_t a_Count, Pixel a_Pixel );

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

    //bool                      m_BufferReady;
    //ScreenBuffer              m_ScreenBuffer;
    //Thread*                   m_Thread;
    //std::condition_variable   m_ConditionVariable;
    //std::mutex                m_Mutex;
};
} // namespace ConsoleGL