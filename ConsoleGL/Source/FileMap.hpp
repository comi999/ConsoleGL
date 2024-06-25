#pragma once
#include <string>

class FileMap
{
public:

	FileMap() = default;
	FileMap( const FileMap& ) = delete;
	FileMap( FileMap&& a_FileMap ) noexcept;
	FileMap( const std::string& a_Name, const size_t a_Size, const bool a_ReadOnly = false );
	FileMap( const uintptr_t a_Handle, const bool a_ReadOnly = false );
	~FileMap();
	
	FileMap& operator=( const FileMap& ) = delete;
	FileMap& operator=( FileMap&& a_FileMap ) noexcept;

	bool IsValid() const { return m_Handle != 0u; }
	bool Create( const std::string& a_Name, const size_t a_Size, const bool a_ReadOnly = false ) { *this = FileMap( a_Name, a_Size, a_ReadOnly ); return IsValid(); }
	bool Clear();
	void* Data() const { return m_Memory; }
	uintptr_t GetHandle() const { return m_Handle; }
	size_t GetSize() const { return m_Size; }
	bool IsReadOnly() const { return m_IsReadOnly; }

private:

	void* m_Memory = nullptr;
	uintptr_t m_Handle = 0u;
	size_t m_Size = 0u;
	bool m_IsReadOnly = false;
};