#include <stdarg.h>
#include <stdio.h>
#include <Windows.h>

#include <Log.hpp>

void Log( const char* a_String )
{
	OutputDebugStringA( a_String );
}

template < ELogLevel _LogLevel >
void LogImpl( const char* a_LogCategory, const char* a_Formatting, const va_list a_FormatStringInputs )
{
	thread_local char LogBuffer[ LOG_MAX_LENGTH ];
	int32_t LogOffset = 0u;

	// Log messages are of the form of "[LEVEL][CATEGORY]: MSG".

	// First need to print in "[LEVEL]".
	if constexpr ( _LogLevel == ELogLevel::Info )
	{
		LogOffset += sprintf_s( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "[INFO]" );
	}
	else if constexpr ( _LogLevel == ELogLevel::Warning )
	{
		LogOffset += sprintf_s( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "[WARNING]" );
	}
	else if constexpr ( _LogLevel == ELogLevel::Critical )
	{
		LogOffset += sprintf_s( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "[CRITICAL]" );
	}

	// Next we need to print in "[CATEGORY]: ".
	LogOffset += sprintf_s( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "[%s]: ", a_LogCategory );

	// Lastly we need to print the "MSG".
	LogOffset += vsnprintf( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, a_Formatting, a_FormatStringInputs );

	// We want to end the log with a new-line character.
	LogOffset += sprintf_s( LogBuffer + LogOffset, LOG_MAX_LENGTH - LogOffset, "\n" );

	// Output the log.
	Log( LogBuffer );
}

#define DEFINE_LOGGER_FUNCTION( Level ) \
	void Log_##Level( const char* a_LogCategory, const char* a_Formatting, ... ) \
	{ \
		va_list FormatStringInputs; \
		va_start( FormatStringInputs, a_Formatting ); \
		LogImpl< ELogLevel::Level >( a_LogCategory, a_Formatting, FormatStringInputs ); \
		va_end( FormatStringInputs ); \
	}

DEFINE_LOGGER_FUNCTION( Info );
DEFINE_LOGGER_FUNCTION( Warning );
DEFINE_LOGGER_FUNCTION( Critical );

void Log( const char* a_LogCategory, const ELogLevel a_LogLevel, const char* a_Formatting, const va_list a_FormatStringInputs )
{
	switch ( a_LogLevel )
	{
	case ELogLevel::Critical: Log_Critical( a_LogCategory, a_Formatting, a_FormatStringInputs ); return;
	case ELogLevel::Warning: Log_Warning( a_LogCategory, a_Formatting, a_FormatStringInputs ); return;
	case ELogLevel::Info: Log_Info( a_LogCategory, a_Formatting, a_FormatStringInputs ); return;
	case ELogLevel::None:;
	}
}

void Log( const char* a_LogCategory, const ELogLevel a_LogLevel, const char* a_Formatting, ... )
{
	va_list FormatStringInputs;
	va_start( FormatStringInputs, a_Formatting );
	Log( a_LogCategory, a_LogLevel, a_Formatting, FormatStringInputs );
	va_end( FormatStringInputs );
}