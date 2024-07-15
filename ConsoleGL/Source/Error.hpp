#pragma once
#include <ConsoleGL.hpp>

namespace ConsoleGL
{
	class Error
	{
	public:

		static EError GetLastError() { return s_LastError; }
		static void SetLastError( const EError a_Error ) { s_LastError = a_Error; }
		static void ClearLastError() { s_LastError = Error_NoError; }
		static const char* GetLastErrorMessage() { return GetErrorMessage( s_LastError ); }
		static const char* GetErrorMessage( const EError a_Error );

	private:

		static EError s_LastError;
	};
}
