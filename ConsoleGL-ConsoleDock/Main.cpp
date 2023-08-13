#include <iostream>
#include <thread>
#include <string>
#include <Windows.h>

int main()
{
    auto Event = CreateEvent(
        NULL,
        false,
        false,
        L"console_dock_event"
    );

    if ( Event == NULL )
    {
        return 1;
    }

    ( void )WaitForSingleObject( Event, -1 );

    return 0;
}