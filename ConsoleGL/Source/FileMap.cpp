#include <FileMap.hpp>

#include <Windows.h>


FileMap::FileMap( FileMap&& a_FileMap ) noexcept
{
	if ( !a_FileMap.IsValid() )
	{
		return;
	}

    m_Memory = a_FileMap.m_Memory;
    m_Handle = a_FileMap.m_Handle;
    m_Size = a_FileMap.m_Size;
    m_IsReadOnly = a_FileMap.m_IsReadOnly;

    a_FileMap.m_Memory = nullptr;
    a_FileMap.m_Handle = 0u;
    a_FileMap.m_Size = 0u;
    a_FileMap.m_IsReadOnly = false;
}

FileMap::FileMap( const std::string& a_Name, const size_t a_Size, const bool a_ReadOnly )
{
    if ( a_Size == 0u )
    {
	    return;
    }
    
	LARGE_INTEGER Size;
    Size.QuadPart = a_Size;
    
	// Attempt to create file mapping.
	m_Handle = ( uintptr_t )CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        nullptr,
        a_ReadOnly ? PAGE_READONLY : PAGE_READWRITE,
        Size.HighPart,
        Size.LowPart,
        a_Name.c_str() );

    // If the file mapping failed to be created, then fail.
    if ( m_Handle == 0u )
    {
        return;
    }

    // Try to get a pointer to the file mapping.
    m_Memory = MapViewOfFile(
        ( HANDLE )m_Handle,
        a_ReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
        0, 0,
        Size.QuadPart );

    // If we couldn't get a file map pointer, then fail.
    if ( m_Memory == nullptr )
    {
    	ENSURE_LOG( CloseHandle( ( HANDLE )m_Handle ), "Failed to close file map handle." );
	    m_Handle = 0u;
        m_Size = 0u;
        return;
    }

    // If it already existed before creating it, then the size here
    // would have been set above, so we shouldn't try to stomp the size value.
    m_Size = a_Size;
    m_IsReadOnly = a_ReadOnly;
}

FileMap::FileMap( const uintptr_t a_Handle, const bool a_ReadOnly )
{
    if ( a_Handle == 0u )
    {
	    return;
    }

    // Attempt to get the size of the file mapping.
    LARGE_INTEGER Size;
    if ( !GetFileSizeEx( ( HANDLE )a_Handle, &Size ) )
    {
	    return;
    }

    // Try to get a pointer to the file mapping.
    m_Memory = MapViewOfFile(
        ( HANDLE )a_Handle,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        Size.QuadPart );

    // If we couldn't get a file map pointer, then fail.
    if ( m_Memory == nullptr )
    {
    	ENSURE_LOG( CloseHandle( ( HANDLE )m_Handle ), "Failed to close file map handle." );
	    m_Handle = 0u;
        return;
    }

    m_Size = Size.QuadPart;
    m_IsReadOnly = a_ReadOnly;
}

FileMap::~FileMap()
{
	Clear();
}


FileMap& FileMap::operator=( FileMap&& a_FileMap ) noexcept
{
    Clear();

	if ( a_FileMap.IsValid() )
	{
		m_Memory = a_FileMap.m_Memory;
        m_Handle = a_FileMap.m_Handle;
        m_Size = a_FileMap.m_Size;
        m_IsReadOnly = a_FileMap.m_IsReadOnly;

        a_FileMap.m_Memory = nullptr;
        a_FileMap.m_Handle = 0u;
        a_FileMap.m_Size = 0u;
        a_FileMap.m_IsReadOnly = false;
	}

    return *this;
}

bool FileMap::Clear()
{
	if ( IsValid() )
	{
		ENSURE_LOG( CloseHandle( ( HANDLE )m_Handle ), "Failed to close file map handle." );
		m_Memory = nullptr;
		m_Handle = 0u;
		m_Size = 0u;
        m_IsReadOnly = false;
		return true;
	}

	return true;
}