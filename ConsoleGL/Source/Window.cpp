#include <chrono>
#define UNICODE
#include <Window.hpp>
#include <Colour.hpp>
#include <Pixel.hpp>

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


const ConsoleGL::WindowDock*              ConsoleGL::WindowDock::s_CurrentlyBorrowed;
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

    // This should be replaced with a Process object.
    if ( !CreateProcessA(
        nullptr,
        ( "../ConsoleGL-ConsoleDock/ConsoleGL-ConsoleDock.exe " + Prefix ).data(),
        nullptr,
        nullptr,
        true,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &m_InternalInfo->StartupInfo,
        &m_InternalInfo->ProcessInfo ) )
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

ConsoleGL::WindowDock::~WindowDock()
{
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
    for ( const Pixel* Buffer : m_Buffers )
    {
        delete[] Buffer;
    }
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
    //Dock->m_InternalInfo->ConsoleOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    //Dock->m_InternalInfo->ConsoleInputHandle = GetStdHandle( STD_INPUT_HANDLE );
    Dock->m_InternalInfo->WindowHandle = GetConsoleWindow();

    // Set window attributes.
    SetWindowLong( Dock->m_InternalInfo->WindowHandle, GWL_STYLE, WS_CAPTION | DS_MODALFRAME | WS_MINIMIZEBOX | WS_SYSMENU );
    SetWindowPos( Dock->m_InternalInfo->WindowHandle, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW );
    SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT );
    SetConsoleTitleA( a_Title.c_str() );
    NewWindow->m_Title = a_Title;

    // Change console visual size to a minimum so ScreenBuffer can shrink
	// below the actual visual size
	SMALL_RECT WindowRect = { 0, 0, 1, 1 };
	SetConsoleWindowInfo( GetStdHandle( STD_OUTPUT_HANDLE ), true, &WindowRect );

    // Set the size of the screen buffer
	ENSURE_LOG( SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), { ( SHORT )a_Width, ( SHORT )a_Height } ), "Failed to set console screen buffer size." );

	// Assign screen buffer to the console
	ENSURE_LOG( SetConsoleActiveScreenBuffer( GetStdHandle( STD_OUTPUT_HANDLE ) ), "Failed to set console screen buffer." );

    // Set console font.
    CONSOLE_FONT_INFOEX FontInfo;
    ZeroMemory( &FontInfo, sizeof( FontInfo ) );
    FontInfo.cbSize = sizeof( FontInfo );
    FontInfo.nFont = 0;
    FontInfo.dwFontSize = { ( SHORT )a_PixelWidth, ( SHORT )a_PixelHeight };
    FontInfo.FontFamily = FF_DONTCARE;
    FontInfo.FontWeight = FW_NORMAL;
	wcscpy_s( FontInfo.FaceName, L"Consolas" );
	ENSURE_LOG( SetCurrentConsoleFontEx( GetStdHandle( STD_OUTPUT_HANDLE ), false, &FontInfo ), "Failed to set console font." );

    NewWindow->m_PixelWidth = a_PixelWidth;
    NewWindow->m_PixelHeight = a_PixelHeight;

    // Get screen buffer info and check the maximum allowed window size. Return
	// error if exceeded, so user knows their dimensions/fontsize are too large
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	ENSURE_LOG( GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &ScreenBufferInfo ), "Failed to get console screen buffer info." );
	ENSURE_LOG( a_Width <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.X, "Failed to create console window of with width %u.", a_Width );
	ENSURE_LOG( a_Height <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.Y, "Failed to create console window of with height %u.", a_Height );

    NewWindow->m_Width = a_Width;
    NewWindow->m_Height = a_Height;

	// Set Physical Console Window Size
	WindowRect = { 0, 0, ( SHORT )( NewWindow->m_Width - 1 ), ( SHORT )( NewWindow->m_Height - 1 ) };
    Dock->m_InternalInfo->WindowRegion = WindowRect;
	ENSURE_LOG( SetConsoleWindowInfo( GetStdHandle( STD_OUTPUT_HANDLE ), true, &WindowRect ), "Failed to set console window info." );

    // Set up buffers.
    NewWindow->m_Buffers.resize( a_BufferCount );

    for ( auto& Buffer : NewWindow->m_Buffers )
    {
        Buffer = new Pixel[ NewWindow->m_Width * NewWindow->m_Height ];
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

    auto Iter = std::find_if( WindowDock::s_WindowDocks.begin(), WindowDock::s_WindowDocks.end(), [ a_Window ]( const WindowDock& Dock ) { return &Dock == a_Window->m_Dock; } );
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

    const HANDLE OutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    if ( OutputHandle == INVALID_HANDLE_VALUE )
    {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
    ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
    ENSURE_LOG( GetConsoleScreenBufferInfoEx( OutputHandle, &ScreenBufferInfo ), "Failed to get console window screen buffer info." );

    for ( uint8_t i = 0u; i < 16u; ++i )
    {
        ScreenBufferInfo.ColorTable[ i ] =
            ( static_cast< DWORD >( a_Colours[ i ].b ) << 16 ) |
            ( static_cast< DWORD >( a_Colours[ i ].g ) << 8  ) |
            ( static_cast< DWORD >( a_Colours[ i ].r )       ) ;
    }

    ENSURE_LOG( SetConsoleScreenBufferInfoEx( OutputHandle, &ScreenBufferInfo), "Failed to set console window screen buffer info." );
}

void ConsoleGL::Window::SetColours( EColourSet a_ColourSet )
{
    switch ( a_ColourSet )
    {
    case ConsoleGL::EColourSet::DEFAULT:
    {
        SetColours( ColourSetDefault );
        break;
    }
    case ConsoleGL::EColourSet::NEW:
    {
        TODO( "Add New colour set." );
        SetColours( ColourSetDefault );
        break;
    }
    case ConsoleGL::EColourSet::LEGACY:
    {
        TODO( "Add Legacy colour set." );
        SetColours( ColourSetDefault );
        break;
    }
    case ConsoleGL::EColourSet::GREYSCALE:
    {
        SetColours( ColourSetGreyscale );
        break;
    }
    case ConsoleGL::EColourSet::SEPIA:
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
        ( CHAR_INFO* )Active->m_Buffers[ IndexToDraw ],
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

void ConsoleGL::Window::SetBuffer( const Pixel a_Pixel )
{
    SetPixels( 0u, m_Width * m_Height, a_Pixel );
}

void ConsoleGL::Window::SetRect( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const Pixel a_Pixel )
{
    for ( uint32_t y = 0, Start = a_X + a_Y * m_Width; y < a_Height; ++y, Start += m_Width )
    {
        SetPixels( Start, a_Width, a_Pixel );
    }
}

void ConsoleGL::Window::SetPixel( const uint32_t a_Index, const Pixel a_Pixel )
{
    *( m_Buffers[ m_ActiveBuffer ] + a_Index ) = a_Pixel;
}

void ConsoleGL::Window::SetPixel( const uint32_t a_X, const uint32_t a_Y, const Pixel a_Pixel )
{
    SetPixel( a_X + a_Y * m_Width, a_Pixel );
}

void ConsoleGL::Window::SetPixels( const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel )
{
    Pixel* Buffer = GetBuffer() + a_Index;

    for ( uint32_t i = 0; i < a_Count; ++i )
    {
        Buffer[ i ] = a_Pixel;
    }
}

void ConsoleGL::Window::SetPixels( const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Count, const Pixel a_Pixel )
{
    SetPixels( a_X + a_Y * m_Width, a_Count, a_Pixel );
}