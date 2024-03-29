#include <ConsoleGL/Window.hpp>

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

ConsoleGL::Window* ConsoleGL::Window::s_Active = nullptr;

ConsoleGL::Window::Window()
    : m_ConsoleOutputHandle( NULL )
    , m_ConsoleInputHandle( NULL )
    , m_CommandReady( NULL )
    , m_CommandComplete( NULL )
    , m_CommandBufferHandle( NULL )
    , m_CommandBuffer( nullptr )
    , m_WindowHandle( NULL )
    , m_WindowRegion()
    , m_StartupInfo()
    , m_ProcessInfo()
    , m_ConsoleProcessID()
    , m_Width()
    , m_Height()
    , m_PixelWidth()
    , m_PixelHeight()
    , m_Buffers()
    , m_ActiveBuffer( 0 )
    , m_Title()
{}

ConsoleGL::Window::~Window()
{
    for ( auto Buffer : m_Buffers )
    {
        delete[] Buffer;
    }
}

ConsoleGL::Window* ConsoleGL::Window::Create( const std::string& a_Title, uint32_t a_Width, uint32_t a_Height, uint32_t a_PixelWidth, uint32_t a_PixelHeight, uint32_t a_BufferCount )
{
    Window* NewWindow = new Window();
    Window* PreWindow = s_Active;

    // Start the console dock process.
    ZeroMemory( &NewWindow->m_StartupInfo, sizeof( NewWindow->m_StartupInfo ) );
    NewWindow->m_StartupInfo.cb = sizeof( NewWindow->m_StartupInfo );
    ZeroMemory( &NewWindow->m_ProcessInfo, sizeof( NewWindow->m_ProcessInfo ) );
    NewWindow->m_Title = a_Title;
    std::wstring Title( a_Title.begin(), a_Title.end() );

    NewWindow->m_CommandBufferHandle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof( WindowCommandBuffer ),
        ( L"command_buffer_" + Title ).c_str() );

    if ( NewWindow->m_CommandBufferHandle == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    NewWindow->m_CommandBuffer = ( LPTSTR )MapViewOfFile(
        NewWindow->m_CommandBufferHandle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof( WindowCommandBuffer ) );

    if ( NewWindow->m_CommandBuffer == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    CreateProcess(
        CONSOLE_DOCK_PROCESS_PATH,
        Title.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &NewWindow->m_StartupInfo,
        &NewWindow->m_ProcessInfo );

    auto ProcessStartedEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"process_started_" + Title ).c_str() );

    if ( ProcessStartedEvent == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    // Wait for process to complete creation.
    WaitForSingleObject( ProcessStartedEvent, -1 );

    NewWindow->m_CommandReady = CreateEvent(
        NULL,
        false,
        false,
        ( L"command_ready_" + Title ).c_str() );

    if ( NewWindow->m_CommandReady == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    NewWindow->m_CommandComplete = CreateEvent(
        NULL,
        false,
        false,
        ( L"command_complete_" + Title ).c_str() );

    if ( NewWindow->m_CommandComplete == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    // Temporarily attach new console to set it's data.
    if ( PreWindow )
    {
        auto CommandBuffer = ( WindowCommandBuffer* )PreWindow->m_CommandBuffer;
        CommandBuffer->Command = EWindowCommand::Attach;
        CommandBuffer->Value = PreWindow->m_ConsoleProcessID;
        SetEvent( PreWindow->m_CommandReady );
        WaitForSingleObject( PreWindow->m_CommandComplete, -1 );
        FreeConsole();
    }

    AttachConsole( NewWindow->m_ProcessInfo.dwProcessId );
    auto CommandBuffer = ( WindowCommandBuffer* )NewWindow->m_CommandBuffer;
    CommandBuffer->Command = EWindowCommand::Release;
    SetEvent( NewWindow->m_CommandReady );
    WaitForSingleObject( NewWindow->m_CommandComplete, -1 );
    NewWindow->m_ConsoleProcessID = NewWindow->m_ProcessInfo.dwProcessId;
    NewWindow->m_ConsoleOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    NewWindow->m_ConsoleInputHandle = GetStdHandle( STD_INPUT_HANDLE );
    NewWindow->m_WindowHandle = GetConsoleWindow();

    // Set console font.
    CONSOLE_FONT_INFOEX FontInfo;
    FontInfo.cbSize = sizeof( FontInfo );
    FontInfo.nFont = 0;
    FontInfo.dwFontSize = { ( short )a_PixelWidth, ( short )a_PixelHeight };
    FontInfo.FontFamily = FF_DONTCARE;
    FontInfo.FontWeight = FW_NORMAL;
    wcscpy_s( FontInfo.FaceName, L"Terminal" );
    SetCurrentConsoleFontEx( NewWindow->m_ConsoleOutputHandle, false, &FontInfo );
    NewWindow->m_PixelWidth = a_PixelWidth;
    NewWindow->m_PixelHeight = a_PixelHeight;

    // Get screen buffer info object.
    CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
    ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
    GetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleOutputHandle, &ScreenBufferInfo );
    //// set colours here
    SetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleOutputHandle, &ScreenBufferInfo );

    // Get largest possible window size that can fit on screen.
    COORD LargestWindow = GetLargestConsoleWindowSize( NewWindow->m_ConsoleOutputHandle );

    // If smaller than requested size, destroy window and exit.
    if ( LargestWindow.X < a_Width ||
        LargestWindow.Y < a_Height )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    // Set width and height.
    NewWindow->m_Width = a_Width;
    NewWindow->m_Height = a_Height;

    // Set window region rect.
    NewWindow->m_WindowRegion.Left = 0;
    NewWindow->m_WindowRegion.Top = 0;
    NewWindow->m_WindowRegion.Right = a_Width - 1;
    NewWindow->m_WindowRegion.Bottom = a_Height - 1;

    // Set console attributes.
    SetConsoleScreenBufferSize( NewWindow->m_ConsoleOutputHandle, LargestWindow );
    SetConsoleWindowInfo( NewWindow->m_ConsoleOutputHandle, true, &NewWindow->m_WindowRegion );
    GetConsoleScreenBufferInfoEx( NewWindow->m_ConsoleOutputHandle, &ScreenBufferInfo );
    SetConsoleScreenBufferSize( NewWindow->m_ConsoleOutputHandle, { ( short )a_Width, ( short )a_Height } );

    // Set cursor attributes.
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo( NewWindow->m_ConsoleOutputHandle, &CursorInfo );
    CursorInfo.bVisible = false;
    SetConsoleCursorInfo( NewWindow->m_ConsoleOutputHandle, &CursorInfo );

    // Set window attributes.
    SetWindowLong( NewWindow->m_WindowHandle, GWL_STYLE, WS_CAPTION | DS_MODALFRAME | WS_MINIMIZEBOX | WS_SYSMENU );
    SetWindowPos( NewWindow->m_WindowHandle, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW );
    SetConsoleMode( NewWindow->m_ConsoleInputHandle, /*ENABLE_EXTENDED_FLAGS |*/ ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT );
    SetConsoleTitle( Title.c_str() );

    // Set up buffers.
    NewWindow->m_Buffers.resize( a_BufferCount );

    for ( auto& Buffer : NewWindow->m_Buffers )
    {
        Buffer = new Pixel[ NewWindow->m_Width * NewWindow->m_Height ];
    }

    // Return the attached console to the active window.
    CommandBuffer = ( WindowCommandBuffer* )NewWindow->m_CommandBuffer;
    CommandBuffer->Command = EWindowCommand::Attach;
    CommandBuffer->Value = GetCurrentProcessId();  // NewWindow->m_ConsoleProcessID;
    SetEvent( NewWindow->m_CommandReady );
    //WaitForSingleObject( NewWindow->m_CommandComplete, -1 );
    FreeConsole();

    if ( PreWindow )
    {
        AttachConsole( PreWindow->m_ConsoleProcessID );
        CommandBuffer = ( WindowCommandBuffer* )PreWindow->m_CommandBuffer;
        CommandBuffer->Command = EWindowCommand::Release;
        SetEvent( PreWindow->m_CommandReady );
        WaitForSingleObject( PreWindow->m_CommandComplete, -1 );
    }

    return NewWindow;
}

void ConsoleGL::Window::Destroy( Window* a_Window )
{
    if ( !a_Window )
    {
        return;
    }

    if ( s_Active == a_Window )
    {
        SetActive( nullptr );
    }

    auto CommandBuffer = ( WindowCommandBuffer* )a_Window->m_CommandBuffer;
    CommandBuffer->Command = EWindowCommand::Exit;
    SetEvent( a_Window->m_CommandReady );
    TerminateProcess( a_Window->m_ProcessInfo.hProcess, 0 );
    CloseHandle( a_Window->m_ProcessInfo.hThread );
    CloseHandle( a_Window->m_ProcessInfo.hProcess );
    delete a_Window;
}

void ConsoleGL::Window::SetActive( Window* a_ConsoleWindow )
{
    if ( s_Active )
    {
        // Reattach the console to the window dock.
        auto CommandBuffer = ( WindowCommandBuffer* )s_Active->m_CommandBuffer;
        CommandBuffer->Command = EWindowCommand::Attach;
        CommandBuffer->Value = s_Active->m_ConsoleProcessID;
        SetEvent( s_Active->m_CommandReady );
        WaitForSingleObject( s_Active->m_CommandComplete, -1 );
        fclose( stdout );
        FreeConsole();
    }

    s_Active = a_ConsoleWindow;

    if ( s_Active )
    {
        AttachConsole( s_Active->m_ConsoleProcessID );
        auto CommandBuffer = ( WindowCommandBuffer* )s_Active->m_CommandBuffer;
        CommandBuffer->Command = EWindowCommand::Release;
        SetEvent( s_Active->m_CommandReady );
        WaitForSingleObject( s_Active->m_CommandComplete, -1 );
        freopen_s( ( FILE** )stdout, "CONOUT$", "w", stdout );
    }
}

void ConsoleGL::Window::SetTitle( const std::string& a_Title )
{
    if ( !s_Active )
    {
        return;
    }

    SetConsoleTitle( std::wstring( a_Title.begin(), a_Title.end() ).c_str() );
    s_Active->m_Title = a_Title;
}

void ConsoleGL::Window::SetColours( const ConsoleGL::Colour* a_Colours )
{
    if ( !s_Active || !a_Colours )
    {
        return;
    }

    auto OutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    if ( OutputHandle == INVALID_HANDLE_VALUE )
    {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
    ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
    GetConsoleScreenBufferInfoEx( OutputHandle, &ScreenBufferInfo );

    for ( int i = 0; i < 16; ++i )
    {
        ScreenBufferInfo.ColorTable[ i ] =
            ( static_cast< uint32_t >( a_Colours[ i ].b ) << 16 ) |
            ( static_cast< uint32_t >( a_Colours[ i ].g ) << 8  ) |
            ( static_cast< uint32_t >( a_Colours[ i ].r ) );
    }

    SetConsoleScreenBufferInfoEx( OutputHandle, &ScreenBufferInfo);
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
    uint32_t IndexToDraw = s_Active->m_ActiveBuffer;

    if ( s_Active->m_Buffers.size() > 1 )
    {
        ++s_Active->m_ActiveBuffer;

        if ( !( s_Active->m_ActiveBuffer < s_Active->m_Buffers.size() ) )
        {
            s_Active->m_ActiveBuffer = 0u;
        }
    }

    WriteConsoleOutput(
        GetStdHandle( STD_OUTPUT_HANDLE ),
        ( CHAR_INFO* )s_Active->m_Buffers[ IndexToDraw ],
        { ( short )s_Active->m_Width, ( short )s_Active->m_Height },
        { 0, 0 },
        &s_Active->m_WindowRegion );
}

void ConsoleGL::Window::SwapBuffer( uint32_t a_Index )
{
    s_Active->m_ActiveBuffer = a_Index;
    SwapBuffer();
}

void ConsoleGL::Window::SetBuffer( Pixel a_Pixel )
{
    SetPixels( 0u, m_Width * m_Height, a_Pixel );
}

void ConsoleGL::Window::SetRect( uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel )
{
    for ( uint32_t y = 0, Start = a_X + a_Y * m_Width; y < a_Height; ++y, Start += m_Width )
    {
        SetPixels( Start, a_Width, a_Pixel );
    }
}

void ConsoleGL::Window::SetPixel( uint32_t a_Index, Pixel a_Pixel )
{
    *( m_Buffers[ m_ActiveBuffer ] + a_Index ) = a_Pixel;
}

void ConsoleGL::Window::SetPixel( uint32_t a_X, uint32_t a_Y, Pixel a_Pixel )
{
    SetPixel( a_X + a_Y * m_Width, a_Pixel );
}

void ConsoleGL::Window::SetPixels( uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel )
{
    Pixel* Buffer = GetBuffer() + a_Index;

    for ( int i = 0; i < a_Count; ++i )
    {
        Buffer[ i ] = a_Pixel;
    }
}

void ConsoleGL::Window::SetPixels( uint32_t a_X, uint32_t a_Y, uint32_t a_Count, Pixel a_Pixel )
{
    SetPixels( a_X + a_Y * m_Width, a_Count, a_Pixel );
}