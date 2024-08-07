#include <stdlib.h>
#include <cstdio>
#include <varargs.h>

#include <Assert.hpp>

#if CONFIG_DEBUG
int32_t Format( char* a_Buffer, const size_t a_BufferLength, const char* a_FormatString, va_list a_Inputs )
{
	return vsnprintf( a_Buffer, a_BufferLength, a_FormatString, a_Inputs );
}

int32_t Format( char* a_Buffer, const size_t a_BufferLength, const char* a_FormatString, ... )
{
	va_list Inputs;
	va_start( Inputs );
	const int32_t Written = Format( a_Buffer, a_BufferLength, a_FormatString, Inputs );
	va_end( Inputs );
	return Written;
}

void Break()
{
	__debugbreak();
}
#endif

void Abort()
{
	abort();
}

#if CONFIG_DEBUG
void Message( const char* a_Type, const char* a_Function, const char* a_File, const int a_Line, const char* a_FormatString = nullptr, const va_list a_Inputs = nullptr )
{
	thread_local char LogBuffer[ LOG_MAX_LENGTH ];
	int32_t LogOffset = Format( LogBuffer, LOG_MAX_LENGTH,
			"[%s]: Failed in function \"%s\" @\n"
			"\t\t%s(%i)\n", a_Type, a_Function, a_File, a_Line );

	if ( a_FormatString )
	{
		// Add a '\t\t' to the start.
		LogOffset += Format( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "\t\t" );

		LogOffset += Format( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, a_FormatString, a_Inputs );

		// Add a '\n' to the end.
		( void )Format( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "\n" );
	}

	Log( LogBuffer );
}
#endif

#if CONFIG_DEBUG
void Check( const bool a_Condition, const char* a_Function, const char* a_File, const int a_Line )
{
	if ( !a_Condition )
	{
		Message( "CHECK", a_Function, a_File, a_Line );
	}
}

void Check_Log( const bool a_Condition, const char* a_Function, const char* a_File, const int a_Line, const char* a_FormatString, ... )
{
	if ( !a_Condition )
	{
		va_list Inputs;
		va_start( Inputs );
		Message( "CHECK", a_Function, a_File, a_Line, a_FormatString, Inputs );
		va_end( Inputs );
	}
}
#endif

#if CONFIG_DEBUG
bool Assert( const bool a_Condition, const char* a_Function, const char* a_File, const int a_Line )
{
    if ( !a_Condition )
    {
		Message( "ASSERT", a_Function, a_File, a_Line );
		Break();
    }

    return a_Condition;
}

bool Assert_Log( const bool a_Condition, const char* a_Function, const char* a_File, const int a_Line, const char* a_FormatString, ...  )
{
    if ( !a_Condition )
    {
		va_list Inputs;
		va_start( Inputs );
		Message( "ASSERT", a_Function, a_File, a_Line, a_FormatString, Inputs );
		va_end( Inputs );
    }

    return a_Condition;
}
#endif

bool Ensure( const bool a_Condition
#if CONFIG_DEBUG
	, const char* a_Function, const char* a_File, const int a_Line
#endif
	)
{
    if ( !a_Condition )
    {
#if CONFIG_DEBUG
		Message( "ENSURE", a_Function, a_File, a_Line );
		Break();
#endif
		Abort();
    }

    return a_Condition;
}


bool Ensure_Log( const bool a_Condition
#if CONFIG_DEBUG
	, const char* a_Function, const char* a_File, const int a_Line, const char* a_FormatString, ...
#endif
	)
{
	if ( !a_Condition )
    {
#if CONFIG_DEBUG
		va_list Inputs;
		va_start( Inputs );
		Message( "ENSURE", a_Function, a_File, a_Line, a_FormatString, Inputs );
		va_end( Inputs );
		Break();
#endif
		Abort();
    }

    return a_Condition;
}