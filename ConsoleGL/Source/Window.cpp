#include <chrono>
#include <algorithm>

#include <Window.hpp>

// Disable dll exported functions.
#define NOUSER
#include <Windows.h>

#define CONSOLE_DOCK_PATH "ConsoleGL-ConsoleDock.exe"

/*
BLACK	        0,0,0	    
DARK_BLUE	    0,0,128	    
DARK_GREEN	    0,128,0	    
DARK_CYAN	    0,128,128	
DARK_RED	    128,0,0	    
DARK_MAGENTA	128,0,128	
DARK_YELLOW	    128,128,0	
DARK_WHITE	    192,192,192	
BRIGHT_BLACK	128,128,128	
BRIGHT_BLUE	    0,0,255	    
BRIGHT_GREEN	0,255,0	    
BRIGHT_CYAN	    0,255,255	
BRIGHT_RED	    255,0,0	    
BRIGHT_MAGENTA	255,0,255	
BRIGHT_YELLOW	255,255,0	
WHITE	        255,255,255	
*/

const ConsoleGL::WindowDock*        ConsoleGL::WindowDock::s_CurrentlyBorrowed;
std::list< ConsoleGL::WindowDock >  ConsoleGL::WindowDock::s_WindowDocks;

struct ConsoleGL::WindowDock::InternalInfo
{
    HWND                WindowHandle;
    SMALL_RECT          WindowRegion;
    STARTUPINFOA        StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
};

const ConsoleGL::Colour ConsoleGL::Window::ColourSetDefault[ 16 ] =
{
    { 0,   0,   0   },
    { 255, 0,   0   },
    { 0,   255, 0   },
    { 0,   0,   255 },
    { 255, 255, 0   },
    { 255, 0,   255 },
    { 0,   255, 255 },
    { 255, 255, 255 },
    { 85,  85,  85  },
    { 170, 85,  85  },
    { 85,  170, 85  },
    { 85,  85,  170 },
    { 170, 170, 85  },
    { 170, 85,  170 },
    { 85,  170, 170 },
    { 170, 170, 170 }
};

const ConsoleGL::Colour ConsoleGL::Window::ColourSetGreyscale[ 16 ] =
{
    { 0,   0,   0   },
    { 76,  76,  76  },
    { 149, 149, 149 },
    { 29,  29,  29  },
    { 225, 225, 225 },
    { 105, 105, 105 },
    { 178, 178, 178 },
    { 254, 254, 254 },
    { 83,  83,  83  },
    { 108, 108, 108 },
    { 133, 133, 133 },
    { 93,  93,  93  },
    { 158, 158, 158 },
    { 118, 118, 118 },
    { 143, 143, 143 },
    { 168, 168, 168 }
};

const ConsoleGL::Colour ConsoleGL::Window::ColourSetSepia[ 16 ] =
{
    { 0,   0,   0   },
    { 100, 88,  69  },
    { 196, 174, 136 },
    { 48,  42,  33  },
    { 255, 255, 205 },
    { 148, 131, 102 },
    { 244, 217, 169 },
    { 255, 255, 238 },
    { 114, 102, 79  },
    { 148, 131, 102 },
    { 180, 160, 125 },
    { 130, 116, 90  },
    { 213, 190, 148 },
    { 164, 146, 113 },
    { 196, 174, 136 },
    { 229, 204, 159 }
};

#if __has_include("ConsoleDock.inl") && IS_CONSOLEGL
MESSAGE( "ConsoleDock.inl found." );
#include <ConsoleDock.inl>
#define CONSOLE_DOCK_FOUND
#include <fstream>
#include <filesystem>
#endif

ConsoleGL::WindowDock::WindowDock()
	: m_Docked( nullptr )
{
    const std::string Prefix = std::to_string( std::chrono::high_resolution_clock::now().time_since_epoch().count() );

    // Create a command buffer.
    if ( !m_CommandBuffer.Create( Prefix + "_CommandBuffer", sizeof( CommandBuffer ) ) )
    {
        return;
    }

    // Create the command ready event.
    if ( !m_CommandReady.Create( Prefix + "_CommandReady", true ) )
    {
        m_CommandBuffer.Clear();
        return;
    }
    
    // Create the command complete event.
    if ( !m_CommandComplete.Create( Prefix + "_CommandComplete", true ) )
    {
        m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        return;
    }

    if ( !m_ProcessStarted.Create( Prefix + "_ProcessStarted" ) )
    {
	    m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        m_CommandComplete.Clear();
        return;
    }

    m_InternalInfo = std::make_shared< InternalInfo >();
    ZeroMemory( &m_InternalInfo->StartupInfo, sizeof( m_InternalInfo->StartupInfo ) );
    m_InternalInfo->StartupInfo.cb = sizeof( m_InternalInfo->StartupInfo );
    ZeroMemory( &m_InternalInfo->ProcessInfo, sizeof( m_InternalInfo->ProcessInfo ) );

#ifdef CONSOLE_DOCK_FOUND
    MESSAGE("Found console dock exe array." );
    bool FailedToWriteConsoleDockExe = false;

    if ( !std::filesystem::exists( "ConsoleDock.exe" ) )
    {
    	std::ofstream ConsoleDockExe( "ConsoleDock.exe", std::ios::binary | std::ios::out );

        if ( !ConsoleDockExe.is_open() )
	    {
		    FailedToWriteConsoleDockExe = true;
	    }
        else
        {
	        ConsoleDockExe.write( ( const char* )ConsoleDock, sizeof( ConsoleDock ) );
        }
    }
#endif

    // This should be replaced with a Process object.
    if (
#ifdef CONSOLE_DOCK_FOUND
        FailedToWriteConsoleDockExe ||
        !CreateProcessA(
        nullptr,
        ( "ConsoleDock.exe " + Prefix ).data(),
        nullptr,
        nullptr,
        true,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &m_InternalInfo->StartupInfo,
        &m_InternalInfo->ProcessInfo )
#else
        true
#endif
        )
    {
	    m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        m_CommandComplete.Clear();
        m_ProcessStarted.Clear();
        m_InternalInfo.reset();
        return;
    }

    if ( !m_ProcessStarted.Wait() )
    {
	    m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        m_CommandComplete.Clear();
        m_ProcessStarted.Clear();
        m_InternalInfo.reset();

        ENSURE_LOG( TerminateProcess( m_InternalInfo->ProcessInfo.hProcess, 1 ), "Failed to terminate console dock process." );
    }
}

ConsoleGL::WindowDock::WindowDock( WindowDock&& a_WindowDock ) noexcept
	: m_InternalInfo( std::move( a_WindowDock.m_InternalInfo ) )
	, m_CommandBuffer( std::move( a_WindowDock.m_CommandBuffer ) )
	, m_CommandReady( std::move( a_WindowDock.m_CommandReady ) )
	, m_CommandComplete( std::move( a_WindowDock.m_CommandComplete ) )
	, m_ProcessStarted( std::move( a_WindowDock.m_ProcessStarted ) )
	, m_Docked( a_WindowDock.m_Docked )
{
	a_WindowDock.m_Docked = nullptr;
}

ConsoleGL::WindowDock::~WindowDock()
{
    // If this is an invalid window dock, nothing to destroy.
    if ( !IsValid() )
    {
	    return;
    }

    m_CommandBuffer.Clear();
    m_CommandReady.Clear();
    m_CommandComplete.Clear();
	m_ProcessStarted.Clear();
    ENSURE_LOG( TerminateProcess( m_InternalInfo->ProcessInfo.hProcess, 0 ), "Failed to terminate console dock process." );
    delete m_Docked;
}

ConsoleGL::WindowDock& ConsoleGL::WindowDock::operator=( WindowDock&& a_WindowDock ) noexcept
{
    // If this is an invalid window dock, nothing to destroy.
    if ( !IsValid() )
    {
		return *this;
    }

    m_InternalInfo      = std::move( a_WindowDock.m_InternalInfo );
	m_CommandBuffer     = std::move( a_WindowDock.m_CommandBuffer );
	m_CommandReady      = std::move( a_WindowDock.m_CommandReady );
	m_CommandComplete   = std::move( a_WindowDock.m_CommandComplete );
	m_ProcessStarted    = std::move( a_WindowDock.m_ProcessStarted );
	m_Docked            = a_WindowDock.m_Docked;

    a_WindowDock.m_Docked = nullptr;
    
	return *this;
}

ConsoleGL::WindowDock* ConsoleGL::WindowDock::Create()
{
    if ( WindowDock Dock; Dock.IsValid() )
    {
	    return &s_WindowDocks.emplace_back( std::move( Dock ) );
    }

    return nullptr;
}

bool ConsoleGL::WindowDock::Destroy( const WindowDock* a_WindowDock )
{
	if ( !a_WindowDock )
	{
		return false;
	}

    const auto Iter = std::find_if( s_WindowDocks.begin(), s_WindowDocks.end(), [ a_WindowDock ]( const WindowDock& Dock ) { return &Dock == a_WindowDock; } );

    if ( Iter == s_WindowDocks.end() )
    {
	    return false;
    }

    s_WindowDocks.erase( Iter );
    return true;
}

int ConsoleGL::WindowDock::RunListener( const std::string& a_Name )
{
    const Event ProcessStarted( a_Name + "_ProcessStarted" );
    const Event CommandReady( a_Name + "_CommandReady" );
    const Event CommandComplete( a_Name + "_CommandComplete" );
    const FileMap CommandBuffer( a_Name + "_CommandBuffer", sizeof( CommandBuffer ), true );

    ENSURE_LOG( ProcessStarted.IsValid(), "Could not create event '%s_ProcessStarted.", a_Name );
    ENSURE_LOG( CommandReady.IsValid(), "Could not create event '%s_CommandReady.", a_Name );
    ENSURE_LOG( CommandComplete.IsValid(), "Could not create event '%s_CommandComplete.", a_Name );
    ENSURE_LOG( CommandBuffer.IsValid(), "Could not create file map '%s_CommandBuffer.", a_Name );

    ENSURE_LOG( ProcessStarted.Trigger(), "Could not trigger '%s_ProcessStarted'.", a_Name );

    bool Continue = true;

    while ( Continue )
    {
	    ENSURE_LOG( CommandReady.Wait(), "Failed to wait for event '%s_CommandReady." );

        const auto Buffer = ( WindowDock::CommandBuffer* )CommandBuffer.Data();

	    switch ( Buffer->Command )
	    {
		case ECommand::Attach:
        {
            const DWORD ProcessID = Buffer->Value;
            ENSURE_LOG( AttachConsole( ProcessID ), "Could not attach console for process ID %u.", ProcessID );
            break;
        }
	    case ECommand::Release:
		{
			ENSURE_LOG( FreeConsole(), "Could not free console." );
            break;
		}
	    case ECommand::Terminate:
		{
			Continue = false;
            break;
		}
	    }

        ENSURE_LOG( CommandComplete.Trigger(), "Failed to wait for event '%s_CommandComplete." );
    }

	return 0;
}

bool ConsoleGL::WindowDock::IsBorrowed() const
{
    return s_CurrentlyBorrowed == this;
}

bool ConsoleGL::WindowDock::Borrow() const
{
    if ( s_CurrentlyBorrowed == this )
    {
	    return true;
    }

    ENSURE_LOG( !( s_CurrentlyBorrowed && !s_CurrentlyBorrowed->Return() ), "Failed to return currently borrowed console window." );
	ENSURE_LOG( AttachConsole( m_InternalInfo->ProcessInfo.dwProcessId ), "Failed to attach console window." );

    // Set command.
    CommandBuffer* Buffer = ( CommandBuffer* )m_CommandBuffer.Data();
    Buffer->Command = ECommand::Release;
    
    ENSURE_LOG( m_CommandReady.Trigger(), "Failed to trigger console dock release." );
    ENSURE_LOG( m_CommandComplete.Wait(), "Failed to wait for console dock release." );
    
    s_CurrentlyBorrowed = this;
    return true;
}

bool ConsoleGL::WindowDock::Return() const
{
    if ( s_CurrentlyBorrowed != this )
    {
	    return true;
    }

    CommandBuffer* Buffer = ( CommandBuffer* )m_CommandBuffer.Data();
    Buffer->Command = ECommand::Attach;
    Buffer->Value = ( uint32_t )GetCurrentProcessId();

    ENSURE_LOG( !( !m_CommandReady.Trigger() || !m_CommandComplete.Wait() ), "Failed to trigger console dock attach." );
    ENSURE_LOG( FreeConsole(), "Failed to free console window." );

    s_CurrentlyBorrowed = nullptr;

    return true;
}

ConsoleGL::Window::Window( WindowDock& a_Dock )
    : m_Width()
    , m_Height()
    , m_PixelWidth()
    , m_PixelHeight()
    , m_ActiveBuffer( 0u )
	, m_Dock( &a_Dock )
{}

ConsoleGL::Window::~Window()
{

}

ConsoleGL::Window* ConsoleGL::Window::Create( const std::string& a_Title, uint32_t a_Width, uint32_t a_Height, uint32_t a_PixelWidth, uint32_t a_PixelHeight, uint32_t a_BufferCount )
{
    WindowDock* Dock = WindowDock::Create();

    if ( !Dock )
    {
	    return nullptr;
    }

    // Create new Window and assign it to the dock.
    Window* NewWindow = new Window( *Dock );
    Dock->m_Docked = NewWindow;

    // Temporarily return any currently borrowed window.
    const WindowDock* PreviouslyBorrowed = WindowDock::s_CurrentlyBorrowed;

    // Borrow the new docked window.
	ENSURE_LOG( Dock->Borrow(), "Could not borrow window." );
    Dock->m_InternalInfo->WindowHandle = GetConsoleWindow();

    const HANDLE StdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    const HANDLE StdInputHandle = GetStdHandle( STD_INPUT_HANDLE );

    // Change console visual size to a minimum so ScreenBuffer can shrink
	// below the actual visual size
	SMALL_RECT WindowRect = { 0, 0, 1, 1 };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );

    // Set the size of the screen buffer
	ENSURE_LOG( SetConsoleScreenBufferSize( StdOutputHandle, { ( SHORT )a_Width, ( SHORT )a_Height } ), "Failed to set console screen buffer size." );

    // Assign screen buffer to the console
	ENSURE_LOG( SetConsoleActiveScreenBuffer( StdOutputHandle ), "Failed to set console screen buffer." );

    // Set console font.
    CONSOLE_FONT_INFOEX FontInfo;
    ZeroMemory( &FontInfo, sizeof( FontInfo ) );
    FontInfo.cbSize = sizeof( FontInfo );
    FontInfo.nFont = 0;
    FontInfo.dwFontSize = { ( SHORT )a_PixelWidth, ( SHORT )a_PixelHeight };
    FontInfo.FontFamily = FF_DONTCARE;
    FontInfo.FontWeight = FW_NORMAL;
	wcscpy_s( FontInfo.FaceName, L"Consolas" );
	ENSURE_LOG( SetCurrentConsoleFontEx( StdOutputHandle, false, &FontInfo ), "Failed to set console font." );

    // Get screen buffer info.
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	ENSURE_LOG( GetConsoleScreenBufferInfo( StdOutputHandle, &ScreenBufferInfo ), "Failed to get console screen buffer info." );
	ENSURE_LOG( a_Width <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.X, "Failed to create console window of with width %u.", a_Width );
	ENSURE_LOG( a_Height <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.Y, "Failed to create console window of with height %u.", a_Height );

    // Set Physical Console Window Size
	WindowRect = { 0, 0, ( SHORT )( a_Width - 1 ), ( SHORT )( a_Height - 1 ) };
    Dock->m_InternalInfo->WindowRegion = WindowRect;
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );
    ENSURE_LOG( SetConsoleMode( StdInputHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT ), "Failed to set console mode." );
    ENSURE_LOG( SetConsoleTitleA( a_Title.c_str() ), "Failed to set console title." );
    
    // Set window info.
    NewWindow->m_Title = a_Title;
    NewWindow->m_PixelWidth = a_PixelWidth;
    NewWindow->m_PixelHeight = a_PixelHeight;
    NewWindow->m_Width = a_Width;
    NewWindow->m_Height = a_Height;
    NewWindow->m_Buffers.reserve( a_BufferCount );

    for ( uint32_t i = 0; i < a_BufferCount; ++i )
    {
        NewWindow->m_Buffers.emplace_back( a_Width, a_Height );
    }

    // Return the new console back to its dock.
    ENSURE_LOG( Dock->Return(), "Failed to return docked console window." );

    // If there was a previously borrowed window, re-borrow it.
    if ( PreviouslyBorrowed )
    {
        ( void )PreviouslyBorrowed->Borrow();
    }

    return NewWindow;
}

void ConsoleGL::Window::Destroy( Window* a_Window )
{
    if ( !a_Window )
    {
	    return;
    }

    if ( WindowDock::s_CurrentlyBorrowed == a_Window->m_Dock )
    {
	    ENSURE_LOG( WindowDock::s_CurrentlyBorrowed->Return(), "Failed to return console window." );
    }

    const auto Iter = std::find_if( WindowDock::s_WindowDocks.begin(), WindowDock::s_WindowDocks.end(), [ a_Window ]( const WindowDock& Dock ) { return &Dock == a_Window->m_Dock; } );
    WindowDock::s_WindowDocks.erase( Iter );
}

void ConsoleGL::Window::SetActive( Window* a_ConsoleWindow )
{
    if ( WindowDock::s_CurrentlyBorrowed )
    {
	    ENSURE_LOG( WindowDock::s_CurrentlyBorrowed->Return(), "Failed to return console window." );
    }

    if ( !a_ConsoleWindow )
    {
	    return;
    }

    ENSURE_LOG( a_ConsoleWindow->m_Dock->Borrow(), "Failed to borrow console window." );
}

ConsoleGL::Window* ConsoleGL::Window::GetActive()
{
	if ( WindowDock::s_CurrentlyBorrowed )
	{
		return WindowDock::s_CurrentlyBorrowed->m_Docked;
	}

    return nullptr;
}

void ConsoleGL::Window::SetTitle( const std::string& a_Title )
{
    if ( Window* Active = GetActive() )
    {
	    ENSURE_LOG( SetConsoleTitleA( a_Title.c_str() ), "Failed to set window title." );
		Active->m_Title = a_Title;
    }
}

void ConsoleGL::Window::SetColours( const Colour* a_Colours )
{
    if ( !WindowDock::s_CurrentlyBorrowed || !a_Colours )
    {
        return;
    }

    const HANDLE StdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    if ( StdOutputHandle == INVALID_HANDLE_VALUE )
    {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
    ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
    ENSURE_LOG( GetConsoleScreenBufferInfoEx( StdOutputHandle, &ScreenBufferInfo ), "Failed to get console window screen buffer info." );

    for ( uint8_t i = 0u; i < 16u; ++i )
    {
        ScreenBufferInfo.ColorTable[ i ] =
            ( static_cast< DWORD >( a_Colours[ i ].b ) << 16 ) |
            ( static_cast< DWORD >( a_Colours[ i ].g ) << 8  ) |
            ( static_cast< DWORD >( a_Colours[ i ].r )       ) ;
    }

    // Set the screen buffer info.
    ENSURE_LOG( SetConsoleScreenBufferInfoEx( StdOutputHandle, &ScreenBufferInfo ), "Failed to set console window screen buffer info." );

    // Due to weirdness with console windows, we need to set the rect to 1x1 and then back to full size again to get rid of scrollbars caused by the above call.
	SMALL_RECT WindowRect = { 0, 0, 1, 1 };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );
	WindowRect = { 0, 0, ( SHORT )(  WindowDock::s_CurrentlyBorrowed->m_Docked->m_Width - 1 ), ( SHORT )(  WindowDock::s_CurrentlyBorrowed->m_Docked->m_Height - 1 ) };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );
}

void ConsoleGL::Window::SetColours( EColourSet a_ColourSet )
{
    switch ( a_ColourSet )
    {
    case EColourSet::DEFAULT:
    {
        SetColours( ColourSetDefault );
        break;
    }
    case EColourSet::GREYSCALE:
    {
        SetColours( ColourSetGreyscale );
        break;
    }
    case EColourSet::SEPIA:
    {
        SetColours( ColourSetSepia );
        break;
    }
    }
}

void ConsoleGL::Window::SwapBuffer()
{
    if ( !WindowDock::s_CurrentlyBorrowed )
    {
	    return;
    }

    Window* Active = WindowDock::s_CurrentlyBorrowed->m_Docked;

    const uint32_t IndexToDraw = Active->m_ActiveBuffer;

    if ( Active->m_Buffers.size() > 1 )
    {
        ++Active->m_ActiveBuffer;

        if ( Active->m_ActiveBuffer >= Active->m_Buffers.size() )
        {
            Active->m_ActiveBuffer = 0u;
        }
    }

    WriteConsoleOutput(
        GetStdHandle( STD_OUTPUT_HANDLE ),
        ( CHAR_INFO* )Active->GetBuffer( IndexToDraw )->GetPixels(),
        { ( SHORT )Active->m_Width, ( SHORT )Active->m_Height },
        { 0, 0 },
        &WindowDock::s_CurrentlyBorrowed->m_InternalInfo->WindowRegion );
}

void ConsoleGL::Window::SwapBuffer( const uint32_t a_Index )
{
	if ( !WindowDock::s_CurrentlyBorrowed )
    {
	    return;
    }

    WindowDock::s_CurrentlyBorrowed->m_Docked->m_ActiveBuffer = a_Index;
    SwapBuffer();
}

//void ConsoleGL::PixelBuffer::SetBuffer( const Pixel a_Pixel )
//{
//    SetPixels( 0u, m_Width * m_Height, a_Pixel );
//}
//
//void ConsoleGL::PixelBuffer::SetPixel( const uint32_t a_Index, const Pixel a_Pixel )
//{
//    *( m_Buffers[ m_ActiveBuffer ] + a_Index ) = a_Pixel;
//}
//
//void ConsoleGL::PixelBuffer::SetPixel( const uint32_t a_X, const uint32_t a_Y, const Pixel a_Pixel )
//{
//    SetPixel( a_X + a_Y * m_Width, a_Pixel );
//}
//
//void ConsoleGL::PixelBuffer::SetPixels( const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel )
//{
//    Pixel* Buffer = GetBuffer() + a_Index;
//
//    for ( uint32_t i = 0; i < a_Count; ++i )
//    {
//        Buffer[ i ] = a_Pixel;
//    }
//}
//
//void ConsoleGL::PixelBuffer::SetPixels( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Count, const Pixel a_Pixel )
//{
//    SetPixels( a_X + a_Y * m_Width, a_Count, a_Pixel );
//}
//
//void ConsoleGL::PixelBuffer::DrawLine( const uint32_t a_XBegin, const uint32_t a_XEnd, const uint32_t a_YBegin, const uint32_t a_YEnd, const Pixel a_Pixel )
//{
//
//}
//
//void ConsoleGL::PixelBuffer::DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel )
//{
//    for ( uint32_t y = 0, Start = a_X + a_Y * m_Width; y < a_Height; ++y, Start += m_Width )
//    {
//        SetPixels( Start, a_Width, a_Pixel );
//    }
//}

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
    for ( uint32_t y = 0u; y < m_Height; ++y )
        for ( uint32_t x = 0u; x < m_Width; ++x )
            a_FragmentFn( x, y, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawLine( const uint32_t a_XBegin, const uint32_t a_XEnd, const uint32_t a_YBegin, const uint32_t a_YEnd, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	int32_t x, y;
    int32_t dx = a_XEnd - a_XBegin;
    int32_t dy = a_YEnd - a_YBegin;
    int32_t dx1 = abs( dx );
    int32_t dy1 = abs( dy );
    int32_t px = 2 * dy1 - dx1;
    int32_t py = 2 * dx1 - dy1;
    int32_t xe, ye, i;

	if ( dy1 <= dx1 )
	{
		if ( dx >= 0 )
        { 
            x = a_XBegin; 
            y = a_YBegin; 
            xe = a_XEnd; 
        }
		else
		{ 
            x = a_XEnd; 
            y = a_YEnd; 
            xe = a_XBegin; 
        }

        SetPixel( x, y, a_FragmentFn( x, y, a_FragmentFnPayload ) );
		
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

            SetPixel( x, y, a_FragmentFn( x, y, a_FragmentFnPayload ) );
		}
	}
	else
	{
		if ( dy >= 0 )
		{ 
            x = a_XBegin; 
            y = a_YBegin; 
            ye = a_YEnd; 
        }
		else
		{ 
            x = a_XEnd;
            y = a_YEnd; 
            ye = a_YBegin; 
        }

        SetPixel( x, y, a_FragmentFn( x, y, a_FragmentFnPayload ) );

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
			
            SetPixel( x, y, a_FragmentFn( x, y, a_FragmentFnPayload ) );
		}
	}
}

void ConsoleGL::PixelBuffer::DrawHorizontalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    for ( uint32_t y = a_YBegin; y < a_YBegin + a_Length; ++y )
    {
        a_FragmentFn( a_XBegin, y, a_FragmentFnPayload );
    }
}

void ConsoleGL::PixelBuffer::DrawVerticalLine( const uint32_t a_XBegin, const uint32_t a_YBegin, const uint32_t a_Length, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    for ( uint32_t x = a_XBegin; x < a_XBegin + a_Length; ++x )
    {
        a_FragmentFn( x, a_YBegin, a_FragmentFnPayload );
    }
}

void ConsoleGL::PixelBuffer::DrawTriangle( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    DrawLine( a_X0, a_X1, a_Y0, a_Y1, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( a_X1, a_X2, a_Y1, a_Y2, a_FragmentFn, a_FragmentFnPayload );
    DrawLine( a_X2, a_X0, a_Y2, a_Y0, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::PixelBuffer::DrawTriangleFilled( const uint32_t a_X0, const uint32_t a_X1, const uint32_t a_X2, const uint32_t a_Y0, const uint32_t a_Y1, const uint32_t a_Y2, const FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
    // Using code from: https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
    // All credit goes to author.

    // 28.4 fixed-point coordinates
    const int Y1 = int(16.0f * (float)a_Y0);
    const int Y2 = int(16.0f * (float)a_Y1);
    const int Y3 = int(16.0f * (float)a_Y2);
    const int X1 = int(16.0f * (float)a_X0);
    const int X2 = int(16.0f * (float)a_X1);
    const int X3 = int(16.0f * (float)a_X2);

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

void ConsoleGL::PixelBuffer::DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawRectFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawCircle( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawCircleFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawEllipse( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}

void ConsoleGL::PixelBuffer::DrawEllipseFilled( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_RadiusMinor, const uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload ) {}
