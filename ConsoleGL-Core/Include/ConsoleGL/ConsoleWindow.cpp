#include <ConsoleGL/ConsoleWindow.hpp>

ConsoleGL::Window* ConsoleGL::Window::s_Active = nullptr;

ConsoleGL::Window::Window()
    : m_ConsoleOutputHandle( NULL )
    , m_ConsoleInputHandle( NULL )
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

    auto ProcessCreationEvent = CreateEvent( NULL, false, false, ( L"console_dock_event_" + Title ).c_str() );

    if ( ProcessCreationEvent == NULL )
    {
        SetActive( PreWindow );
        Destroy( NewWindow );
        return nullptr;
    }

    auto ProcessCompletedEvent = CreateEvent( NULL, false, false, ( L"process_completed_" + Title ).c_str() );

    if ( ProcessCompletedEvent == NULL )
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

    // Wait for process to complete creation.
    WaitForSingleObject( ProcessCompletedEvent, -1 );

    // Temporarily attach new console to set it's data.
    AttachConsole( NewWindow->m_ProcessInfo.dwProcessId );
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
    SetConsoleMode( NewWindow->m_ConsoleInputHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT );
    SetTitle( NewWindow, a_Title );

    // Set up buffers.
    NewWindow->m_Buffers.resize( a_BufferCount );

    for ( auto& Buffer : NewWindow->m_Buffers )
    {
        Buffer = new Pixel[ NewWindow->m_Width * NewWindow->m_Height ];
        memset( Buffer, 0, sizeof( Pixel ) * NewWindow->m_Width * NewWindow->m_Height );
    }

    // Return the attached console to the active window.
    FreeConsole();

    if ( PreWindow )
    {
        AttachConsole( PreWindow->m_ConsoleProcessID );
    }

    return NewWindow;
}

void ConsoleGL::Window::Destroy()
{
    if ( !s_Active )
    {
        return;
    }

    Window* PreWindow = s_Active;
    SetActive( nullptr );
    TerminateProcess( PreWindow->m_ProcessInfo.hProcess, 0 );
    CloseHandle( PreWindow->m_ProcessInfo.hThread );
    CloseHandle( PreWindow->m_ProcessInfo.hProcess );
    delete PreWindow;
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

    TerminateProcess( a_Window->m_ProcessInfo.hProcess, 0 );
    CloseHandle( a_Window->m_ProcessInfo.hThread );
    CloseHandle( a_Window->m_ProcessInfo.hProcess );
    delete a_Window;
}

void ConsoleGL::Window::SetActive( Window* a_ConsoleWindow )
{
    if ( s_Active )
    {
        fclose( stdout );
        FreeConsole();
    }

    s_Active = a_ConsoleWindow;

    if ( s_Active )
    {
        AttachConsole( s_Active->m_ConsoleProcessID );
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
}

void ConsoleGL::Window::SetTitle( Window* a_Window, const std::string& a_Title )
{
    if ( !a_Window )
    {
        return;
    }

    Window* PreWindow = s_Active;
    FreeConsole();
    AttachConsole( a_Window->m_ConsoleProcessID );
    SetConsoleTitle( std::wstring( a_Title.begin(), a_Title.end() ).c_str() );
    FreeConsole();

    if ( PreWindow )
    {
        AttachConsole( PreWindow->m_ConsoleProcessID );
    }
}

void ConsoleGL::Window::SwapBuffer()
{
    if ( s_Active->m_Buffers.size() == 0 )
    {
        return;
    }

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

void ConsoleGL::Window::SetBuffer( Pixel a_Pixel )
{
    SetPixels( 0u, s_Active->m_Width * s_Active->m_Height, a_Pixel );
}

void ConsoleGL::Window::SetRect( uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel )
{
    for ( uint32_t y = 0, Start = a_X + a_Y * s_Active->m_Width; y < a_Height; ++y, Start += s_Active->m_Width )
    {
        SetPixels( Start, a_Width, a_Pixel );
    }
}

void ConsoleGL::Window::SetPixel( uint32_t a_Index, Pixel a_Pixel )
{
    *( s_Active->m_Buffers[ s_Active->m_ActiveBuffer ] + a_Index ) = a_Pixel;
}

void ConsoleGL::Window::SetPixel( uint32_t a_X, uint32_t a_Y, Pixel a_Pixel )
{
    SetPixel( a_X + a_Y * s_Active->m_Width, a_Pixel );
}

void ConsoleGL::Window::SetPixels( uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel )
{
    Pixel* Buffer = s_Active->m_Buffers[ s_Active->m_ActiveBuffer ] + a_Index;

    for ( int i = 0; i < a_Count; ++i )
    {
        Buffer[ i ] = a_Pixel;
    }
}

void ConsoleGL::Window::SetPixels( uint32_t a_X, uint32_t a_Y, uint32_t a_Count, Pixel a_Pixel )
{
    SetPixels( a_X + a_Y * s_Active->m_Width, a_Count, a_Pixel );
}