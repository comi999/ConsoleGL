#include <chrono>

#include <Window.hpp>
#include <Error.hpp>

// Disable dll exported functions.
#define _USER32_
#include <Windows.h>

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

#if IS_CONSOLEGL
#include <ConsoleDock.inl>
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
        Error::SetLastError( Error_WindowDockCreationFailure );
        return;
    }

    // Create the command ready event.
    if ( !m_CommandReady.Create( Prefix + "_CommandReady", true ) )
    {
        m_CommandBuffer.Clear();
        Error::SetLastError( Error_WindowDockCreationFailure );
        return;
    }
    
    // Create the command complete event.
    if ( !m_CommandComplete.Create( Prefix + "_CommandComplete", true ) )
    {
        m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        Error::SetLastError( Error_WindowDockCreationFailure );
        return;
    }

    if ( !m_ProcessStarted.Create( Prefix + "_ProcessStarted" ) )
    {
	    m_CommandBuffer.Clear();
        m_CommandReady.Clear();
        m_CommandComplete.Clear();
        Error::SetLastError( Error_WindowDockCreationFailure );
        return;
    }

    m_InternalInfo = std::make_shared< InternalInfo >();
    ZeroMemory( &m_InternalInfo->StartupInfo, sizeof( m_InternalInfo->StartupInfo ) );
    m_InternalInfo->StartupInfo.cb = sizeof( m_InternalInfo->StartupInfo );
    ZeroMemory( &m_InternalInfo->ProcessInfo, sizeof( m_InternalInfo->ProcessInfo ) );

#if IS_CONSOLEGL
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
#if IS_CONSOLEGL
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
        Error::SetLastError( Error_WindowDockCreationFailure );
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
        Error::SetLastError( Error_WindowDockCreationFailure );
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

void ConsoleGL::Window::SetActive( Window* a_Window )
{
    if ( WindowDock::s_CurrentlyBorrowed )
    {
	    ENSURE_LOG( WindowDock::s_CurrentlyBorrowed->Return(), "Failed to return console window." );
    }

    if ( !a_Window )
    {
	    return;
    }

    ENSURE_LOG( a_Window->m_Dock->Borrow(), "Failed to borrow console window." );
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

	Error::SetLastError( Error_NoActiveWindow );
}

void ConsoleGL::Window::SetColours( const Colour* a_Colours )
{
    if ( !WindowDock::s_CurrentlyBorrowed )
    {
        Error::SetLastError( Error_NoActiveWindow );
        return;
    }

    if ( !a_Colours )
    {
        
	    return;
    }

    const HANDLE StdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    if ( StdOutputHandle == INVALID_HANDLE_VALUE )
    {
		Error::SetLastError( Error_WindowFailure );
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

void ConsoleGL::Window::SetColours( const EColourSet a_ColourSet )
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
    	Error::SetLastError( Error_NoActiveWindow );
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
    	Error::SetLastError( Error_NoActiveWindow );
	    return;
    }

    WindowDock::s_CurrentlyBorrowed->m_Docked->m_ActiveBuffer = a_Index;
    SwapBuffer();
}

bool ConsoleGL::Window::HasFocus() const
{
	return ::GetActiveWindow() == m_Dock->m_InternalInfo->WindowHandle;
}

uint8_t KeyCodes[ 99 ] = 
{
	0x08,
	0x09,
	0x0D,
	0x10,
	0x11,
	0x12,
	0x14,
	0x1B,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	0x27,
	0x28,
	0x2C,
	0x2D,
	0x2E,
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49,
	0x4A,
	0x4B,
	0x4C,
	0x4D,
	0x4E,
	0x4F,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5A,
	0x70,
	0x71,
	0x72,
	0x73,
	0x74,
	0x75,
	0x76,
	0x77,
	0x78,
	0x79,
	0x7A,
	0x7B,
	0x7C,
	0x7D,
	0x7E,
	0x7F,
	0x80,
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x87,
	0x90,
	0x91,
	0xA0,
	0xA1,
	0xA2,
	0xA3,
	0xA4,
	0xA5,
	0xBA,
	0xBB,
	0xBC,
	0xBD,
	0xBE,
	0xBF,
	0xC0,
	0xDB,
	0xDC,
	0xDD,
	0xDE
};

uint8_t MouseCodes[ 3 ] =
{
	0x01,
	0x02,
	0x04
};


std::bitset< 99 >	ConsoleGL::Window::s_KeyStates;
std::bitset< 3  >	ConsoleGL::Window::s_MouseStates;
float				ConsoleGL::Window::s_MouseX;
float				ConsoleGL::Window::s_MouseY;
float				ConsoleGL::Window::s_MouseDeltaX;
float				ConsoleGL::Window::s_MouseDeltaY;

bool ConsoleGL::Window::IsKeyDown( const EKeyboardKey a_KeyboardKey )
{
	return GetKeyState( KeyCodes[ static_cast< uint8_t >( a_KeyboardKey ) ] ) & 0x8000;
}

bool ConsoleGL::Window::IsKeyUp( const EKeyboardKey a_KeyboardKey )
{
	return !( GetKeyState( KeyCodes[ static_cast< uint8_t >( a_KeyboardKey ) ] ) & 0x8000 );
}

bool ConsoleGL::Window::IsKeyPressed( const EKeyboardKey a_KeyboardKey )
{
	return !s_KeyStates[ static_cast< uint8_t >( a_KeyboardKey ) ] && IsKeyDown( a_KeyboardKey );
}

bool ConsoleGL::Window::IsKeyReleased( const EKeyboardKey a_KeyboardKey )
{
	return s_KeyStates[ static_cast< uint8_t >( a_KeyboardKey ) ] && IsKeyUp( a_KeyboardKey );
}

bool ConsoleGL::Window::IsMouseDown( const EMouseButton a_MouseButton )
{
	return GetKeyState( MouseCodes[ static_cast< uint8_t >( a_MouseButton ) ] ) & 0x8000;
}

bool ConsoleGL::Window::IsMouseUp( const EMouseButton a_MouseButton )
{
	return !( GetKeyState( MouseCodes[ static_cast< uint8_t >( a_MouseButton ) ] ) & 0x8000 );
}

bool ConsoleGL::Window::IsMousePressed( const EMouseButton a_MouseButton )
{
	return !s_MouseStates[ static_cast< uint8_t >( a_MouseButton ) ] && IsMouseDown( a_MouseButton );
}

bool ConsoleGL::Window::IsMouseReleased( const EMouseButton a_MouseButton )
{
	return s_MouseStates[ static_cast< uint8_t >( a_MouseButton ) ] && IsMouseUp( a_MouseButton );
}

void ConsoleGL::Window::GetMousePosition( float& o_X, float& o_Y )
{
	o_X = s_MouseX;
	o_Y = s_MouseY;
}

void ConsoleGL::Window::GetMouseDelta( float& o_X, float& o_Y )
{
	o_X = s_MouseDeltaX;
	o_Y = s_MouseDeltaY;
}

void ConsoleGL::Window::PollEvents()
{
	const Window* Active = GetActive();

    if ( !Active )
    {
	    Error::SetLastError( Error_NoActiveWindow );
        return;
    }

	for ( size_t i = 0u; i < s_KeyStates.size(); ++i )
	{
		s_KeyStates[ i ] = GetKeyState( KeyCodes[ i ] ) & 0x8000;
	}

	for ( size_t i = 0u; i < s_MouseStates.size(); ++i )
	{
		s_MouseStates[ i ] = GetKeyState( MouseCodes[ i ] ) & 0x8000;
	}

    // Get the current mouse position.
    POINT Coordinates{ 0, 0 };

	if ( !GetCursorPos( &Coordinates ) || !ScreenToClient( Active->m_Dock->m_InternalInfo->WindowHandle, &Coordinates ) )
	{
		s_MouseX = 0.0f;
		s_MouseY = 0.0f;
        s_MouseDeltaX = 0.0f;
        s_MouseDeltaY = 0.0f;
	    Error::SetLastError( Error_WindowFailure );
		return;
	}

    float NormalizedX = ( float )Coordinates.x;
	float NormalizedY = ( float )Coordinates.y;

    // Normalize coordinates.
    NormalizedX /= Active->m_Width * Active->m_PixelWidth;
    NormalizedY /= Active->m_Height * Active->m_PixelHeight;

    // Set delta.
    s_MouseDeltaX = NormalizedX - s_MouseX;
    s_MouseDeltaY = NormalizedY - s_MouseY;

    // Set new position.
    s_MouseX = NormalizedX;
    s_MouseY = NormalizedY;
}
