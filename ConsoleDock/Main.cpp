#include <iostream>
#include <thread>
#include <string>
#include <Windows.h>

int main( int argc, const char** argv )
{
    wchar_t Buffer0[ 256 ];
    wchar_t Buffer1[ 256 ];

    //for ( int i = 0; i < argc; ++i )
    //{
    //    std::cout << argv[ i ];
    //}

    mbstowcs( Buffer0, ( std::string( "WakeEvent_" ) + argv[ 0 ] ).c_str(), 256 );
    auto WakeEvent = CreateEvent(
        NULL,
        true,
        false,
        Buffer0
    );

    mbstowcs( Buffer1, ( std::string( "SleepEvent_" ) + argv[ 0 ] ).c_str(), 256 );
    auto SleepEvent = CreateEvent(
        NULL,
        true,
        false,
        Buffer1
    );

    while ( true )
    {
        WaitForSingleObject(
            WakeEvent,
            -1
        );

        ResetEvent( WakeEvent );

        std::cout << "awake_event triggered!";

        WaitForSingleObject(
            SleepEvent,
            -1
        );

        ResetEvent( SleepEvent );

        std::cout << "sleep_event triggered!";
    }

    return 0;
}