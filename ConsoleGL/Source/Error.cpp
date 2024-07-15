#include <Error.hpp>

ConsoleGL::EError ConsoleGL::Error::s_LastError = Error_NoError;

const char* ConsoleGL::Error::GetErrorMessage( const EError a_Error )
{
#define SET_ERROR_MESSAGE( Type, Message ) case Error_##Type: return Message;

	switch ( a_Error )
	{
		default: return "";

		SET_ERROR_MESSAGE( NoError, "No error." );
		SET_ERROR_MESSAGE( NoActiveWindow, "No active window was set. Please CreateWindow and SetActiveWindow()" );
		SET_ERROR_MESSAGE( NoActiveContext, "" );
	}

#undef SET_ERROR_MESSAGE
}