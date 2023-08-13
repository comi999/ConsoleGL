#include <iostream>
#include <thread>
#include <string>
#include <Windows.h>

int main( int a_ArgCount, const char** a_Args )
{
    std::wstring Title;

    {
        std::string Temp = a_Args[ 0 ];
        Title = std::wstring( Temp.begin(), Temp.end() );
    }

    auto IndefiniteWaitEvent = CreateEvent(
        NULL,
        false,
        false,
        ( L"console_dock_event_" + Title ).c_str()
    );

    if ( IndefiniteWaitEvent == NULL )
    {
        return 1;
    }

    auto ProcessCompletedEvent = CreateEvent( NULL, false, false, ( L"process_completed_" + Title ).c_str() );

    if ( ProcessCompletedEvent == NULL )
    {
        return 1;
    }

    SetEvent( ProcessCompletedEvent );

    ( void )WaitForSingleObject( IndefiniteWaitEvent, -1 );

    return 0;
}