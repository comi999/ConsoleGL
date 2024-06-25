#include <Windows.h>
#include <Event.hpp>

Event::Event( const std::string& a_Name, const bool a_AutoReset )
	: m_Handle{ ( uintptr_t )CreateEventA(
        nullptr,
        !a_AutoReset,
        false,
        a_Name.c_str() ) }
{}

Event::Event( Event&& a_Event ) noexcept
{
	if ( !a_Event.IsValid() )
	{
		return;
	}

	m_Handle = a_Event.m_Handle;
	a_Event.m_Handle = 0u;
}

Event::~Event()
{
	if ( IsValid() )
	{
		ENSURE_LOG( CloseHandle( ( HANDLE )m_Handle ), "Failed to close event handle." );
	}
}


Event& Event::operator=( Event&& a_Event ) noexcept
{
	Clear();

	if ( a_Event.IsValid() )
	{
		m_Handle = a_Event.m_Handle;
		a_Event.m_Handle = 0u;
	}

	return *this;
}

bool Event::Trigger() const
{
	if ( !IsValid() )
	{
		return false;
	}

	return SetEvent( ( HANDLE )m_Handle );
}

bool Event::IsTriggered() const
{
	if ( !IsValid() )
	{
		return false;
	}

	return WaitForSingleObject( ( HANDLE )m_Handle, 0u );
}


bool Event::Reset() const
{
	if ( !IsValid() )
	{
		return false;
	}

	return ResetEvent( ( HANDLE )m_Handle );
}

bool Event::Wait( const uint32_t a_Milliseconds ) const
{
	if ( !IsValid() )
	{
		return false;
	}

	return WaitForSingleObject( ( HANDLE )m_Handle, a_Milliseconds ) == WAIT_OBJECT_0;
}

void Event::Clear()
{
	if ( !IsValid() )
	{
		return;
	}

	ENSURE_LOG( CloseHandle( ( HANDLE )m_Handle ), "Failed to close event handle." );
	m_Handle = 0u;
}