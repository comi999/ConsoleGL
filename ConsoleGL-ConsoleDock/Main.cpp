#include <iostream>
#include <Windows.h>

#include <Event.hpp>
#include <FileMap.hpp>

enum class EWindowDockCommand
{
    Release,
    Attach,
    Terminate
};

struct WindowDockCommandBuffer
{
    EWindowDockCommand Command;
    uint32_t Value;
};

int main( int a_ArgCount, const char** a_Args )
{
	if ( a_ArgCount != 2 )
	{
		return 1;
	}

    const std::string Name = a_Args[ 1 ];
	const Event ProcessStarted( Name + "_ProcessStarted" );
    const Event CommandReady( Name + "_CommandReady" );
    const Event CommandComplete( Name + "_CommandComplete" );
    const FileMap CommandBuffer( Name + "_CommandBuffer", sizeof( CommandBuffer ), true );

    ENSURE_LOG( ProcessStarted.IsValid(), "Could not create event '%s_ProcessStarted.", Name );
    ENSURE_LOG( CommandReady.IsValid(), "Could not create event '%s_CommandReady.", Name );
    ENSURE_LOG( CommandComplete.IsValid(), "Could not create event '%s_CommandComplete.", Name );
    ENSURE_LOG( CommandBuffer.IsValid(), "Could not create file map '%s_CommandBuffer.", Name );
    ENSURE_LOG( ProcessStarted.Trigger(), "Could not trigger '%s_ProcessStarted'.", Name );

    bool Continue = true;

    while ( Continue )
    {
	    ENSURE_LOG( CommandReady.Wait(), "Failed to wait for event '%s_CommandReady." );

        const auto Buffer = ( WindowDockCommandBuffer* )CommandBuffer.Data();

	    switch ( Buffer->Command )
	    {
		case EWindowDockCommand::Attach:
        {
            const DWORD ProcessID = Buffer->Value;
            ENSURE_LOG( AttachConsole( ProcessID ), "Could not attach console for process ID %u.", ProcessID );
            break;
        }
	    case EWindowDockCommand::Release:
		{
			ENSURE_LOG( FreeConsole(), "Could not free console." );
            break;
		}
	    case EWindowDockCommand::Terminate:
		{
			Continue = false;
            break;
		}
	    }

        ENSURE_LOG( CommandComplete.Trigger(), "Failed to wait for event '%s_CommandComplete." );
    }
}
