#include <array>
#include <optional>
#include <map>
#include <fstream>
#include <chrono>
#include <filesystem>


#include <ConsoleGL.hpp>
#include <Error.hpp>
#include <Window.hpp>

#include <memory_module/MemoryModule.hpp>

// Disable dll exported functions.
#define _USER32_
#include <Windows.h>
#include <synchapi.h>
#undef CreateWindow

#if IS_CONSOLEGL
//#include <WindowDock.inl>
#include <PixelMap.inl>
#include <ShaderCompiler.inl>
#endif

ConsoleGL::EError ConsoleGL::GetLastError()
{
	return Error::GetLastError();
}

const char* ConsoleGL::GetLastErrorMessage()
{
	return Error::GetLastErrorMessage();
}

const char* ConsoleGL::GetErrorMessage( const EError a_Error )
{
	return Error::GetErrorMessage( a_Error );
}

char	ShaderInfoLog[ SHADER_INFO_LOG_MAX ];
size_t	ShaderInfoLogLength = 0u;

void ClearShaderInfoLog()
{
	ZeroMemory( ShaderInfoLog, sizeof( ShaderInfoLog ) );
	ShaderInfoLogLength = 0u;
}

void WriteToShaderInfoLog( const char* a_String, const size_t a_Length )
{
	memcpy_s( ShaderInfoLog + ShaderInfoLogLength, sizeof( ShaderInfoLog ) - 1 - ShaderInfoLogLength, a_String, a_Length );
	ShaderInfoLogLength += a_Length;
}

template < size_t _Size >
void WriteToShaderInfoLog( const char( &a_String )[ _Size ] )
{
	WriteToShaderInfoLog( a_String, _Size );
}

#pragma region Repository

#define DEFINE_REPOSITORY( Name, Capacity ) \
class Name##Repository \
{ \
public: \
\
	using Type = ConsoleGL::Name; \
	static constexpr size_t C##apacity = Capacity; \
\
	Name##Repository() = default; \
	Name##Repository( const Name##Repository& ) = delete; \
	Name##Repository( Name##Repository&& ) = delete; \
	Name##Repository& operator=( const Name##Repository& ) = delete; \
	Name##Repository& operator=( Name##Repository&& ) = delete; \
\
	size_t Used() const { return SlotsUsed; } \
	size_t Free() const { return C##apacity - Used(); } \
\
	template < typename... _Args > \
	Type* Create( _Args&&... a_Args ) \
	{ \
		const size_t Index = Next(); \
\
		if ( Index == Instances.size() ) \
		{ \
			ConsoleGL::Error::SetLastError( ConsoleGL::Error_##Name##CapacityReached ); \
			return nullptr; \
		} \
\
		++SlotsUsed; \
		return &Instances[ Index ].emplace( std::forward< _Args >( a_Args )... ); \
	} \
\
	bool IsValid( const Type* a_Instance ) const { return IndexOf( a_Instance ) != -1; } \
\
	size_t IndexOf( const Type* a_Instance ) const \
	{ \
		static const size_t Offset = [] \
		{ \
			std::optional Object{ 0 }; \
			return ( size_t )&Object.value() - ( size_t )&Object; \
		}(); \
\
		const size_t Index = ( const std::optional< Type >* )( a_Instance - Offset ) - &*Instances.begin(); \
\
		if ( Index < Instances.size() ) \
		{ \
			return Index; \
		} \
\
		return -1; \
	} \
\
	bool Destroy( const Type* a_Instance ) \
	{ \
		const size_t Index = IndexOf( a_Instance ); \
\
		if ( Index == -1 ) \
		{ \
			ConsoleGL::Error::SetLastError( ConsoleGL::Error_Invalid##Name##Handle ); \
			return false; \
		} \
\
		--SlotsUsed; \
		Instances[ Index ].reset(); \
\
		if ( Tombstone > Index ) \
		{ \
			Tombstone = Index; \
		} \
	} \
\
private: \
\
	size_t Next() const \
	{ \
		while ( Tombstone < C##apacity && Instances[ Tombstone ].has_value() ) ++Tombstone; \
		return Tombstone; \
	} \
\
	std::array< std::optional< Type >, C##apacity > Instances; \
	mutable size_t Tombstone = 0u; \
	size_t SlotsUsed = 0u; \
};

#pragma endregion

namespace ConsoleGL
{
	struct Context
	{
		ShaderProgramHandle	ActiveShaderProgram;
	};

	enum class EShaderSourceType
	{
		None,
		FileSource,
		FileCompiled,
		MemorySource,
		MemoryCompiled
	};

	struct ShaderProcInfo
	{
		bool IsMemoryLoaded;
		const void* Handle;
		const void* Entry;
		const void* Info;

		// .. Here will be where all the reflection info will be created from ShaderCompiler.exe
	};

	struct Shader
	{
		EShaderType							Type;
		EShaderSourceType					SourceType;
		std::string							File;
		std::string							Source;
		std::vector< uint8_t >				Binary;
		size_t								BufferSize;
		const void*							Buffer;
		std::shared_ptr< ShaderProcInfo >	Proc;
		std::vector< ShaderProgramHandle >  AttachedTo;
	};

	struct ShaderProgramEntry
	{
		std::shared_ptr< ShaderProcInfo >	Proc;
		ShaderHandle						Attached;
	};

	struct ShaderProgram
	{
		ShaderProgramEntry	Entries[ 2u /*ShaderStages*/ ];
		bool				IsLinked;
	};
}

//DEFINE_REPOSITORY( Window, WINDOW_MAX_COUNT );
DEFINE_REPOSITORY( Context, CONTEXT_MAX_COUNT );
DEFINE_REPOSITORY( Shader, SHADER_MAX_COUNT );
DEFINE_REPOSITORY( ShaderProgram, SHADER_PROGRAM_MAX_COUNT );

struct
{
	//WindowRepository				Windows;
	ContextRepository				Contexts;
	ShaderRepository				Shaders;
	ShaderProgramRepository			ShaderPrograms;
	ConsoleGL::WindowHandle			ActiveWindow;
	ConsoleGL::ContextHandle		ActiveContext;
} static State;

#pragma region Window functions

ConsoleGL::Window* ConsoleGL::CreateWindow( const char* a_Title, const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_PixelWidth, const uint32_t a_PixelHeight, const uint32_t a_BufferCount )
{
	return Window::Create( a_Title, a_Width, a_Height, a_PixelWidth, a_PixelHeight, a_BufferCount );
}

void ConsoleGL::DestroyWindow( Window* a_Window )
{
	return Window::Destroy( a_Window );
}

void ConsoleGL::SetActiveWindow( Window* a_Window )
{
	Window::SetActive( a_Window );
}

ConsoleGL::Window* ConsoleGL::GetActiveWindow()
{
	return Window::GetActive();
}

void ConsoleGL::SetWindowTitle( const char* a_Title )
{
	Window::SetTitle( a_Title );
}

void ConsoleGL::SetWindowColoursFromArray( const Colour* a_Colours )
{
	Window::SetColours( a_Colours );
}

void ConsoleGL::SetWindowColoursFromSet( const EColourSet a_ColourSet )
{
	Window::SetColours( a_ColourSet );
}

void ConsoleGL::SwapWindowBuffer()
{
	Window::SwapBuffer();
}

void ConsoleGL::SwapWindowBufferByIndex( uint32_t a_Index )
{
	Window::SwapBuffer( a_Index );
}

const char* ConsoleGL::GetWindowTitle( Window* a_Window )
{
	return a_Window->GetTitle().c_str();
}

uint32_t ConsoleGL::GetWindowWidth( Window* a_Window )
{
	return a_Window->GetWidth();
}

uint32_t ConsoleGL::GetWindowHeight( Window* a_Window )
{
	return a_Window->GetHeight();
}

uint32_t ConsoleGL::GetWindowBufferIndex( Window* a_Window )
{
	return a_Window->GetBufferIndex();
}

uint32_t ConsoleGL::GetWindowBufferCount( Window* a_Window )
{
	return a_Window->GetBufferCount();
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBuffer( Window* a_Window )
{
	return a_Window->GetBuffer();
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBufferByIndex( Window* a_Window, const uint32_t a_Index )
{
	return a_Window->GetBuffer( a_Index );
}

#pragma endregion

#pragma region Context functions

ConsoleGL::Context* ConsoleGL::CreateContext()
{
	return State.Contexts.Create();
}

bool ConsoleGL::DestroyContext( const ContextHandle a_Context )
{
	const bool Success = State.Contexts.Destroy( a_Context );

	if ( Success && State.ActiveContext == a_Context )
	{
		State.ActiveContext = nullptr;
	}

	return Success;
}

void ConsoleGL::SetActiveContext( const ContextHandle a_Context )
{
	State.ActiveContext = a_Context;
}

ConsoleGL::Context* ConsoleGL::GetActiveContext()
{
	return State.ActiveContext;
}

#pragma endregion

#pragma region Input functions

bool ConsoleGL::IsKeyUp( const EKeyboardKey a_KeyboardKey )
{
	return Window::IsKeyUp( a_KeyboardKey );
}

bool ConsoleGL::IsKeyDown( const EKeyboardKey a_KeyboardKey )
{
	return Window::IsKeyDown( a_KeyboardKey );
}

bool ConsoleGL::IsKeyPressed( const EKeyboardKey a_KeyboardKey )
{
	return Window::IsKeyPressed( a_KeyboardKey );
}

bool ConsoleGL::IsKeyReleased( const EKeyboardKey a_KeyboardKey )
{
	return Window::IsKeyReleased( a_KeyboardKey );
}

bool ConsoleGL::IsMouseDown( const EMouseButton a_MouseButton )
{
	return Window::IsMouseDown( a_MouseButton );
}

bool ConsoleGL::IsMouseUp( const EMouseButton a_MouseButton )
{
	return Window::IsMouseUp( a_MouseButton );
}

bool ConsoleGL::IsMousePressed( const EMouseButton a_MouseButton )
{
	return Window::IsMousePressed( a_MouseButton );
}

bool ConsoleGL::IsMouseReleased( const EMouseButton a_MouseButton )
{
	return Window::IsMouseReleased( a_MouseButton );
}

void ConsoleGL::GetMousePosition( float& o_X, float& o_Y )
{
	return Window::GetMousePosition( o_X, o_Y );
}

void ConsoleGL::GetMouseDelta( float& o_X, float& o_Y )
{
	return Window::GetMouseDelta( o_X, o_Y );
}

void ConsoleGL::PollEvents()
{
	return Window::PollEvents();
}

#pragma endregion

#pragma region Pixel map functions

const ConsoleGL::Pixel* ConsoleGL::GetPixelMap()
{
#if IS_CONSOLEGL
	return ( const Pixel* )PixelMap;
#else
	static constexpr Pixel Empty;
	return &Empty;
#endif
}

const ConsoleGL::Pixel* ConsoleGL::MapColourToPixel( const Colour a_Colour )
{
#if IS_CONSOLEGL
	const uint8_t R = a_Colour.r >> PIXEL_MAP_MIP_LEVEL;
	const uint8_t G = a_Colour.g >> PIXEL_MAP_MIP_LEVEL;
	const uint8_t B = a_Colour.b >> PIXEL_MAP_MIP_LEVEL;

	return ( const Pixel* )PixelMap + ( R * PIXEL_MAP_SIZE * PIXEL_MAP_SIZE ) + ( G * PIXEL_MAP_SIZE ) + B;
#else
	return GetPixelMap();
#endif
}

size_t ConsoleGL::GetPixelMapSize()
{
#if IS_CONSOLEGL
	return PIXEL_MAP_VOLUME;
#else
	return 1u;
#endif
}

#pragma endregion

#pragma region PixelBuffer functions

ConsoleGL::Pixel* ConsoleGL::GetPixelBufferPixels( PixelBuffer* a_Buffer )
{
	return a_Buffer->GetPixels();
}

uint32_t ConsoleGL::GetPixelBufferSize( PixelBuffer* a_Buffer )
{
	return a_Buffer->GetSize();
}

uint32_t ConsoleGL::GetPixelBufferWidth( PixelBuffer* a_Buffer )
{
	return a_Buffer->GetWidth();
}

uint32_t ConsoleGL::GetPixelBufferHeight( PixelBuffer* a_Buffer )
{
	return a_Buffer->GetHeight();
}

#pragma endregion

#pragma region Basic drawing functions

void ConsoleGL::SetPixel( PixelBuffer* a_Buffer, uint32_t a_Index, Pixel a_Pixel )
{
	a_Buffer->SetPixel( a_Index, a_Pixel );
}

void ConsoleGL::SetPixelByPosition( PixelBuffer* a_Buffer, Coord a_Position, Pixel a_Pixel )
{
	a_Buffer->SetPixel( a_Position, a_Pixel );
}

void ConsoleGL::SetPixels( PixelBuffer* a_Buffer, uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel )
{
	a_Buffer->SetPixels( a_Index, a_Count, a_Pixel );
}

void ConsoleGL::SetPixelsByPosition( PixelBuffer* a_Buffer, Coord a_Position, uint32_t a_Count, Pixel a_Pixel )
{
	a_Buffer->SetPixels( a_Position, a_Count, a_Pixel );
}

void ConsoleGL::SetBuffer( PixelBuffer* a_Buffer, Pixel a_Pixel )
{
	a_Buffer->SetBuffer( a_Pixel );
}

void ConsoleGL::SetBufferFn( PixelBuffer* a_Buffer, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->SetBuffer( a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawLine( PixelBuffer* a_Buffer, const Seg& a_Seg, Pixel a_Pixel )
{
	a_Buffer->DrawLine( a_Seg, a_Pixel );
}

void ConsoleGL::DrawLineFn( PixelBuffer* a_Buffer, const Seg& a_Seg, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawLine( a_Seg, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawHorizontalLine( PixelBuffer* a_Buffer, Coord a_Begin, uint32_t a_Length, Pixel a_Pixel )
{
	a_Buffer->DrawHorizontalLine( a_Begin, a_Length, a_Pixel );
}

void ConsoleGL::DrawHorizontalLineFn( PixelBuffer* a_Buffer, Coord a_Begin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawHorizontalLine( a_Begin, a_Length, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawVerticalLine( PixelBuffer* a_Buffer, Coord a_Begin, uint32_t a_Length, Pixel a_Pixel )
{
	a_Buffer->DrawVerticalLine( a_Begin, a_Length, a_Pixel );
}

void ConsoleGL::DrawVerticalLineFn( PixelBuffer* a_Buffer, Coord a_Begin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawVerticalLine( a_Begin, a_Length, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawTriangle( PixelBuffer* a_Buffer, const Tri& a_Tri, Pixel a_Pixel )
{
	return a_Buffer->DrawTriangle( a_Tri, a_Pixel );
}

void ConsoleGL::DrawTriangleFn( PixelBuffer* a_Buffer, const Tri& a_Tri, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	return a_Buffer->DrawTriangle( a_Tri, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawTriangleFilled( PixelBuffer* a_Buffer, const Tri& a_Tri, Pixel a_Pixel )
{
	return a_Buffer->DrawTriangleFilled( a_Tri, a_Pixel );
}

void ConsoleGL::DrawTriangleFilledFn( PixelBuffer* a_Buffer, const Tri& a_Tri, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	return a_Buffer->DrawTriangleFilled( a_Tri, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRect( PixelBuffer* a_Buffer, const Rect& a_Rect, Pixel a_Pixel )
{
	a_Buffer->DrawRect( a_Rect, a_Pixel );
}

void ConsoleGL::DrawRectFn( PixelBuffer* a_Buffer, const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRect( a_Rect, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectRotated( PixelBuffer* a_Buffer, const Rect& a_Rect, float a_Radians, Pixel a_Pixel )
{
	a_Buffer->DrawRect( a_Rect, a_Radians, a_Pixel );
}

void ConsoleGL::DrawRectRotatedFn( PixelBuffer* a_Buffer, const Rect& a_Rect, float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRect( a_Rect, a_Radians, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectFilled( PixelBuffer* a_Buffer, const Rect& a_Rect, Pixel a_Pixel )
{
	a_Buffer->DrawRectFilled( a_Rect, a_Pixel );
}

void ConsoleGL::DrawRectFilledFn( PixelBuffer* a_Buffer, const Rect& a_Rect, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRectFilled( a_Rect, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectFilledRotated( PixelBuffer* a_Buffer, const Rect& a_Rect, float a_Radians, Pixel a_Pixel )
{
	a_Buffer->DrawRectFilled( a_Rect, a_Radians, a_Pixel );
}

void ConsoleGL::DrawRectFilledRotatedFn( PixelBuffer* a_Buffer, const Rect& a_Rect, float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRectFilled( a_Rect, a_Radians, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawCircle( PixelBuffer* a_Buffer, Coord a_Centre, uint32_t a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawCircle( a_Centre, a_Radius, a_Pixel );
}

void ConsoleGL::DrawCircleFn( PixelBuffer* a_Buffer, Coord a_Centre, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawCircle( a_Centre, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawCircleFilled( PixelBuffer* a_Buffer, Coord a_Centre, uint32_t a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawCircleFilled( a_Centre, a_Radius, a_Pixel );
}

void ConsoleGL::DrawCircleFilledFn( PixelBuffer* a_Buffer, Coord a_Centre, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawCircleFilled( a_Centre, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawEllipse( PixelBuffer* a_Buffer, Coord a_Centre, Coord a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawEllipse( a_Centre, a_Radius, a_Pixel );
}

void ConsoleGL::DrawEllipseFn( PixelBuffer* a_Buffer, Coord a_Centre, Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawEllipse( a_Centre, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawEllipseFilled( PixelBuffer* a_Buffer, Coord a_Centre, Coord a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawEllipseFilled( a_Centre, a_Radius, a_Pixel );
}

void ConsoleGL::DrawEllipseFilledFn( PixelBuffer* a_Buffer, Coord a_Centre, Coord a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawEllipseFilled( a_Centre, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

#pragma endregion

#pragma region Rendering functions

ConsoleGL::Shader* ConsoleGL::CreateShader( const EShaderType a_ShaderType )
{
	Shader* Created = State.Shaders.Create();
	Created->Type = a_ShaderType;
	return Created;
}

bool ConsoleGL::SetShaderSourceFromFile( const ShaderHandle a_Shader, const char* a_File )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	if ( !a_File )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( IsShaderCompiled( a_Shader ) )
	{
		Error::SetLastError( Error_ShaderAlreadyCompiled );
		return false;
	}

	a_Shader->SourceType = EShaderSourceType::FileSource;
	a_Shader->File = a_File;
	return true;
}

bool ConsoleGL::SetShaderSourceFromString( const ShaderHandle a_Shader, const char* a_Source, const size_t a_Size )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	if ( !a_Source || !a_Size )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( IsShaderCompiled( a_Shader ) )
	{
		Error::SetLastError( Error_ShaderAlreadyCompiled );
		return false;
	}

	a_Shader->SourceType = EShaderSourceType::MemorySource;
	a_Shader->Source = std::string_view{ a_Source, a_Size };
	return true;
}

bool ConsoleGL::SetShaderBinaryFromFile( const ShaderHandle a_Shader, const char* a_File )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	if ( !a_File )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( IsShaderCompiled( a_Shader ) )
	{
		Error::SetLastError( Error_ShaderAlreadyCompiled );
		return false;
	}

	a_Shader->SourceType = EShaderSourceType::FileCompiled;
	a_Shader->File = a_File;
	return true;
}

bool ConsoleGL::SetShaderBinaryFromMemory( const ShaderHandle a_Shader, const void* a_Buffer, const size_t a_Size )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	if ( !a_Buffer || !a_Size )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( IsShaderCompiled( a_Shader ) )
	{
		Error::SetLastError( Error_ShaderAlreadyCompiled );
		return false;
	}

	a_Shader->SourceType = EShaderSourceType::FileCompiled;
	a_Shader->Buffer = a_Buffer;
	a_Shader->BufferSize = a_Size;
	return true;
}

bool ConsoleGL::CompileShader( const ShaderHandle a_Shader )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

#if IS_CONSOLEGL
	auto RunCompiler = []( const char* a_SourceFile, const char* a_OutputFile ) -> bool
	{
		bool FailedToWriteShaderCompilerExe = false;

		if ( !std::filesystem::exists( "ShaderCompiler.exe" ) )
		{
			std::ofstream ShaderCompilerExe( "ShaderCompiler.exe", std::ios::binary | std::ios::out );

		    if ( !ShaderCompilerExe.is_open() )
		    {
			    FailedToWriteShaderCompilerExe = true;
		    }
		    else
		    {
		        ShaderCompilerExe.write( ( const char* )ShaderCompiler, sizeof( ShaderCompiler ) );
		    }
		}

		if ( FailedToWriteShaderCompilerExe )
		{
			Error::SetLastError( Error_ShaderCompilerWriteFailure );
			return false;
		}

		HANDLE PipeRead, PipeWrite;
		SECURITY_ATTRIBUTES SecurityAttributes = { sizeof( SECURITY_ATTRIBUTES ) };
		SecurityAttributes.bInheritHandle = true;
		SecurityAttributes.lpSecurityDescriptor = nullptr;

		// Create a pipe for the child process's STDOUT
		if ( !CreatePipe( &PipeRead, &PipeWrite, &SecurityAttributes, 0 ) ) 
		{
			Error::SetLastError( Error_ShaderCompilerPipeCreationFailure );
		    return false;
		}

		// Ensure the read handle to the pipe for STDOUT is not inherited
		if ( !SetHandleInformation( PipeRead, HANDLE_FLAG_INHERIT, 0 ) )
		{
			Error::SetLastError( Error_ShaderCompilerProcessCreationFailure );
			return false;
		}

		std::string ShaderCompilerCmd = std::vformat( "ShaderCompiler.exe --source \"{}\" --output \"{}\"", std::make_format_args( a_SourceFile, a_OutputFile ) );
		STARTUPINFOA StartupInfo;
		ZeroMemory( &StartupInfo, sizeof( StartupInfo ) );
		StartupInfo.cb = sizeof( StartupInfo );
		StartupInfo.hStdOutput = PipeWrite;
		StartupInfo.hStdError = PipeWrite;
		StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION ProcessInfo;
		ZeroMemory( &ProcessInfo, sizeof( ProcessInfo ) );

		if ( !CreateProcessA(
		    nullptr,
		    ShaderCompilerCmd.data(),
		    nullptr,
		    nullptr,
		    true,
		    CREATE_NEW_CONSOLE,
		    nullptr,
		    nullptr,
		    &StartupInfo,
		    &ProcessInfo ) )
		{
			Error::SetLastError( Error_ShaderCompilerProcessCreationFailure );
			ENSURE_LOG( CloseHandle( PipeWrite ), "Failed to close write pipe for ShaderCompiler.exe." );
			ENSURE_LOG( CloseHandle( PipeRead ), "Failed to close read pipe for ShaderCompiler.exe." );
			return false;
		}
		
		ENSURE_LOG( WaitForSingleObject( ProcessInfo.hProcess, -1 ) == WAIT_OBJECT_0, "Failed to wait for ShaderCompiler.exe to complete." );
		ENSURE_LOG( CloseHandle( PipeWrite ), "Failed to close write pipe for ShaderCompiler.exe." );

		DWORD BytesRead;
		ShaderInfoLogLength = 0u;
		char* BufferHead = ShaderInfoLog;
		while ( ReadFile( PipeRead, BufferHead, SHADER_INFO_LOG_MAX - 1 - ShaderInfoLogLength, &BytesRead, nullptr ) != false ) {
			ShaderInfoLogLength += BytesRead;
		    ShaderInfoLog[ ShaderInfoLogLength ] = '\0';
		    BufferHead += BytesRead;
		}

		ENSURE_LOG( CloseHandle( PipeRead ), "Failed to close ShaderCompiler write pipe." );
		ENSURE_LOG( CloseHandle( ProcessInfo.hProcess ) && CloseHandle( ProcessInfo.hThread ), "Failed to close ShaderCompiler process." );
		
		return true;
	};
#endif

	auto MakeProcInfo = []( const bool a_IsMemoryLoaded, const void* a_Handle, const void* a_Entry, const void* a_Info )
	{
		ShaderProcInfo* ProcInfo = new ShaderProcInfo;
		ProcInfo->IsMemoryLoaded = a_IsMemoryLoaded;
		ProcInfo->Handle = a_Handle;
		ProcInfo->Entry = a_Entry;
		ProcInfo->Info = a_Info;

		return std::shared_ptr< ShaderProcInfo >( ProcInfo, +[]( const ShaderProcInfo* a_ProcInfo )
		{
			if ( a_ProcInfo->IsMemoryLoaded )
			{
				MemoryFreeLibrary( ( HMEMORYMODULE )a_ProcInfo->Handle );
			}
			else
			{
				FreeLibrary( ( HMODULE )a_ProcInfo->Handle );
			}
		} );
	};
		
	switch ( a_Shader->SourceType )
	{
	case EShaderSourceType::MemorySource:
	{
#if IS_CONSOLEGL
		// What will the resulting source file name be?
		const std::string SourceFile = "temp_source.cpp";

		// Create the shader source file.
		{
			std::ofstream ShaderFile( SourceFile, std::ios::binary | std::ios::out );

			if ( !ShaderFile.is_open() )
			{
				Error::SetLastError( Error_ShaderSourceFileCreationFailure );
				return false;
			}

			ShaderFile.write( a_Shader->Source.c_str(), a_Shader->Source.size() );
		}

		a_Shader->File = SourceFile;
#else
		return false;
#endif
	}
	case EShaderSourceType::FileSource:
	{
#if IS_CONSOLEGL
		const std::string SourceFile = a_Shader->File.c_str();
		const std::string OutputFile = std::filesystem::path{ SourceFile }.replace_extension( "cglshader" ).string();

		// Attempt to compile the source file. If it failed, error will be set internally.
		if ( !RunCompiler( SourceFile.c_str(), OutputFile.c_str() ) )
		{
			return false;
		}

		// If this case is a fall through of the above case, then this means the source file is a temp file and
		// should be deleted after being compiled.
		if ( a_Shader->SourceType == EShaderSourceType::MemorySource && !std::filesystem::remove( a_Shader->File ) )
		{
			Error::SetLastError( Error_ShaderSourceFileCleanupFailure );
			return false;
		}

		// Now that the source file has been compiled, we can upgrade this source.
		a_Shader->File = OutputFile;
#else
		return false;
#endif
	}
	case EShaderSourceType::FileCompiled:
	{
		// If the type is FileCompiled, then we want to just load the library directly from the file, because this was a user-provided file.
		if ( a_Shader->SourceType == EShaderSourceType::FileCompiled )
		{
			const HMODULE ShaderHandle = LoadLibraryA( a_Shader->File.c_str() );

			if ( !ShaderHandle )
			{
				Error::SetLastError( Error_ShaderBinaryLoadFailure );
				return false;
			}

			const void* ShaderEntry = GetProcAddress( ShaderHandle, "run" );
			
			if ( !ShaderEntry )
			{
				Error::SetLastError( Error_ShaderEntryNotFound );
				return false;
			}

			const void* ShaderInfo = GetProcAddress( ShaderHandle, "info" );

			if ( !ShaderInfo )
			{
				Error::SetLastError( Error_ShaderInfoNotFound );
				return false;
			}

			// Setup proc info.
			a_Shader->Proc = MakeProcInfo( false, ShaderHandle, ShaderEntry, ShaderInfo );
			return true;
		}

		// If the type if FileSource or MemorySource, this means we've made a temporarily compiled file to load binary from, and delete after.
		{
			std::ifstream ShaderFile( a_Shader->File, std::ios::binary | std::ios::in );

			if ( !ShaderFile.is_open() )
			{
				Error::SetLastError( Error_ShaderBinaryFileLoadFailure );
				return false;
			}

			const size_t ShaderFileSize = std::filesystem::file_size( a_Shader->File );
			a_Shader->Binary.resize( ShaderFileSize );
			
			ShaderFile.read( ( char* )a_Shader->Binary.data(), ShaderFileSize );
			a_Shader->Buffer = a_Shader->Binary.data();
			a_Shader->BufferSize = ShaderFileSize;
		}

		// We have loaded the binary, now we can delete the compiled temp file.
		if ( !std::filesystem::remove( a_Shader->File ) )
		{
			Error::SetLastError( Error_ShaderBinaryFileCleanupFailure );
			return false;
		}

		a_Shader->File = "";
	}
	case EShaderSourceType::MemoryCompiled:
	{
		const HMEMORYMODULE ShaderHandle = MemoryLoadLibrary( a_Shader->Buffer, a_Shader->BufferSize );
		
		if ( !ShaderHandle )
		{
			Error::SetLastError( Error_ShaderBinaryLoadFailure );
			return false;
		}

		const void* ShaderEntry = MemoryGetProcAddress( ShaderHandle, "run" );

		if ( !ShaderEntry )
		{
			Error::SetLastError( Error_ShaderEntryNotFound );
			return false;
		}
		
		const void* ShaderInfo = MemoryGetProcAddress( ShaderHandle, "info" );

		if ( !ShaderInfo )
		{
			Error::SetLastError( Error_ShaderInfoNotFound );
			return false;
		}

		a_Shader->Proc = MakeProcInfo( true, ShaderHandle, ShaderEntry, ShaderInfo );
		return true;
	}
	default: break;
	}

	return true;
}

bool ConsoleGL::IsShaderCompiled( const ShaderHandle a_Shader )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	return a_Shader->Proc.get();
}

const char* ConsoleGL::GetShaderInfoLog()
{
	return ShaderInfoLog;
}

size_t ConsoleGL::GetShaderInfoLogLength()
{
	return ShaderInfoLogLength;
}

ConsoleGL::ShaderProgram* ConsoleGL::CreateShaderProgram()
{
	return State.ShaderPrograms.Create();
}

bool ConsoleGL::AttachShader( const ShaderProgramHandle a_ShaderProgram, const ShaderHandle a_Shader )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	if ( a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramAlreadyLinked );
		return false;
	}

	auto& Entry = a_ShaderProgram->Entries[ ( size_t )a_Shader->Type ];

	// If there is already an attached shader at this location, we must first detach it.
	if ( Entry.Attached )
	{
		DetachShader( a_ShaderProgram, Entry.Attached );
	}

	// Now we can attach the shader.
	Entry.Attached = a_Shader;
	a_Shader->AttachedTo.push_back( a_ShaderProgram );
	return true;
}

bool ConsoleGL::LinkProgram( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	if ( a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramAlreadyLinked );
		return false;
	}

	ClearShaderInfoLog();
	bool LinkFailure = false;

	// We need to verify the state of all shader entries.
	// For entries that are mandatory, there needs to be one,
	// and for all entries that are there, they need to be compiled.
	for ( size_t i = 0; i < 2u; ++i )
	{
		const auto& Entry = a_ShaderProgram->Entries[ i ];

		switch ( ( EShaderType )i )
		{
		case EShaderType::Vertex:
		{
			if ( !Entry.Attached ) LinkFailure = true, WriteToShaderInfoLog( "No vertex shader attached.\n" );
			else if ( !IsShaderCompiled( Entry.Attached ) ) LinkFailure = true, WriteToShaderInfoLog( "Vertex shader is not compiled.\n" );
			break;
		}
		case EShaderType::Fragment:
		{
			if ( !Entry.Attached ) LinkFailure = true, WriteToShaderInfoLog( "No fragment shader attached.\n" );
			else if ( !IsShaderCompiled( Entry.Attached ) ) LinkFailure = true, WriteToShaderInfoLog( "Fragment shader is not compiled.\n" );
			break;
		}
		}
	}

	if ( LinkFailure )
	{
		Error::SetLastError( Error_ShaderProgramLinkFailure );
		return false;
	}

	// We can now copy across proc infos.
	for ( auto& Entry : a_ShaderProgram->Entries )
	{
		Entry.Proc = Entry.Attached->Proc;
	}

	return a_ShaderProgram->IsLinked = true;
}

size_t ConsoleGL::GetAttachedShaderCount( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	size_t Total = 0u;

	for ( const auto& Entry : a_ShaderProgram->Entries )
		if ( Entry.Attached ) 
			++Total;

	return Total;
}

bool ConsoleGL::GetAttachedShaders( ShaderProgram* a_ShaderProgram, ShaderHandle* a_Shaders )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	if ( !a_Shaders )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	size_t Total = 0u;

	for ( const auto& Entry : a_ShaderProgram->Entries )
		if ( Entry.Attached )
			a_Shaders[ Total++ ] = Entry.Attached;

	return true;
}

bool ConsoleGL::DetachShader( const ShaderProgramHandle a_ShaderProgram, const ShaderHandle a_Shader )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	auto& Entry = a_ShaderProgram->Entries[ ( size_t )a_Shader->Type ];

	// Can't detach this shader if it's not the attached shader.
	if ( Entry.Attached != a_Shader )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	Entry.Attached = nullptr;

	// Remove this program from the shaders list of attached to programs.
	if ( const auto Iter = std::find( a_Shader->AttachedTo.begin(), a_Shader->AttachedTo.end(), a_ShaderProgram ); Iter != a_Shader->AttachedTo.end() )
	{
		a_Shader->AttachedTo.erase( Iter );
	}

	return true;
}

bool ConsoleGL::DetachShaderByType( const ShaderProgramHandle a_ShaderProgram, const EShaderType a_ShaderType )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	auto& Entry = a_ShaderProgram->Entries[ ( size_t )a_ShaderType ];

	if ( !Entry.Attached )
	{
		Error::SetLastError( Error_ShaderEntryNotFound );
		return false;
	}

	// Remove this program from the shaders list of attached to programs.
	if ( const auto Iter = std::find( Entry.Attached->AttachedTo.begin(), Entry.Attached->AttachedTo.end(), a_ShaderProgram ); Iter != Entry.Attached->AttachedTo.end() )
	{
		Entry.Attached->AttachedTo.erase( Iter );
	}

	Entry.Attached = nullptr;
	return true;
}

bool ConsoleGL::DeleteShader( const ShaderHandle a_Shader )
{
	if ( !a_Shader || !State.Shaders.IsValid( a_Shader ) )
	{
		Error::SetLastError( Error_InvalidShaderHandle );
		return false;
	}

	// Go through and remove this shader from all programs.
	for ( const ShaderProgramHandle ShaderProgram : a_Shader->AttachedTo )
	{
		ShaderProgram->Entries[ ( size_t )a_Shader->Type ].Attached = nullptr;
	}

	return State.Shaders.Destroy( a_Shader );
}

bool ConsoleGL::DeleteProgram( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram || !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	for ( auto& Entry : a_ShaderProgram->Entries )
	{
		if ( Entry.Attached )
		{
			// Remove this program from the shaders list of attached to programs.
			if ( const auto Iter = std::find( Entry.Attached->AttachedTo.begin(), Entry.Attached->AttachedTo.end(), a_ShaderProgram ); Iter != Entry.Attached->AttachedTo.end() )
			{
				Entry.Attached->AttachedTo.erase( Iter );
			}
		}

		Entry.Attached = nullptr;
	}

	return State.ShaderPrograms.Destroy( a_ShaderProgram );
}

#pragma endregion
























void ConsoleGL::RunTest()
{
	int Width = 100, Height = 100;

	// Create window.
	ConsoleGL::Window* window0 = ConsoleGL::CreateWindow( "window0", Width, Height, 8, 8, 2 );
	ConsoleGL::SetActiveWindow( window0 );
	ConsoleGL::SetWindowColoursFromSet( ConsoleGL::EColourSet::DEFAULT );

	// Create a graphics context.
	ConsoleGL::Context* Context = ConsoleGL::CreateContext();
	ConsoleGL::SetActiveContext( Context );

	// Create a shader program.
	ConsoleGL::ShaderProgram* DefaultShaderProgram = ConsoleGL::CreateShaderProgram();

	// Create vertex shader.
	ConsoleGL::Shader* DefaultVertexShader = ConsoleGL::CreateShader( ConsoleGL::EShaderType::Vertex );
	const std::string VertexShaderSource = R"(
struct Layout
{
	mat4 uni_PV;
	mat4 uni_M;
	vec4 in_ddPosition;
	vec4 out_Position;
	//vec4 out_Position;
	vec2 attr10_Rotations;
	vec4 out_FragColour;
};

extern "C" __declspec(dllexport) void run( Layout* layout )
{
	layout->out_Position = layout->uni_PV * layout->uni_M * layout->out_FragColour;
}
)";

	ConsoleGL::SetShaderSourceFromString( DefaultVertexShader, VertexShaderSource.c_str(), VertexShaderSource.size() );
	ConsoleGL::CompileShader( DefaultVertexShader );
	ConsoleGL::AttachShader( DefaultShaderProgram, DefaultVertexShader );

	// Create fragment shader.
	ConsoleGL::Shader* DefaultFragmentShader = ConsoleGL::CreateShader( ConsoleGL::EShaderType::Fragment );
	const std::string FragmentShaderSource = R"(
struct Layout
{
	mat4 uni_PV;
	mat4 uni_M;
	vec4 in_ddPosition;
	vec4 out_Position;
	//vec4 out_Position;
	vec2 attr10_Rotations;
	vec4 out_FragColour;
};

extern "C" __declspec(dllexport) void run( Layout* layout )
{
	layout->out_Position = layout->uni_PV * layout->uni_M * layout->out_FragColour;
}
)";

	ConsoleGL::SetShaderSourceFromString( DefaultFragmentShader, FragmentShaderSource.c_str(), FragmentShaderSource.size() );
	ConsoleGL::CompileShader( DefaultFragmentShader );
	ConsoleGL::AttachShader( DefaultShaderProgram, DefaultFragmentShader );

	// Link the shader program.
	ConsoleGL::LinkProgram( DefaultShaderProgram );
	ConsoleGL::DetachShader( DefaultShaderProgram, DefaultVertexShader );
	ConsoleGL::DeleteShader( DefaultVertexShader );
	ConsoleGL::DetachShader( DefaultShaderProgram, DefaultFragmentShader );
	ConsoleGL::DeleteShader( DefaultFragmentShader );

	struct Field
	{
		const char* Name;
		uint64_t Offset;
		uint64_t Size;
		uint16_t DataType;
		uint16_t IOType;
		uint16_t BuiltinVar;
		uint16_t Slot;
	};
	
	struct LayoutInfo
	{
		Field* Fields;
		size_t FieldCount;
	};

	using func_type = LayoutInfo*(*)();

	func_type func = (func_type)DefaultShaderProgram->Entries[(size_t)EShaderType::Fragment].Proc->Info;

	auto* f = func();

	for ( int i = 0; i < f->FieldCount; ++i )
	{
		f->Fields[ i ].Name;
	}

	while ( true )
	{
		/*float Colourf[3];
		func(Colourf);

		Colour col{ Colourf[ 0 ], Colourf[ 1 ], Colourf[ 2 ] };
		Pixel pix = *MapColourToPixel( col );

		auto buff0 = GetWindowBuffer(window0);
		SetBuffer(buff0, pix);
		SwapWindowBuffer();*/
	}
}