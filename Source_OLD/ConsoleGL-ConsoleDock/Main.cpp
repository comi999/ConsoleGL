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

    const HANDLE MapFileHandle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof( WindowCommandBuffer ),
        ( L"command_buffer_" + Title ).c_str() );

    if ( MapFileHandle == nullptr )
    {
        return 1;
    }

    const LPCTSTR CommandBuffer = ( LPTSTR )MapViewOfFile(
        MapFileHandle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof( WindowCommandBuffer ) );

    if ( CommandBuffer == nullptr )
    {
        CloseHandle( MapFileHandle );
        return 1;
    }

    const HANDLE CommandReadyEvent = CreateEvent(
        nullptr,
        false,
        false,
        ( L"command_ready_" + Title ).c_str() );

    if ( CommandReadyEvent == nullptr )
    {
        return 1;
    }

    const HANDLE CommandCompleteEvent = CreateEvent(
        nullptr,
        false,
        false,
        ( L"command_complete_" + Title ).c_str() );

    if ( CommandCompleteEvent == nullptr )
    {
        return 1;
    }

    const HANDLE ProcessStartedEvent = CreateEvent(
        nullptr,
        false,
        false,
        ( L"process_started_" + Title ).c_str() );

    if ( ProcessStartedEvent == nullptr )
    {
        return 1;
    }

    SetEvent( ProcessStartedEvent );

    bool Running = true;

    while ( Running )
    {
        ( void )WaitForSingleObject( CommandReadyEvent, -1 );

        const auto& [ Command, Value ] = *( WindowCommandBuffer* )CommandBuffer;

        switch ( Command )
        {
        case EWindowCommand::Release:
        {
            FreeConsole();
            break;
        }
        case EWindowCommand::Attach:
        {
            AttachConsole( Value );
            break;
        }
        case EWindowCommand::Exit:
        {
            Running = false;
            break;
        }
        default:
            break;
        }

        SetEvent( CommandCompleteEvent );
    }
    
    CloseHandle( MapFileHandle );
    UnmapViewOfFile( CommandBuffer );

    return 0;
}