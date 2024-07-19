#include <array>
#include <optional>
#include <map>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <bitset>
#include <set>

#include <ConsoleGL.hpp>
#include <Error.hpp>
#include <Event.hpp>
#include <FileMap.hpp>
#include <PixelBuffer.hpp>

#include <memory_module/MemoryModule.hpp>

// Disable dll exported functions.
#define _USER32_
#include <Windows.h>
#include <synchapi.h>
#undef CreateWindow

#if IS_CONSOLEGL
#include <ConsoleDock.inl>
#include <PixelMap.inl>
#include <ShaderCompiler.inl>
#endif

#define SHADER_STAGE_COUNT 2u

// To do
// Move error into this cpp.
// Make all function calls give Error_NoError if they succeed.
// Really think about vin,vout etc
// strip down shader compiler to only deal with strings and not validate anything
// move shader field validation into cpp
// make shaderinfo logging better, strip down shadercompiler txt.
// properly define the destructors so that things are cleaned up at the end.

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

char	ShaderProgramInfoLog[ SHADER_PROGRAM_INFO_LOG_MAX ];
size_t	ShaderProgramInfoLogLength = 0u;

void ClearShaderInfoLog()
{
	ZeroMemory( ShaderInfoLog, sizeof( ShaderInfoLog ) );
	ShaderInfoLogLength = 0u;
}

void ClearShaderProgramInfoLog()
{
	ZeroMemory( ShaderProgramInfoLog, sizeof( ShaderProgramInfoLog ) );
	ShaderProgramInfoLogLength = 0u;
}

void WriteToShaderInfoLog( const char* a_String, ... )
{
	va_list VaList;
	va_start( VaList, a_String );
	const size_t Length = vsprintf_s( ShaderInfoLog + ShaderInfoLogLength, sizeof( ShaderInfoLog ) - 1 - ShaderInfoLogLength, a_String, VaList );
	va_end( VaList );
	ShaderInfoLogLength += Length;
}

void WriteToShaderProgramInfoLog( const char* a_String, ... )
{
	va_list VaList;
	va_start( VaList, a_String );
	const size_t Length = vsprintf_s( ShaderProgramInfoLog + ShaderProgramInfoLogLength, sizeof( ShaderProgramInfoLog ) - 1 - ShaderProgramInfoLogLength, a_String, VaList );
	va_end( VaList );
	ShaderProgramInfoLogLength += Length;
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
	~Name##Repository() \
	{ \
		for ( auto& Instance : Instances ) \
		{ \
			if ( Instance.has_value() ) \
			{ \
				Destructor##Name( &Instance.value() ); \
			} \
		} \
	} \
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

	struct Window
	{
	    uint32_t					    Width;
	    uint32_t					    Height;
	    uint32_t					    PixelWidth;
	    uint32_t					    PixelHeight;
	    std::vector< PixelBuffer >	    Buffers;
	    uint32_t					    ActiveBuffer;
	    std::string					    Title;
	    FileMap                         CommandBuffer;
		Event                           CommandReady;
	    Event                           CommandComplete;
	    Event                           ProcessStarted;
		HWND							WindowHandle;
	    STARTUPINFOA					StartupInfo;
	    PROCESS_INFORMATION				ProcessInfo;
	};

	enum class EShaderSourceType
	{
		None,
		FileSource,
		FileCompiled,
		MemorySource,
		MemoryCompiled
	};

	enum class EShaderVarType
	{
		Custom,
		Int8,
		Int16,
		Int32,
		Int64,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Float,
		Double,
		Bool,
		Mat2,
		Mat3,
		Mat4,
		Mat2x3,
		Mat2x4,
		Mat3x2,
		Mat3x4,
		Mat4x2,
		Mat4x3,
		Dmat2,
		Dmat3,
		Dmat4,
		Dmat2x3,
		Dmat2x4,
		Dmat3x2,
		Dmat3x4,
		Dmat4x2,
		Dmat4x3,
		Vec2,
		Vec3,
		Vec4,
		Dvec2,
		Dvec3,
		Dvec4,
		Ivec2,
		Ivec3,
		Ivec4,
		Uvec2,
		Uvec3,
		Uvec4
	};

	enum class EShaderInterpQual
	{
		None,
		Flat,
		Affine,
		Perspective
	};

	enum class EShaderStorageQual
	{
		In,
		Out,
		Uniform,
		Attribute
	};

	enum class EVertexShaderInbuilt
	{
		// The clip-space output position of the current vertex
		out_vec4_VertPosition,

		// Not an inbuilt type.
		None
	};

	enum class EFragmentShaderInbuilt
	{
		// The location of the fragment in window space
		in_vec4_FragCoord,
	
		// The colour to set the fragment
		out_vec4_FragColour,
	
		// The depth to assign to the fragment.
		out_float_FragDepth,
		
		// Not an inbuilt type.
		None
	};

	struct ShaderVariableInfo
	{
		const char* InterpQual = nullptr;
		const char* StorageQual = nullptr;
		const char* Type = nullptr;
		const char* Name = nullptr;
		const void* Data = nullptr;
		uint64_t	Size = 0u;
		uint64_t	Length = 1u;
		uint64_t	Location = -1;
	};
	
	struct ShaderInfo
	{
		ShaderVariableInfo* Variables;
		size_t				VariableCount;
	};

	using ShaderProcRunFn	= void(*)();
	using ShaderProcInfoFn	= ShaderInfo*(*)();

	struct ShaderProcInfo
	{
		const void*				Handle;
		ShaderProcRunFn			Run;
		const ShaderInfo*		Info;
	};

	struct ShaderVariable
	{
		EShaderInterpQual	InterQual;
		EShaderStorageQual	StorageQual;
		EShaderVarType		VarType;
		std::string			Name;
		void*				Data;
		uint32_t			Location;
		uint32_t			Size;
		uint32_t			Length;
	};

	using ShaderUniformArray	= std::array< std::optional< ShaderVariable	>, MAX_SHADER_UNIFORMS	 >;
	using ShaderAttributeArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_ATTRIBUTES >;
	using ShaderParameterArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_PARAMETERS >;

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
		ShaderUniformArray					Uniforms;
		ShaderAttributeArray				Attributes;
		ShaderParameterArray				Parameters;
	};

	using ShaderProgramUniformArray		= std::array< std::optional< ShaderVariable	>, MAX_SHADER_UNIFORMS	 >;
	using ShaderProgramAttributeArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_ATTRIBUTES >;
	using ShaderProgramParameterArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_PARAMETERS >;

	using VertexInbuiltVarArray		= std::array< void*, ( size_t )EVertexShaderInbuilt::None >;
	using FragmentInbuiltVarArray	= std::array< void*, ( size_t )EFragmentShaderInbuilt::None >;

	struct ShaderProgramEntry
	{
		std::shared_ptr< ShaderProcInfo > Proc;
		ShaderHandle					  Attached;
	};

	struct ShaderProgram
	{
		bool						IsLinked;
		ShaderProgramEntry			Entries[ SHADER_STAGE_COUNT ];
		ShaderProgramUniformArray	Uniforms;
		ShaderProgramAttributeArray Attributes;
		ShaderProgramParameterArray Parameters;
		VertexInbuiltVarArray		VertexInbuiltVars;
		FragmentInbuiltVarArray		FragmentInbuiltVars;
	};

	struct Context
	{
		ShaderProgramHandle	ActiveShaderProgram;
		ShaderProcRunFn		ShaderEntries[ SHADER_STAGE_COUNT ];
	};
}

bool ConvertShaderVar( const ConsoleGL::ShaderVariableInfo& a_ShaderVarInfo, ConsoleGL::ShaderVariable& o_ShaderVar )
{
	const std::string InterpQual = a_ShaderVarInfo.InterpQual;

	if ( InterpQual == "" ) o_ShaderVar.InterQual = ConsoleGL::EShaderInterpQual::None;
	else if ( InterpQual == "affn" ) o_ShaderVar.InterQual = ConsoleGL::EShaderInterpQual::Affine;
	else if ( InterpQual == "flat" ) o_ShaderVar.InterQual = ConsoleGL::EShaderInterpQual::Flat;
	else if ( InterpQual == "prsp" ) o_ShaderVar.InterQual = ConsoleGL::EShaderInterpQual::Perspective;
	else return false;

	const std::string StorageQual = a_ShaderVarInfo.StorageQual;

	if ( StorageQual == "" ) return false;
	else if ( StorageQual == "out" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Out;
	else if ( StorageQual == "in" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::In;
	else if ( StorageQual == "attrib" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Attribute;
	else if ( StorageQual == "uniform" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Uniform;
	else return false;

	// If there is a interpolation qualifier, then the storage qualifier must be out.
	if ( o_ShaderVar.InterQual != ConsoleGL::EShaderInterpQual::None && o_ShaderVar.StorageQual != ConsoleGL::EShaderStorageQual::Out )
	{
		return false;
	}

	// If there is no interpolation qualifier and it's an out variable, then we use the default, perspective.
	if ( o_ShaderVar.InterQual == ConsoleGL::EShaderInterpQual::None && o_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::Out )
	{
		o_ShaderVar.InterQual = ConsoleGL::EShaderInterpQual::Perspective;
	}

	o_ShaderVar.Location = a_ShaderVarInfo.Location == ( uint64_t )-1 ? -1 : a_ShaderVarInfo.Location;

	// Validate that if there is a location, then it is within range.
	switch ( o_ShaderVar.StorageQual )
	{
	case ConsoleGL::EShaderStorageQual::In:
	case ConsoleGL::EShaderStorageQual::Out:
		if ( o_ShaderVar.Location != -1 )
		{
			return false;
		}
		break;
	case ConsoleGL::EShaderStorageQual::Uniform:
		if ( o_ShaderVar.Location >= MAX_SHADER_UNIFORMS )
		{
			return false;
		}
		break;
	case ConsoleGL::EShaderStorageQual::Attribute:
		if ( o_ShaderVar.Location >= MAX_SHADER_ATTRIBUTES )
		{
			return false;
		}
		break;
	default: break;
	}

	const std::string VarType = a_ShaderVarInfo.Type;

	if ( VarType == "" ) return false;
	else if ( VarType == "int8"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Int8;
	else if ( VarType == "int16"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Int16;
	else if ( VarType == "int32"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Int32;
	else if ( VarType == "int64"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Int64;
	else if ( VarType == "uint8"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uint8;
	else if ( VarType == "uint16"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uint16;
	else if ( VarType == "uint32"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uint32;
	else if ( VarType == "uint64"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uint64;
	else if ( VarType == "float"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Float;
	else if ( VarType == "double"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Double;
	else if ( VarType == "bool"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Bool;
	else if ( VarType == "mat2"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat2;
	else if ( VarType == "mat3"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat3;
	else if ( VarType == "mat4"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat4;
	else if ( VarType == "mat2x3"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat2x3;
	else if ( VarType == "mat2x4"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat2x4;
	else if ( VarType == "mat3x2"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat3x2;
	else if ( VarType == "mat3x4"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat3x4;
	else if ( VarType == "mat4x2"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat4x2;
	else if ( VarType == "mat4x3"  ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Mat4x3;
	else if ( VarType == "dmat2"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat2;
	else if ( VarType == "dmat3"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat3;
	else if ( VarType == "dmat4"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat4;
	else if ( VarType == "dmat2x3" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat2x3;
	else if ( VarType == "dmat2x4" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat2x4;
	else if ( VarType == "dmat3x2" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat3x2;
	else if ( VarType == "dmat3x4" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat3x4;
	else if ( VarType == "dmat4x2" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat4x2;
	else if ( VarType == "dmat4x3" ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dmat4x3;
	else if ( VarType == "vec2"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Vec2;
	else if ( VarType == "vec3"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Vec3;
	else if ( VarType == "vec4"    ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Vec4;
	else if ( VarType == "dvec2"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dvec2;
	else if ( VarType == "dvec3"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dvec3;
	else if ( VarType == "dvec4"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Dvec4;
	else if ( VarType == "ivec2"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Ivec2;
	else if ( VarType == "ivec3"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Ivec3;
	else if ( VarType == "ivec4"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Ivec4;
	else if ( VarType == "uvec2"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uvec2;
	else if ( VarType == "uvec3"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uvec3;
	else if ( VarType == "uvec4"   ) o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Uvec4;
	else o_ShaderVar.VarType = ConsoleGL::EShaderVarType::Custom;

	o_ShaderVar.Name = a_ShaderVarInfo.Name;

	if ( o_ShaderVar.Name == "" )
	{
		return false;
	}

	o_ShaderVar.Data = const_cast< void* >( a_ShaderVarInfo.Data );

	if ( o_ShaderVar.Data == nullptr )
	{
		return false;
	}

	o_ShaderVar.Size = a_ShaderVarInfo.Size;

	if ( o_ShaderVar.Size == 0u )
	{
		return false;
	}

	o_ShaderVar.Length = a_ShaderVarInfo.Length;

	if ( o_ShaderVar.Length == 0u )
	{
		return false;
	}

	return true;
}

ConsoleGL::EVertexShaderInbuilt TryConvertVertexShaderInbuilt( const ConsoleGL::ShaderVariable& a_ShaderVar )
{
	return ConsoleGL::EVertexShaderInbuilt::None;
}

// Define destructor functions for each object.
void DestructorWindow( ConsoleGL::Window* a_Window ) {}
void DestructorContext( ConsoleGL::Context* a_Context ) {}
void DestructorShader( ConsoleGL::Shader* a_Shader ) {}
void DestructorShaderProgram( ConsoleGL::ShaderProgram* a_ShaderProgram ) {}

// Define repositories for each object.
DEFINE_REPOSITORY( Window, WINDOW_MAX_COUNT );
DEFINE_REPOSITORY( Context, CONTEXT_MAX_COUNT );
DEFINE_REPOSITORY( Shader, SHADER_MAX_COUNT );
DEFINE_REPOSITORY( ShaderProgram, SHADER_PROGRAM_MAX_COUNT );

struct
{
	WindowRepository				Windows;
	ContextRepository				Contexts;
	ShaderRepository				Shaders;
	ShaderProgramRepository			ShaderPrograms;
	ConsoleGL::WindowHandle			ActiveWindow;
	ConsoleGL::ContextHandle		ActiveContext;
} static State;

#pragma region Window functions

bool ReturnWindow( const ConsoleGL::WindowHandle a_Window )
{
	if ( State.ActiveWindow != a_Window )
    {
	    return true;
    }

    ConsoleGL::WindowDockCommandBuffer* Buffer = ( ConsoleGL::WindowDockCommandBuffer* )a_Window->CommandBuffer.Data();
    Buffer->Command = ConsoleGL::EWindowDockCommand::Attach;
    Buffer->Value = ( uint32_t )GetCurrentProcessId();

    ENSURE_LOG( !( !a_Window->CommandReady.Trigger() || !a_Window->CommandComplete.Wait() ), "Failed to trigger console dock attach." );
    ENSURE_LOG( FreeConsole(), "Failed to free console window." );

    State.ActiveWindow = nullptr;

    return true;
}

bool BorrowWindow( const ConsoleGL::WindowHandle a_Window )
{
	if ( State.ActiveWindow == a_Window )
    {
	    return true;
    }

    ENSURE_LOG( !( State.ActiveWindow && !ReturnWindow( State.ActiveWindow ) ), "Failed to return currently borrowed console window." );
	ENSURE_LOG( AttachConsole( a_Window->ProcessInfo.dwProcessId ), "Failed to attach console window." );

    // Set command.
    ConsoleGL::WindowDockCommandBuffer* Buffer = ( ConsoleGL::WindowDockCommandBuffer* )a_Window->CommandBuffer.Data();
    Buffer->Command = ConsoleGL::EWindowDockCommand::Release;
    
    ENSURE_LOG( a_Window->CommandReady.Trigger(), "Failed to trigger console dock release." );
    ENSURE_LOG( a_Window->CommandComplete.Wait(), "Failed to wait for console dock release." );
    
    State.ActiveWindow = a_Window;
    return true;
}


ConsoleGL::Window* ConsoleGL::CreateWindow( const char* a_Title, const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_PixelWidth, const uint32_t a_PixelHeight, const uint32_t a_BufferCount )
{
#if IS_CONSOLEGL
	Window* Created = State.Windows.Create();

	if ( !Created )
	{
		return nullptr;
	}

	const std::string Prefix = std::to_string( std::chrono::high_resolution_clock::now().time_since_epoch().count() );

    // Create a command buffer.
    if ( !Created->CommandBuffer.Create( Prefix + "_CommandBuffer", sizeof( Created->CommandBuffer ) ) )
    {
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
        return nullptr;
    }

    // Create the command ready event.
    if ( !Created->CommandReady.Create( Prefix + "_CommandReady", true ) )
    {
        Created->CommandBuffer.Clear();
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
        return nullptr;
    }
    
    // Create the command complete event.
    if ( !Created->CommandComplete.Create( Prefix + "_CommandComplete", true ) )
    {
        Created->CommandBuffer.Clear();
        Created->CommandReady.Clear();
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
        return nullptr;
    }

    if ( !Created->ProcessStarted.Create( Prefix + "_ProcessStarted" ) )
    {
	    Created->CommandBuffer.Clear();
        Created->CommandReady.Clear();
        Created->CommandComplete.Clear();
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
        return nullptr;
    }

    ZeroMemory( &Created->StartupInfo, sizeof( Created->StartupInfo ) );
    Created->StartupInfo.cb = sizeof( Created->StartupInfo );
    ZeroMemory( &Created->ProcessInfo, sizeof( Created->ProcessInfo ) );

    bool FailedToWriteConsoleDockExe = false;

    if ( !std::filesystem::exists( "ConsoleDock.exe" ) )
    {
    	std::ofstream ConsoleDockExe( "ConsoleDock.exe", std::ios::binary | std::ios::out );

        if ( !ConsoleDockExe.is_open() )
	    {
		    FailedToWriteConsoleDockExe = true;
	    }
        else
        {
	        ConsoleDockExe.write( ( const char* )ConsoleDock, sizeof( ConsoleDock ) );
        }
    }
	if (
        FailedToWriteConsoleDockExe ||
        !CreateProcessA(
        nullptr,
        ( "ConsoleDock.exe " + Prefix ).data(),
        nullptr,
        nullptr,
        true,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &Created->StartupInfo,
        &Created->ProcessInfo ) )

    {
	    Created->CommandBuffer.Clear();
        Created->CommandReady.Clear();
        Created->CommandComplete.Clear();
        Created->ProcessStarted.Clear();
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
        return nullptr;
    }

    if ( !Created->ProcessStarted.Wait() )
    {
	    Created->CommandBuffer.Clear();
        Created->CommandReady.Clear();
        Created->CommandComplete.Clear();
        Created->ProcessStarted.Clear();

        ENSURE_LOG( TerminateProcess( Created->ProcessInfo.hProcess, 1 ), "Failed to terminate console dock process." );
		State.Windows.Destroy( Created );
        Error::SetLastError( Error_WindowDockCreationFailure );
    }

    // Temporarily return any currently borrowed window.
    //const WindowDock* PreviouslyBorrowed = WindowDock::s_CurrentlyBorrowed;
	const WindowHandle PreviouslyActive = State.ActiveWindow;

    // Borrow the new docked window.
	ENSURE_LOG( BorrowWindow( Created ), "Could not borrow window." );
    Created->WindowHandle = GetConsoleWindow();

    const HANDLE StdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    const HANDLE StdInputHandle = GetStdHandle( STD_INPUT_HANDLE );

    // Change console visual size to a minimum so ScreenBuffer can shrink
	// below the actual visual size
	SMALL_RECT WindowRect = { 0, 0, 1, 1 };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );

    // Set the size of the screen buffer
	ENSURE_LOG( SetConsoleScreenBufferSize( StdOutputHandle, { ( SHORT )a_Width, ( SHORT )a_Height } ), "Failed to set console screen buffer size." );

    // Assign screen buffer to the console
	ENSURE_LOG( SetConsoleActiveScreenBuffer( StdOutputHandle ), "Failed to set console screen buffer." );

    // Set console font.
    CONSOLE_FONT_INFOEX FontInfo;
    ZeroMemory( &FontInfo, sizeof( FontInfo ) );
    FontInfo.cbSize = sizeof( FontInfo );
    FontInfo.nFont = 0;
    FontInfo.dwFontSize = { ( SHORT )a_PixelWidth, ( SHORT )a_PixelHeight };
    FontInfo.FontFamily = FF_DONTCARE;
    FontInfo.FontWeight = FW_NORMAL;
	wcscpy_s( FontInfo.FaceName, L"Consolas" );
	ENSURE_LOG( SetCurrentConsoleFontEx( StdOutputHandle, false, &FontInfo ), "Failed to set console font." );

    // Get screen buffer info.
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	ENSURE_LOG( GetConsoleScreenBufferInfo( StdOutputHandle, &ScreenBufferInfo ), "Failed to get console screen buffer info." );
	ENSURE_LOG( a_Width <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.X, "Failed to create console window of with width %u.", a_Width );
	ENSURE_LOG( a_Height <= ( uint32_t )ScreenBufferInfo.dwMaximumWindowSize.Y, "Failed to create console window of with height %u.", a_Height );

    // Set Physical Console Window Size
	WindowRect = { 0, 0, ( SHORT )( a_Width - 1 ), ( SHORT )( a_Height - 1 ) };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );
    ENSURE_LOG( SetConsoleMode( StdInputHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT ), "Failed to set console mode." );
    ENSURE_LOG( SetConsoleTitleA( a_Title ), "Failed to set console title." );
    
    // Set window info.
    Created->Title = a_Title;
    Created->PixelWidth = a_PixelWidth;
    Created->PixelHeight = a_PixelHeight;
    Created->Width = a_Width;
    Created->Height = a_Height;
    Created->Buffers.reserve( a_BufferCount );

    for ( uint32_t i = 0; i < a_BufferCount; ++i )
    {
        Created->Buffers.emplace_back( a_Width, a_Height );
    }

    // Return the new console back to its dock.
    ENSURE_LOG( ReturnWindow( Created ), "Failed to return window." );

    // If there was a previously borrowed window, re-borrow it.
    if ( PreviouslyActive )
    {
        ENSURE_LOG( BorrowWindow( PreviouslyActive ), "Failed to borrow back active window." );
    }

    return Created;
#else
	return nullptr;
#endif
}

bool ConsoleGL::DestroyWindow( const WindowHandle a_Window )
{
	return State.Windows.Destroy( a_Window );
}

bool ConsoleGL::SetActiveWindow( const WindowHandle a_Window )
{
	if ( a_Window && !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return false;
	}

	if ( State.ActiveWindow && !ReturnWindow( State.ActiveWindow ) )
    {
		Error::SetLastError( Error_WindowReturnFailure );
	    return false;
    }

    if ( a_Window && !BorrowWindow( a_Window ) )
    {
		Error::SetLastError( Error_WindowBorrowFailure );
	    return false;
    }

    return true;
}

ConsoleGL::Window* ConsoleGL::GetActiveWindow()
{
	return State.ActiveWindow;
}

bool ConsoleGL::SetWindowTitle( const char* a_Title )
{
	if ( !State.ActiveWindow )
	{
		Error::SetLastError( Error_NoActiveWindow );
		return false;
	}

	if ( !SetConsoleTitleA( a_Title ) )
	{
		Error::SetLastError( Error_WindowSetTitleFailure );
		return false;
	}

	State.ActiveWindow->Title = a_Title;
	return true;

}

bool ConsoleGL::SetWindowColoursFromArray( const Colour* a_Colours )
{
	if ( !State.ActiveWindow )
    {
        Error::SetLastError( Error_NoActiveWindow );
        return false;
    }

    const HANDLE StdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    if ( StdOutputHandle == INVALID_HANDLE_VALUE )
    {
		Error::SetLastError( Error_GenericWindowFailure );
        return false;
    }

    CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
    ScreenBufferInfo.cbSize = sizeof( ScreenBufferInfo );
    ENSURE_LOG( GetConsoleScreenBufferInfoEx( StdOutputHandle, &ScreenBufferInfo ), "Failed to get console window screen buffer info." );

	// Set to all black if nullptr
    if ( !a_Colours )
    {
	    for ( uint8_t i = 0u; i < 16u; ++i )
		{
		    ScreenBufferInfo.ColorTable[ i ] = 0u;
		}
    }
	else
	{
		for ( uint8_t i = 0u; i < 16u; ++i )
		{
		    ScreenBufferInfo.ColorTable[ i ] =
		        ( static_cast< DWORD >( a_Colours[ i ].b ) << 16 ) |
		        ( static_cast< DWORD >( a_Colours[ i ].g ) << 8  ) |
		        ( static_cast< DWORD >( a_Colours[ i ].r )       ) ;
		}
	}

    // Set the screen buffer info.
    ENSURE_LOG( SetConsoleScreenBufferInfoEx( StdOutputHandle, &ScreenBufferInfo ), "Failed to set console window screen buffer info." );

    // Due to weirdness with console windows, we need to set the rect to 1x1 and then back to full size again to get rid of scrollbars caused by the above call.
	SMALL_RECT WindowRect = { 0, 0, 1, 1 };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );
	WindowRect = { 0, 0, ( SHORT )( State.ActiveWindow->Width - 1 ), ( SHORT )( State.ActiveWindow->Height - 1 ) };
	ENSURE_LOG( SetConsoleWindowInfo( StdOutputHandle, true, &WindowRect ), "Failed to set console window info." );

	return true;
}

bool ConsoleGL::SetWindowColoursFromSet( const EColourSet a_ColourSet )
{
	const Colour* Colours = nullptr;

	switch ( a_ColourSet )
    {
    case EColourSet::DEFAULT:
    {
    	static constexpr Colour Default[ 16 ] =
		{
		    { 0,   0,   0   },
		    { 255, 0,   0   },
		    { 0,   255, 0   },
		    { 0,   0,   255 },
		    { 255, 255, 0   },
		    { 255, 0,   255 },
		    { 0,   255, 255 },
		    { 255, 255, 255 },
		    { 85,  85,  85  },
		    { 170, 85,  85  },
		    { 85,  170, 85  },
		    { 85,  85,  170 },
		    { 170, 170, 85  },
		    { 170, 85,  170 },
		    { 85,  170, 170 },
		    { 170, 170, 170 }
		};

        Colours = Default;
        break;
    }
    case EColourSet::GREYSCALE:
    {
		static constexpr Colour Greyscale[ 16 ] =
		{
			{ 0,   0,   0   },
			{ 76,  76,  76  },
			{ 149, 149, 149 },
			{ 29,  29,  29  },
			{ 225, 225, 225 },
			{ 105, 105, 105 },
			{ 178, 178, 178 },
			{ 254, 254, 254 },
			{ 83,  83,  83  },
			{ 108, 108, 108 },
			{ 133, 133, 133 },
			{ 93,  93,  93  },
			{ 158, 158, 158 },
			{ 118, 118, 118 },
			{ 143, 143, 143 },
			{ 168, 168, 168 }
		};
		
        Colours = Greyscale;
        break;
    }
    case EColourSet::SEPIA:
    {
    	static constexpr Colour Sepia[ 16 ] =
		{
		    { 0,   0,   0   },
		    { 100, 88,  69  },
		    { 196, 174, 136 },
		    { 48,  42,  33  },
		    { 255, 255, 205 },
		    { 148, 131, 102 },
		    { 244, 217, 169 },
		    { 255, 255, 238 },
		    { 114, 102, 79  },
		    { 148, 131, 102 },
		    { 180, 160, 125 },
		    { 130, 116, 90  },
		    { 213, 190, 148 },
		    { 164, 146, 113 },
		    { 196, 174, 136 },
		    { 229, 204, 159 }
		};

        Colours = Sepia;
        break;
    }
    }

	if ( !Colours )
	{
		Error::SetLastError( Error_WindowInvalidColourSet );
		return false;
	}

	return SetWindowColoursFromArray( Colours );
}

bool ConsoleGL::SwapWindowBuffer()
{
	if ( !State.ActiveWindow )
    {
    	Error::SetLastError( Error_NoActiveWindow );
	    return false;
    }

    const uint32_t IndexToDraw = State.ActiveWindow->ActiveBuffer;

    if ( State.ActiveWindow->Buffers.size() > 1 )
    {
        ++State.ActiveWindow->ActiveBuffer;

        if ( State.ActiveWindow->ActiveBuffer >= State.ActiveWindow->Buffers.size() )
        {
            State.ActiveWindow->ActiveBuffer = 0u;
        }
    }

	SMALL_RECT WindowRegion = { 0, 0, State.ActiveWindow->Width - 1, State.ActiveWindow->Height - 1 };

    if ( !WriteConsoleOutput(
        GetStdHandle( STD_OUTPUT_HANDLE ),
        ( CHAR_INFO* )State.ActiveWindow->Buffers[ IndexToDraw ].GetPixels(),
        { ( SHORT )State.ActiveWindow->Width, ( SHORT )State.ActiveWindow->Height },
        { 0, 0 },
        &WindowRegion ) )
    {
	    Error::SetLastError( Error_WindowBufferWriteFailure );
		return false;
    }

	return true;
}

bool ConsoleGL::SwapWindowBufferToIndex( const uint32_t a_Index )
{
	if ( !State.ActiveWindow )
    {
    	Error::SetLastError( Error_NoActiveWindow );
	    return false;
    }

	if ( a_Index >= State.ActiveWindow->Buffers.size() )
	{
		Error::SetLastError( Error_InvalidWindowBufferIndex );
		return false;
	}

	State.ActiveWindow->ActiveBuffer = a_Index;
    return SwapWindowBuffer();
}

const char* ConsoleGL::GetWindowTitle( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return "";
	}

	return a_Window->Title.c_str();
}

uint32_t ConsoleGL::GetWindowWidth( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Width;
}

uint32_t ConsoleGL::GetWindowHeight( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Width;
}

uint32_t ConsoleGL::GetWindowBufferIndex( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->ActiveBuffer;
}

uint32_t ConsoleGL::GetWindowBufferCount( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Buffers.size();
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBuffer( const WindowHandle a_Window )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return nullptr;
	}

	return &a_Window->Buffers[ a_Window->ActiveBuffer ];
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBufferByIndex( const WindowHandle a_Window, const uint32_t a_Index )
{
	if ( !a_Window || State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return nullptr;
	}

	if ( a_Index >= State.ActiveWindow->Buffers.size() )
	{
		Error::SetLastError( Error_InvalidWindowBufferIndex );
		return nullptr;
	}

	return &a_Window->Buffers[ a_Index ];
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

uint8_t KeyCodes[ 99 ] = 
{
	0x08,
	0x09,
	0x0D,
	0x10,
	0x11,
	0x12,
	0x14,
	0x1B,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	0x27,
	0x28,
	0x2C,
	0x2D,
	0x2E,
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49,
	0x4A,
	0x4B,
	0x4C,
	0x4D,
	0x4E,
	0x4F,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5A,
	0x70,
	0x71,
	0x72,
	0x73,
	0x74,
	0x75,
	0x76,
	0x77,
	0x78,
	0x79,
	0x7A,
	0x7B,
	0x7C,
	0x7D,
	0x7E,
	0x7F,
	0x80,
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x87,
	0x90,
	0x91,
	0xA0,
	0xA1,
	0xA2,
	0xA3,
	0xA4,
	0xA5,
	0xBA,
	0xBB,
	0xBC,
	0xBD,
	0xBE,
	0xBF,
	0xC0,
	0xDB,
	0xDC,
	0xDD,
	0xDE
};

uint8_t MouseCodes[ 3 ] =
{
	0x01,
	0x02,
	0x04
};

std::bitset< 99 >	KeyStates;
std::bitset< 3  >	MouseStates;
float				MouseX;
float				MouseY;
float				MouseDeltaX;
float				MouseDeltaY;

bool ConsoleGL::IsKeyDown( const EKeyboardKey a_KeyboardKey )
{
	return GetKeyState( KeyCodes[ static_cast< uint8_t >( a_KeyboardKey ) ] ) & 0x8000;
}

bool ConsoleGL::IsKeyUp( const EKeyboardKey a_KeyboardKey )
{
	return !( GetKeyState( KeyCodes[ static_cast< uint8_t >( a_KeyboardKey ) ] ) & 0x8000 );
}

bool ConsoleGL::IsKeyPressed( const EKeyboardKey a_KeyboardKey )
{
	return !KeyStates[ static_cast< uint8_t >( a_KeyboardKey ) ] && IsKeyDown( a_KeyboardKey );
}

bool ConsoleGL::IsKeyReleased( const EKeyboardKey a_KeyboardKey )
{
	return KeyStates[ static_cast< uint8_t >( a_KeyboardKey ) ] && IsKeyUp( a_KeyboardKey );
}

bool ConsoleGL::IsMouseDown( const EMouseButton a_MouseButton )
{
	return GetKeyState( MouseCodes[ static_cast< uint8_t >( a_MouseButton ) ] ) & 0x8000;
}

bool ConsoleGL::IsMouseUp( const EMouseButton a_MouseButton )
{
	return !( GetKeyState( MouseCodes[ static_cast< uint8_t >( a_MouseButton ) ] ) & 0x8000 );
}

bool ConsoleGL::IsMousePressed( const EMouseButton a_MouseButton )
{
	return !MouseStates[ static_cast< uint8_t >( a_MouseButton ) ] && IsMouseDown( a_MouseButton );
}

bool ConsoleGL::IsMouseReleased( const EMouseButton a_MouseButton )
{
	return MouseStates[ static_cast< uint8_t >( a_MouseButton ) ] && IsMouseUp( a_MouseButton );
}

void ConsoleGL::GetMousePosition( float& o_X, float& o_Y )
{
	o_X = MouseX;
	o_Y = MouseY;
}

void ConsoleGL::GetMouseDelta( float& o_X, float& o_Y )
{
	o_X = MouseDeltaX;
	o_Y = MouseDeltaY;
}

bool ConsoleGL::PollEvents()
{
    if ( !State.ActiveWindow )
    {
	    Error::SetLastError( Error_NoActiveWindow );
        return false;
    }

	for ( size_t i = 0u; i < KeyStates.size(); ++i )
	{
		KeyStates[ i ] = GetKeyState( KeyCodes[ i ] ) & 0x8000;
	}

	for ( size_t i = 0u; i < MouseStates.size(); ++i )
	{
		MouseStates[ i ] = GetKeyState( MouseCodes[ i ] ) & 0x8000;
	}

    // Get the current mouse position.
    POINT Coordinates{ 0, 0 };

	if ( !GetCursorPos( &Coordinates ) || !ScreenToClient( State.ActiveWindow->WindowHandle, &Coordinates ) )
	{
		MouseX = 0.0f;
		MouseY = 0.0f;
        MouseDeltaX = 0.0f;
        MouseDeltaY = 0.0f;
	    Error::SetLastError( Error_GenericWindowFailure );
		return false;
	}

    float NormalizedX = ( float )Coordinates.x;
	float NormalizedY = ( float )Coordinates.y;

    // Normalize coordinates.
    NormalizedX /= State.ActiveWindow->Width * State.ActiveWindow->PixelWidth;
    NormalizedY /= State.ActiveWindow->Height * State.ActiveWindow->PixelHeight;

    // Set delta.
    MouseDeltaX = NormalizedX - MouseX;
    MouseDeltaY = NormalizedY - MouseY;

    // Set new position.
    MouseX = NormalizedX;
    MouseY = NormalizedY;

	return true;
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

	auto MakeProcInfo = []( const bool a_IsMemoryLoaded, const void* a_Handle, const ShaderProcRunFn a_Run, const ShaderInfo* a_Info )
	{
		ShaderProcInfo* ProcInfo = new ShaderProcInfo;
		ProcInfo->Handle = a_Handle;
		ProcInfo->Run = a_Run;
		ProcInfo->Info = a_Info;

		const auto MemoryDestructor = +[]( const ShaderProcInfo* a_ProcInfo )
		{
			MemoryFreeLibrary( ( HMEMORYMODULE )a_ProcInfo->Handle );
		};

		const auto Destructor = +[]( const ShaderProcInfo* a_ProcInfo )
		{
			FreeLibrary( ( HMODULE )a_ProcInfo->Handle );
		};

		return std::shared_ptr< ShaderProcInfo >( ProcInfo, a_IsMemoryLoaded ? MemoryDestructor : Destructor );
	};

	Shader Temp = *a_Shader;
		
	switch ( Temp.SourceType )
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

			ShaderFile.write( Temp.Source.c_str(), Temp.Source.size() );
		}

		Temp.File = SourceFile;
#else
		return false;
#endif
	}
	case EShaderSourceType::FileSource:
	{
#if IS_CONSOLEGL
		const std::string SourceFile = Temp.File.c_str();
		const std::string OutputFile = std::filesystem::path{ SourceFile }.replace_extension( "cglshader" ).string();

		// Attempt to compile the source file. If it failed, error will be set internally.
		if ( !RunCompiler( SourceFile.c_str(), OutputFile.c_str() ) )
		{
			return false;
		}

		// If this case is a fall through of the above case, then this means the source file is a temp file and
		// should be deleted after being compiled.
		if ( Temp.SourceType == EShaderSourceType::MemorySource && !std::filesystem::remove( Temp.File ) )
		{
			Error::SetLastError( Error_ShaderSourceFileCleanupFailure );
			return false;
		}

		// Now that the source file has been compiled, we can upgrade this source.
		Temp.File = OutputFile;
#else
		return false;
#endif
	}
	case EShaderSourceType::FileCompiled:
	{
		// If the type is FileCompiled, then we want to just load the library directly from the file, because this was a user-provided file.
		if ( Temp.SourceType == EShaderSourceType::FileCompiled )
		{
			const HMODULE ShaderHandle = LoadLibraryA( Temp.File.c_str() );

			if ( !ShaderHandle )
			{
				Error::SetLastError( Error_ShaderBinaryLoadFailure );
				return false;
			}

			const void* ShaderRun = GetProcAddress( ShaderHandle, "run" );
			
			if ( !ShaderRun )
			{
				Error::SetLastError( Error_ShaderRunEntryNotFound );
				return false;
			}

			const void* ShaderInfo = GetProcAddress( ShaderHandle, "info" );

			if ( !ShaderInfo )
			{
				Error::SetLastError( Error_ShaderInfoEntryNotFound );
				return false;
			}

			// Setup proc info.
			Temp.Proc = MakeProcInfo( false, ShaderHandle, ( ShaderProcRunFn )ShaderRun, ( ( ShaderProcInfoFn )ShaderInfo )() );
			break;
		}

		// If the type if FileSource or MemorySource, this means we've made a temporarily compiled file to load binary from, and delete after.
		{
			std::ifstream ShaderFile( Temp.File, std::ios::binary | std::ios::in );

			if ( !ShaderFile.is_open() )
			{
				Error::SetLastError( Error_ShaderBinaryFileLoadFailure );
				return false;
			}

			const size_t ShaderFileSize = std::filesystem::file_size( Temp.File );
			Temp.Binary.resize( ShaderFileSize );
			
			ShaderFile.read( ( char* )Temp.Binary.data(), ShaderFileSize );
			Temp.Buffer = Temp.Binary.data();
			Temp.BufferSize = ShaderFileSize;
		}

		// We have loaded the binary, now we can delete the compiled temp file.
		if ( !std::filesystem::remove( Temp.File ) )
		{
			Error::SetLastError( Error_ShaderBinaryFileCleanupFailure );
			return false;
		}

		Temp.File = "";
	}
	case EShaderSourceType::MemoryCompiled:
	{
		const HMEMORYMODULE ShaderHandle = MemoryLoadLibrary( Temp.Buffer, Temp.BufferSize );
		
		if ( !ShaderHandle )
		{
			Error::SetLastError( Error_ShaderBinaryLoadFailure );
			return false;
		}

		const void* ShaderRun = MemoryGetProcAddress( ShaderHandle, "run" );

		if ( !ShaderRun )
		{
			Error::SetLastError( Error_ShaderRunEntryNotFound );
			return false;
		}
		
		const void* ShaderInfo = MemoryGetProcAddress( ShaderHandle, "info" );

		if ( !ShaderInfo )
		{
			Error::SetLastError( Error_ShaderInfoEntryNotFound );
			return false;
		}

		Temp.Proc = MakeProcInfo( true, ShaderHandle, ( ShaderProcRunFn )ShaderRun, ( ( ShaderProcInfoFn )ShaderInfo )() );
		break;
	}
	default: break;
	}


	bool Error = false;

	// For each field, we want to fill out the shader variable info.
	const size_t FieldCount = Temp.Proc->Info->VariableCount;
	uint32_t ParamIndex = 0u;

	for ( uint32_t i = 0u; i < FieldCount; ++i )
	{
		if ( ShaderVariable Var; !ConvertShaderVar( Temp.Proc->Info->Variables[ i ], Var ) )
		{
			// Error
			Error = true;
		}
		else
		{
			switch ( Var.StorageQual )
			{
			case EShaderStorageQual::Uniform:
				Temp.Uniforms[ Var.Location ] = std::move( Var );
				break;
			case EShaderStorageQual::Attribute:
				Temp.Attributes[ Var.Location ] = std::move( Var );
				break;
			case EShaderStorageQual::In:
			case EShaderStorageQual::Out:
				Temp.Parameters[ ParamIndex++ ] = std::move( Var );
				break;
			}
		}
	}

	if ( Error )
	{
		Error::SetLastError( Error_ShaderCompileError );
		return false;
	}

	*a_Shader = std::move( Temp );
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

	ClearShaderProgramInfoLog();
	bool LinkFailure = false;

	// We need to verify the state of all shader entries.
	// For entries that are mandatory, there needs to be one,
	// and for all entries that are there, they need to be compiled.
	for ( size_t i = 0; i < SHADER_STAGE_COUNT; ++i )
	{
		const auto& Entry = a_ShaderProgram->Entries[ i ];

		switch ( ( EShaderType )i )
		{
		case EShaderType::Vertex:
		{
			if ( !Entry.Attached ) LinkFailure = true, WriteToShaderProgramInfoLog( "No vertex shader attached.\n" );
			else if ( !IsShaderCompiled( Entry.Attached ) ) LinkFailure = true, WriteToShaderProgramInfoLog( "Vertex shader is not compiled.\n" );
			break;
		}
		case EShaderType::Fragment:
		{
			if ( !Entry.Attached ) LinkFailure = true, WriteToShaderProgramInfoLog( "No fragment shader attached.\n" );
			else if ( !IsShaderCompiled( Entry.Attached ) ) LinkFailure = true, WriteToShaderProgramInfoLog( "Fragment shader is not compiled.\n" );
			break;
		}
		}
	}

	if ( LinkFailure )
	{
		Error::SetLastError( Error_ShaderProgramLinkFailure );
		return false;
	}

	// Let's now verify all Shader fields and make sure it all lines up.
	// 1. Uniforms with the same name on all shader stages should be the same location and type.
	// 2. Attributes with the same name on all shader stages must have the same location and type.
	// 3. Out parameters on Vertex need to be matched with In on Fragment and be same type.

	const ShaderHandle VertexShader = a_ShaderProgram->Entries[ ( size_t )EShaderType::Vertex ].Attached;
	const ShaderHandle FragmentShader = a_ShaderProgram->Entries[ ( size_t )EShaderType::Fragment ].Attached;

#pragma region VariableChecks



	
#pragma endregion

	// If there were any link failures after the above step, we should fail here.
	if ( LinkFailure )
	{
		Error::SetLastError( Error_ShaderProgramLinkFailure );
		return false;
	}

	// We need to setup shader program uniforms, attributes and parameters.


	// We can now copy across proc infos.
	for ( auto& Entry : a_ShaderProgram->Entries )
	{
		Entry.Proc = Entry.Attached->Proc;
	}

	// Linking a program that's currently in use is allowed.
	// So since this linking was successful, we want to set the active
	// entry points to these new entries.
	if ( State.ActiveContext && State.ActiveContext->ActiveShaderProgram == a_ShaderProgram )
	{
		for ( size_t i = 0u; i < SHADER_STAGE_COUNT; ++i )
		{
			State.ActiveContext->ShaderEntries[ i ] = a_ShaderProgram->Entries[ i ].Proc->Run;
		}
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
		Error::SetLastError( Error_NoShaderOfType );
		return false;
	}

	// Remove this program from the shaders list of attached to programs.
	if ( const auto Iter = std::ranges::find( Entry.Attached->AttachedTo.begin(), Entry.Attached->AttachedTo.end(), a_ShaderProgram ); Iter != Entry.Attached->AttachedTo.end() )
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

bool ConsoleGL::UseProgram( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	// If this is a nullptr, then it means we want to clear out the current program.
	if ( !a_ShaderProgram )
	{
		State.ActiveContext->ActiveShaderProgram = nullptr;

		for ( auto& Entry : State.ActiveContext->ShaderEntries )
		{
			Entry = nullptr;
		}

		return true;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return false;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return false;
	}

	State.ActiveContext->ActiveShaderProgram = a_ShaderProgram;

	for ( size_t i = 0u; i < SHADER_STAGE_COUNT; ++i )
	{
		State.ActiveContext->ShaderEntries[ i ] = a_ShaderProgram->Entries[ i ].Proc->Run;
	}

	return true;
}

#pragma endregion
























void ConsoleGL::RunTest()
{
	int Width = 160, Height = 90;

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

uniform(0) mat4 P;
uniform(1) mat4 V;
uniform(3) mat4 M;

attrib(0) vec3 Position;
attrib(1) vec3 Normal;
attrib(2) vec3 Colour;

out vec4 VertPos;
flat out vec3 FaceColour;
affn out vec2 UV;

void run()
{
	VertPos = P * V * M * vec4(Position, 1.0f);
}
)";

	ConsoleGL::SetShaderSourceFromString( DefaultVertexShader, VertexShaderSource.c_str(), VertexShaderSource.size() );
	ConsoleGL::CompileShader( DefaultVertexShader );
	ConsoleGL::AttachShader( DefaultShaderProgram, DefaultVertexShader );

	// Create fragment shader.
	ConsoleGL::Shader* DefaultFragmentShader = ConsoleGL::CreateShader( ConsoleGL::EShaderType::Fragment );
	const std::string FragmentShaderSource = R"(

in vec4 FaceColour;
in vec2 UV;
in vec4 FragColour;
in vec4 FragDepth;

void run()
{
	FragColour = FaceColour;
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

	// Use the program now for current context.
	ConsoleGL::UseProgram( DefaultShaderProgram );
}