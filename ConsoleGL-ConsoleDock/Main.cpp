#include <iostream>
#include <thread>
#include <string>
//#include <stdio.h>
//#include <conio.h>
#include <Windows.h>

int main( int a_ArgCount, const char** a_Args )
{
    enum class EWindowCommand
    {
        Release,
        Attach,
        Exit
    };

    struct WindowCommandBuffer
    {
        EWindowCommand Command;
        uint32_t Value;
    };

    std::wstring Title;

    {
        std::string Temp = a_Args[ 0 ];
        Title = std::wstring( Temp.begin(), Temp.end() );
    }

    HANDLE MapFileHandle;
    LPCTSTR CommandBuffer;

    MapFileHandle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof( WindowCommandBuffer ),
        ( L"command_buffer_" + Title ).c_str() );

    if ( MapFileHandle == NULL )
    {
        return 1;
    }

    CommandBuffer = ( LPTSTR )MapViewOfFile(
        MapFileHandle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof( WindowCommandBuffer ) );

    if ( CommandBuffer == NULL )
    {
        CloseHandle( MapFileHandle );
        return 1;
    }

    auto CommandReadyEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"console_dock_command_ready_" + Title ).c_str() );

    if ( CommandReadyEvent == NULL )
    {
        return 1;
    }

    auto CommandCompleteEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"console_dock_command_complete_" + Title ).c_str() );

    if ( CommandCompleteEvent == NULL )
    {
        return 1;
    }

    auto ProcessStartedEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"process_started_" + Title ).c_str() );

    if ( ProcessStartedEvent == NULL )
    {
        return 1;
    }

    SetEvent( ProcessStartedEvent );

    bool Running = true;

    while ( Running )
    {
        ( void )WaitForSingleObject( CommandReadyEvent, -1 );

        WindowCommandBuffer& Buffer = *( WindowCommandBuffer* )CommandBuffer;

        switch ( Buffer.Command )
        {
        case EWindowCommand::Release:
        {
            break;
        }
        case EWindowCommand::Attach:
        {
            break;
        }
        case EWindowCommand::Exit:
        {
            break;
        }
        default:
            break;
        }
    }

    /*auto IndefiniteWaitEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"console_dock_event_" + Title ).c_str()
    );*/



    

    

    CloseHandle( MapFileHandle );
    UnmapViewOfFile( CommandBuffer );

    return 0;
}