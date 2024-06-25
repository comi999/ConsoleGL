#pragma once
#include <string>

class Event
{
public:

    Event() = default;
    Event( const Event& ) = delete;
    Event( Event&& a_Event ) noexcept;
    Event( const std::string& a_Name, const bool a_AutoReset = false );
    Event( const uintptr_t a_Handle ) : m_Handle( a_Handle ) {}
    Event& operator=( const Event& ) = delete;
    Event& operator=( Event&& a_Event ) noexcept;
    ~Event();
    bool Trigger() const;
    bool IsTriggered() const;
    bool Reset() const;
    bool Wait( const uint32_t a_Milliseconds = -1 ) const;
    bool IsValid() const { return m_Handle != 0u; }
    bool Create( const std::string& a_Name, const bool a_AutoReset = false ) { *this = Event( a_Name, a_AutoReset ); return IsValid(); }
    void Clear();
    uintptr_t GetHandle() const { return m_Handle; }

private:

    uintptr_t m_Handle = 0u;
};