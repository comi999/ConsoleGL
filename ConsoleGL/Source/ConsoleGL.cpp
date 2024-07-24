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
#include <detail/func_matrix.hpp>
#include <gtc/type_ptr.hpp>

// Disable dll exported functions.
#define _USER32_
#include <stdbool.h>
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
// VAO can be created, bound, and destroyed, references stay alive.
// VBO can be created, bound, and destroyed, references stay alive.

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
	Type* operator[]( const size_t a_Index ) { return a_Index <= C##apacity && Instances[ a_Index ].has_value() ? &Instances[ a_Index ].value() : nullptr; } \
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

	enum class EShaderStorageQual
	{
		In,
		Out,
		Uniform,
		Attribute
	};

	enum class EShaderInbuiltVar
	{
		// The clip-space output position of the current vertex
		out_vec4_VertPos,

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
		const char* Declaration = nullptr;
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
		EShaderInbuiltVar	InbuiltVar;
		EShaderInterpQual	InterpQual;
		EShaderStorageQual	StorageQual;
		EDataType			DataType;
		std::string			DataTypeName;
		std::string			Name;
		void*				Data;
		uint32_t			Location;
		uint32_t			Size;
		uint32_t			Length;
	};

	using ShaderUniformArray	= std::array< std::optional< ShaderVariable	>, MAX_SHADER_UNIFORMS	 >;
	using ShaderAttributeArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_ATTRIBUTES >;
	using ShaderParameterArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_PARAMETERS >;
	using InbuiltVarArray		= std::array< void*, ( size_t )EShaderInbuiltVar::None >;

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
		InbuiltVarArray						InbuiltVars;
	};

	struct ShaderProgramParameter
	{
		EShaderInterpQual	InterpQual;
		EDataType			DataType;
		std::string			DataTypeName;
		std::string			Name;
		void*				DataVertex;
		void*				DataFragment;
		uint32_t			Size;
		uint32_t			Length;
	};

	using ShaderProgramUniformArray		= std::array< std::optional< ShaderVariable	>, MAX_SHADER_UNIFORMS	 >;
	using ShaderProgramAttributeArray	= std::array< std::optional< ShaderVariable >, MAX_SHADER_ATTRIBUTES >;
	using ShaderProgramParameterArray	= std::array< std::optional< ShaderProgramParameter >, MAX_SHADER_PARAMETERS >;

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
		InbuiltVarArray				InbuiltVars;
	};

	struct Buffer
	{
		std::vector< uint8_t > Data;
	};

	void DestructorBuffer( BufferHandle a_Buffer ) {}

	DEFINE_REPOSITORY( Buffer, SHADER_BUFFER_MAX_COUNT );

	struct VertexAttrib
	{
		EDataType		DataType = EDataType::Float;
		uint32_t		Offset = 0u;
		uint32_t		Size = 0u;
		uint32_t		Stride = 0u;
		bool			Normalize = false;
		bool			Enabled = false;
		BufferHandle	Buffer = nullptr;
	};

	struct VertexArray
	{
		VertexAttrib Attribs[ MAX_SHADER_ATTRIBUTES ];
	};

	void DestructorVertexArray( VertexArrayHandle a_VertexArray ) {}

	DEFINE_REPOSITORY( VertexArray, SHADER_VERTEX_ARRAY_MAX_COUNT );

	struct Context
	{
		ShaderProgramHandle		ActiveShaderProgram;
		ShaderProcRunFn			ShaderEntries[ SHADER_STAGE_COUNT ];
		BufferRepository		Buffers;
		BufferHandle			BufferTargets[ ( size_t )EBufferTarget::MAX ];
		VertexArrayRepository	VertexArrays;
		VertexArray				DefaultVertexArray;
		VertexArrayHandle		ActiveVertexArray;
	};
}

bool ConvertShaderInbuilt( const ConsoleGL::ShaderVariable& a_ShaderVar, ConsoleGL::EShaderInbuiltVar& o_ShaderInbuilt )
{
	switch ( ( ConsoleGL::EShaderInbuiltVar )0 )
	{
	case ConsoleGL::EShaderInbuiltVar::out_vec4_VertPos:
	{
		if ( 
			a_ShaderVar.InterpQual == ConsoleGL::EShaderInterpQual::None &&
			a_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::Out &&
			a_ShaderVar.DataType == ConsoleGL::EDataType::Vec4 &&
			a_ShaderVar.Name == "VertPos" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::out_vec4_VertPos;
			return true;
		}

		if ( a_ShaderVar.Name == "VertPos" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::None;
			return false;
		}
	}
	case ConsoleGL::EShaderInbuiltVar::in_vec4_FragCoord:
	{
		if ( 
			a_ShaderVar.InterpQual == ConsoleGL::EShaderInterpQual::None &&
			a_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::In &&
			a_ShaderVar.DataType == ConsoleGL::EDataType::Vec4 &&
			a_ShaderVar.Name == "FragCoord" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::in_vec4_FragCoord;
			return true;
		}

		if ( a_ShaderVar.Name == "FragCoord" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::None;
			return false;
		}
	}
	case ConsoleGL::EShaderInbuiltVar::out_vec4_FragColour:
	{
		if (
			a_ShaderVar.InterpQual == ConsoleGL::EShaderInterpQual::None &&
			a_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::Out &&
			a_ShaderVar.DataType == ConsoleGL::EDataType::Vec4 &&
			a_ShaderVar.Name == "FragColour" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::out_vec4_FragColour;
			return true;
		}
		
		if ( a_ShaderVar.Name == "FragColour" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::None;
			return false;
		}
	}
	case ConsoleGL::EShaderInbuiltVar::out_float_FragDepth:
	{
		if (
			a_ShaderVar.InterpQual == ConsoleGL::EShaderInterpQual::None &&
			a_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::Out &&
			a_ShaderVar.DataType == ConsoleGL::EDataType::Float &&
			a_ShaderVar.Name == "FragDepth" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::out_float_FragDepth;
			return true;
		}
		
		if ( a_ShaderVar.Name == "FragDepth" )
		{
			o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::None;
			return false;
		}
	}
	default: break;
	}

	o_ShaderInbuilt = ConsoleGL::EShaderInbuiltVar::None;
	return true;
}

bool ConvertShaderVar( const ConsoleGL::ShaderVariableInfo& a_ShaderVarInfo, ConsoleGL::ShaderVariable& o_ShaderVar )
{
	const std::string InterpQual = a_ShaderVarInfo.InterpQual;

	if ( InterpQual == "" ) o_ShaderVar.InterpQual = ConsoleGL::EShaderInterpQual::None;
	else if ( InterpQual == "affn" ) o_ShaderVar.InterpQual = ConsoleGL::EShaderInterpQual::Affine;
	else if ( InterpQual == "flat" ) o_ShaderVar.InterpQual = ConsoleGL::EShaderInterpQual::Flat;
	else if ( InterpQual == "prsp" ) o_ShaderVar.InterpQual = ConsoleGL::EShaderInterpQual::Perspective;
	else return false;

	const std::string StorageQual = a_ShaderVarInfo.StorageQual;

	if ( StorageQual == "" ) return false;
	else if ( StorageQual == "out" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Out;
	else if ( StorageQual == "in" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::In;
	else if ( StorageQual == "attrib" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Attribute;
	else if ( StorageQual == "uniform" ) o_ShaderVar.StorageQual = ConsoleGL::EShaderStorageQual::Uniform;
	else return false;

	if ( a_ShaderVarInfo.Location == -1 )
	{
		o_ShaderVar.Location = -1;
	}
	else
	{
		o_ShaderVar.Location = ( uint32_t )a_ShaderVarInfo.Location;
	}

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

	const std::string DataType = a_ShaderVarInfo.Type;

	if ( DataType == "" ) return false;
	else if ( DataType == "int8"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Int8;
	else if ( DataType == "int16"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Int16;
	else if ( DataType == "int32"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Int32;
	else if ( DataType == "int64"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Int64;
	else if ( DataType == "uint8"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uint8;
	else if ( DataType == "uint16"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uint16;
	else if ( DataType == "uint32"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uint32;
	else if ( DataType == "uint64"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uint64;
	else if ( DataType == "float"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Float;
	else if ( DataType == "double"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Double;
	else if ( DataType == "bool"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Bool;
	else if ( DataType == "mat2"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat2;
	else if ( DataType == "mat3"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat3;
	else if ( DataType == "mat4"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat4;
	else if ( DataType == "mat2x3"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat2x3;
	else if ( DataType == "mat2x4"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat2x4;
	else if ( DataType == "mat3x2"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat3x2;
	else if ( DataType == "mat3x4"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat3x4;
	else if ( DataType == "mat4x2"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat4x2;
	else if ( DataType == "mat4x3"  ) o_ShaderVar.DataType = ConsoleGL::EDataType::Mat4x3;
	else if ( DataType == "dmat2"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat2;
	else if ( DataType == "dmat3"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat3;
	else if ( DataType == "dmat4"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat4;
	else if ( DataType == "dmat2x3" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat2x3;
	else if ( DataType == "dmat2x4" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat2x4;
	else if ( DataType == "dmat3x2" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat3x2;
	else if ( DataType == "dmat3x4" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat3x4;
	else if ( DataType == "dmat4x2" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat4x2;
	else if ( DataType == "dmat4x3" ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dmat4x3;
	else if ( DataType == "vec2"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Vec2;
	else if ( DataType == "vec3"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Vec3;
	else if ( DataType == "vec4"    ) o_ShaderVar.DataType = ConsoleGL::EDataType::Vec4;
	else if ( DataType == "dvec2"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dvec2;
	else if ( DataType == "dvec3"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dvec3;
	else if ( DataType == "dvec4"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Dvec4;
	else if ( DataType == "ivec2"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Ivec2;
	else if ( DataType == "ivec3"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Ivec3;
	else if ( DataType == "ivec4"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Ivec4;
	else if ( DataType == "uvec2"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uvec2;
	else if ( DataType == "uvec3"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uvec3;
	else if ( DataType == "uvec4"   ) o_ShaderVar.DataType = ConsoleGL::EDataType::Uvec4;
	else o_ShaderVar.DataType = ConsoleGL::EDataType::Custom;

	o_ShaderVar.DataTypeName = DataType;

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

	if ( !ConvertShaderInbuilt( o_ShaderVar, o_ShaderVar.InbuiltVar ) )
	{
		return false;
	}

	// If there is no interpolation qualifier and it's an out variable, then we use the default, perspective.
	if ( o_ShaderVar.InterpQual == ConsoleGL::EShaderInterpQual::None && o_ShaderVar.StorageQual == ConsoleGL::EShaderStorageQual::Out && o_ShaderVar.InbuiltVar == ConsoleGL::EShaderInbuiltVar::None )
	{
		o_ShaderVar.InterpQual = ConsoleGL::EShaderInterpQual::Perspective;
	}

	return true;
}

bool IsValidShaderInbuilt( const ConsoleGL::EShaderType a_ShaderType, const ConsoleGL::EShaderInbuiltVar a_InbuiltVar )
{
	switch ( a_ShaderType )
	{
	case ConsoleGL::EShaderType::Vertex:
	{
		switch ( a_InbuiltVar )
		{
		case ConsoleGL::EShaderInbuiltVar::out_vec4_VertPos:
			return true;
		default: return false;
		}
	}
	case ConsoleGL::EShaderType::Fragment:
	{
		switch ( a_InbuiltVar )
		{
		case ConsoleGL::EShaderInbuiltVar::in_vec4_FragCoord:
		case ConsoleGL::EShaderInbuiltVar::out_vec4_FragColour:
		case ConsoleGL::EShaderInbuiltVar::out_float_FragDepth:
			return true;
		default: return false;
		}
	}
	}

	return false;
}

bool GetVarTypeAsUnderlying( const ConsoleGL::EDataType a_VarType, ConsoleGL::EDataType& o_UnderlyingVarType, uint32_t& o_ArraySize )
{
	switch ( a_VarType )
	{
	case ConsoleGL::EDataType::Custom:
		return false;
	case ConsoleGL::EDataType::Int8:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Int16:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Int32:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Int64:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Uint8:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Uint16:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Uint32:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Uint64:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Float:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Double:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Bool:
		o_UnderlyingVarType = a_VarType;
		o_ArraySize = 1u;
		break;
	case ConsoleGL::EDataType::Mat2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 4u;
		break;
	case ConsoleGL::EDataType::Mat3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 9u;
		break;
	case ConsoleGL::EDataType::Mat4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 16u;
		break;
	case ConsoleGL::EDataType::Mat2x3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 6u;
		break;
	case ConsoleGL::EDataType::Mat2x4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 8u;
		break;
	case ConsoleGL::EDataType::Mat3x2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 6u;
		break;
	case ConsoleGL::EDataType::Mat3x4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 12u;
		break;
	case ConsoleGL::EDataType::Mat4x2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 8u;
		break;
	case ConsoleGL::EDataType::Mat4x3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 12u;
		break;
	case ConsoleGL::EDataType::Dmat2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 4u;
		break;
	case ConsoleGL::EDataType::Dmat3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 9u;
		break;
	case ConsoleGL::EDataType::Dmat4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 16u;
		break;
	case ConsoleGL::EDataType::Dmat2x3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 6u;
		break;
	case ConsoleGL::EDataType::Dmat2x4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 8u;
		break;
	case ConsoleGL::EDataType::Dmat3x2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 6u;
		break;
	case ConsoleGL::EDataType::Dmat3x4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 12u;
		break;
	case ConsoleGL::EDataType::Dmat4x2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 8u;
		break;
	case ConsoleGL::EDataType::Dmat4x3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 12u;
		break;
	case ConsoleGL::EDataType::Vec2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 2u;
		break;
	case ConsoleGL::EDataType::Vec3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 3u;
		break;
	case ConsoleGL::EDataType::Vec4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Float;
		o_ArraySize = 4u;
		break;
	case ConsoleGL::EDataType::Dvec2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 2u;
		break;
	case ConsoleGL::EDataType::Dvec3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 3u;
		break;
	case ConsoleGL::EDataType::Dvec4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Double;
		o_ArraySize = 4u;
		break;
	case ConsoleGL::EDataType::Ivec2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Int32;
		o_ArraySize = 2u;
		break;
	case ConsoleGL::EDataType::Ivec3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Int32;
		o_ArraySize = 3u;
		break;
	case ConsoleGL::EDataType::Ivec4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Int32;
		o_ArraySize = 4u;
		break;
	case ConsoleGL::EDataType::Uvec2:
		o_UnderlyingVarType = ConsoleGL::EDataType::Uint32;
		o_ArraySize = 2u;
		break;
	case ConsoleGL::EDataType::Uvec3:
		o_UnderlyingVarType = ConsoleGL::EDataType::Uint32;
		o_ArraySize = 3u;
		break;
	case ConsoleGL::EDataType::Uvec4:
		o_UnderlyingVarType = ConsoleGL::EDataType::Uint32;
		o_ArraySize = 4u;
		break;
	default: break;
	};

	return true;
}

const char* GetShaderTypeName( const ConsoleGL::EShaderType a_ShaderType )
{
	switch ( a_ShaderType )
	{
	case ConsoleGL::EShaderType::Vertex:
		return "Vertex";
	case ConsoleGL::EShaderType::Fragment:
		return "Fragment";
	default: break;
	}

	return "";
}

const char* GetInterpQualName( const ConsoleGL::EShaderInterpQual a_InterpQual )
{
	switch ( a_InterpQual )
	{
	case ConsoleGL::EShaderInterpQual::Flat:
		return "flat";
	case ConsoleGL::EShaderInterpQual::Affine:
		return "affn";
	case ConsoleGL::EShaderInterpQual::Perspective:
		return "prsp";
	default: break;
	}

	return "";
}

const char* GetStorageQualName( const ConsoleGL::EShaderStorageQual a_StorageQual )
{
	switch ( a_StorageQual )
	{
	case ConsoleGL::EShaderStorageQual::Uniform:
		return "uniform";
	case ConsoleGL::EShaderStorageQual::Attribute:
		return "attrib";
	case ConsoleGL::EShaderStorageQual::In:
		return "in";
	case ConsoleGL::EShaderStorageQual::Out:
		return "out";
	default: break;
	}

	return "";
}

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
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return "";
	}

	return a_Window->Title.c_str();
}

uint32_t ConsoleGL::GetWindowWidth( const WindowHandle a_Window )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Width;
}

uint32_t ConsoleGL::GetWindowHeight( const WindowHandle a_Window )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Height;
}

uint32_t ConsoleGL::GetWindowBufferIndex( const WindowHandle a_Window )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->ActiveBuffer;
}

uint32_t ConsoleGL::GetWindowBufferCount( const WindowHandle a_Window )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return 0;
	}

	return a_Window->Buffers.size();
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBuffer( const WindowHandle a_Window )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
	{
		Error::SetLastError( Error_InvalidWindowHandle );
		return nullptr;
	}

	return &a_Window->Buffers[ a_Window->ActiveBuffer ];
}

ConsoleGL::PixelBuffer* ConsoleGL::GetWindowBufferByIndex( const WindowHandle a_Window, const uint32_t a_Index )
{
	if ( !a_Window || !State.Windows.IsValid( a_Window ) )
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


	bool CompileFailure = false;

	ClearShaderInfoLog();

	// For each field, we want to fill out the shader variable info.
	const size_t FieldCount = Temp.Proc->Info->VariableCount;
	uint32_t ParamIndex = 0u;

	for ( uint32_t i = 0u; i < FieldCount; ++i )
	{
		ShaderVariable Var{};
		EShaderInbuiltVar InbuiltVar{};
		EDataType DataType{};
		uint32_t ArraySize{};

		// If this is a malformed shader var to begin with, fail here.
		if ( !ConvertShaderVar( Temp.Proc->Info->Variables[ i ], Var ) )
		{
			WriteToShaderInfoLog( "Declaration '%s' is malformed.\n", Temp.Proc->Info->Variables[ i ].Declaration );
			CompileFailure = true;
		}

		// If this is an inbuilt, check that it's valid for the shader.
		else if ( Var.InbuiltVar != EShaderInbuiltVar::None && !IsValidShaderInbuilt( Temp.Type, Var.InbuiltVar ) )
		{
			WriteToShaderInfoLog( "Inbuilt shader variable '%s' not available in %s shader.\n", Temp.Proc->Info->Variables[ i ].Declaration, GetShaderTypeName( Temp.Type ) );
			CompileFailure = true;
		}

		// If the storage qualifier is attribute, then this is only allowed in a per-vertex shader stage.
		else if ( Var.StorageQual == EShaderStorageQual::Attribute && a_Shader->Type != EShaderType::Vertex )
		{
			WriteToShaderInfoLog( "'attrib' variable '%s' is only allowed in Vertex shader.\n", Temp.Proc->Info->Variables[ i ].Declaration );
			CompileFailure = true;
		}

		// If this is an in variable in a vertex shader, this is not allowed.
		else if ( Var.StorageQual == EShaderStorageQual::In && a_Shader->Type == EShaderType::Vertex )
		{
			WriteToShaderInfoLog( "'in' variable '%s' is only allowed in Fragment shader.\n", Temp.Proc->Info->Variables[ i ].Declaration );
			CompileFailure = true;
		}

		// If this is an out variable in a fragment shader, this is not allowed.
		else if ( Var.StorageQual == EShaderStorageQual::Out && a_Shader->Type == EShaderType::Fragment && Var.InbuiltVar == EShaderInbuiltVar::None )
		{
			WriteToShaderInfoLog( "'out' variable '%s' is only allowed in Fragment shader.", Temp.Proc->Info->Variables[ i ].Declaration );
			CompileFailure = true;
		}

		// In and out custom types are only allowed with flat interpolation.
		else if ( ( Var.StorageQual == EShaderStorageQual::In || Var.StorageQual == EShaderStorageQual::Out ) && Var.DataType == EDataType::Custom && Var.InterpQual != EShaderInterpQual::Flat )
		{
			WriteToShaderInfoLog( "%s variable '%s' with custom type '%s' can only be declared flat.\n", 
				GetStorageQualName( Var.StorageQual ), 
				Temp.Proc->Info->Variables[ i ].Declaration,
				Temp.Proc->Info->Variables[ i ].Type );
			CompileFailure = true;
		}

		// If this is a prsp or affn out, then the underlying type must be float.
		else if ( Var.StorageQual == EShaderStorageQual::Out && ( Var.InterpQual == EShaderInterpQual::Affine || Var.InterpQual == EShaderInterpQual::Perspective ) && ( !GetVarTypeAsUnderlying( Var.DataType, DataType, ArraySize ) || DataType != EDataType::Float ) )
		{
			WriteToShaderInfoLog( "In declaration '%s', non-float type used with '%s'. Only flat can be used with non-float types.\n",
				Temp.Proc->Info->Variables[ i ].Declaration,
				GetInterpQualName( Var.InterpQual ) );
			CompileFailure = true;
		}

		// Interpolation qualifiers can only be used with out.
		else if ( Var.InterpQual != EShaderInterpQual::None && Var.StorageQual != EShaderStorageQual::Out )
		{
			WriteToShaderInfoLog( "In declaration '%s', '%s' can only be used with out variables.\n",
				Temp.Proc->Info->Variables[ i ].Declaration,
				GetInterpQualName( Var.InterpQual ) );
			CompileFailure = true;
		}

		// If this is an inbuilt, then place it in the inbuilt var array.
		else if ( Var.InbuiltVar != EShaderInbuiltVar::None )
		{
			Temp.InbuiltVars[ ( size_t )Var.InbuiltVar ] = Var.Data;
		}
		
		// Place it in the right variable array.
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

	if ( CompileFailure )
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
			if ( !Entry.Attached )
			{
				LinkFailure = true;
				WriteToShaderProgramInfoLog( "No vertex shader attached.\n" );
			}
			else if ( !IsShaderCompiled( Entry.Attached ) ) 
			{
				LinkFailure = true;
				WriteToShaderProgramInfoLog( "Vertex shader is not compiled.\n" );
			}
			break;
		}
		case EShaderType::Fragment:
		{
			if ( !Entry.Attached ) 
			{
				LinkFailure = true;
				WriteToShaderProgramInfoLog( "No fragment shader attached.\n" );
			}
			else if ( !IsShaderCompiled( Entry.Attached ) ) 
			{
				LinkFailure = true;
				WriteToShaderProgramInfoLog( "Fragment shader is not compiled.\n" );
			}
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
	ShaderProgram Temp = *a_ShaderProgram;

	const ShaderHandle VertexShader = a_ShaderProgram->Entries[ ( size_t )EShaderType::Vertex ].Attached;
	const ShaderHandle FragmentShader = a_ShaderProgram->Entries[ ( size_t )EShaderType::Fragment ].Attached;

#pragma region VariableChecks
	// Need to check that all uniforms in specific slots are
	// the same name and type.

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( VertexShader->Uniforms[ i ].has_value() && FragmentShader->Uniforms[ i ].has_value() )
		{
			if ( VertexShader->Uniforms[ i ].value().Name != FragmentShader->Uniforms[ i ].value().Name )
			{
				WriteToShaderProgramInfoLog( "Uniform at location %u declared with different names '%s' and '%s'.\n", i, 
					VertexShader->Uniforms[ i ].value().Name.c_str(), 
					FragmentShader->Uniforms[ i ].value().Name.c_str() );
				LinkFailure = true;
			}

			if ( VertexShader->Uniforms[ i ].value().DataType != FragmentShader->Uniforms[ i ].value().DataType )
			{
				WriteToShaderProgramInfoLog( "Uniform at location %u declared with different types.\n", i );
				LinkFailure = true;
			}
		}

		if ( VertexShader->Uniforms[ i ].has_value() )
		{
			Temp.Uniforms[ i ] = VertexShader->Uniforms[ i ];
		}
		
		if ( FragmentShader->Uniforms[ i ].has_value() )
		{
			Temp.Uniforms[ i ] = FragmentShader->Uniforms[ i ];
		}
	}

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( VertexShader->Attributes[ i ].has_value() )
		{
			Temp.Attributes[ i ] = VertexShader->Attributes[ i ].value();
		}
	}

	// We need to do multiple checks with parameters.
	// Here we can now try and link up in/out parameters by name and type.
	// If there is an out parameter in a vertex shader with the same name as an in parameter
	// in a fragment shader, they must have the same type.
	// We should also place them in the same slot location.

	std::map< std::string, std::pair< const ShaderVariable*, const ShaderVariable* > > Parameters;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( VertexShader->Parameters[ i ].has_value() )
		{
			Parameters[ VertexShader->Parameters[ i ].value().Name ].first = &VertexShader->Parameters[ i ].value();
		}

		if ( FragmentShader->Parameters[ i ].has_value() )
		{
			Parameters[ FragmentShader->Parameters[ i ].value().Name ].second = &FragmentShader->Parameters[ i ].value();
		}
	}

	uint32_t CurrentIndex = 0u;
	
	for ( auto Begin = Parameters.begin(), End = Parameters.end(); Begin != End; ++Begin )
	{
		if ( Begin->second.first && Begin->second.second )
		{
			// First check that type and size are the same.
			if ( Begin->second.first->DataType != Begin->second.second->DataType )
			{
				WriteToShaderProgramInfoLog( "Parameter '%s' declared with different type in Fragment shader.\n", Begin->first.c_str() );
				LinkFailure = true;
				continue;
			}

			// First check that type and size are the same.
			if ( Begin->second.first->Size != Begin->second.second->Size )
			{
				WriteToShaderProgramInfoLog( "Parameter '%s' has different size in Fragment shader.\n", Begin->first.c_str() );
				LinkFailure = true;
				continue;
			}

			ShaderProgramParameter& Parameter = Temp.Parameters[ CurrentIndex++ ].emplace();
			Parameter.InterpQual = Begin->second.first->InterpQual;
			Parameter.DataType = Begin->second.first->DataType;
			Parameter.DataTypeName = Begin->second.first->DataTypeName;
			Parameter.Name = Begin->first;
			Parameter.DataVertex = Begin->second.first->Data;
			Parameter.DataFragment = Begin->second.second->Data;
			Parameter.Size = Begin->second.first->Size;
			Parameter.Length = Begin->second.first->Length;
		}

		// If only singly linked in vertex shader, do nothing.
		else if ( Begin->second.first && !Begin->second.second )
		{
			// Do nothing.
		}

		else if ( !Begin->second.first && Begin->second.second )
		{
			WriteToShaderProgramInfoLog( "'%s' declared in Fragment shader with no input from Vertex shader.\n", Begin->second.second->Name.c_str() );
			LinkFailure = true;
		}
	}

	for ( uint32_t i = 0u; i < ( uint32_t )EShaderInbuiltVar::None; ++i )
	{
		if ( VertexShader->InbuiltVars[ i ] )
		{
			Temp.InbuiltVars[ i ] = VertexShader->InbuiltVars[ i ];
		}
		else if ( FragmentShader->InbuiltVars[ i ] )
		{
			Temp.InbuiltVars[ i ] = FragmentShader->InbuiltVars[ i ];
		}
	}

#pragma endregion

	// We can now copy across proc infos.
	for ( auto& Entry : Temp.Entries )
	{
		Entry.Proc = Entry.Attached->Proc;
	}

	// If there were any link failures after the above step, we should fail here.
	if ( LinkFailure )
	{
		Error::SetLastError( Error_ShaderProgramLinkFailure );
		return false;
	}

	*a_ShaderProgram = std::move( Temp );

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

//...

const char* ConsoleGL::GetShaderProgramInfoLog()
{
	return ShaderProgramInfoLog;
}

size_t ConsoleGL::GetShaderProgramInfoLogLength()
{
	return ShaderProgramInfoLogLength;
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

//...

bool ConsoleGL::CreateBuffers( const uint32_t a_Count, BufferHandle* o_Buffers )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( !o_Buffers )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( State.ActiveContext->Buffers.Free() < a_Count )
	{
		Error::SetLastError( Error_BufferCapacityReached );
		return false;
	}

	for ( uint32_t i = 0u; i < a_Count; ++i )
	{
		o_Buffers[ i ] = State.ActiveContext->Buffers.Create();
	}

	return true;
}

bool ConsoleGL::BindBuffer( const EBufferTarget a_BufferTarget, const BufferHandle a_Buffer )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( a_Buffer && !State.ActiveContext->Buffers.IsValid( a_Buffer ) )
	{
		Error::SetLastError( Error_InvalidBufferHandle );
		return false;
	}

	// Check to see if this buffer is already bound to another target (if it's not a nullptr).
	for ( uint32_t i = 0u; i < ( size_t )EBufferTarget::MAX && a_Buffer; ++i )
	{
		if ( State.ActiveContext->BufferTargets[ i ] == a_Buffer && i != ( size_t )a_BufferTarget )
		{
			Error::SetLastError( Error_BufferAlreadyBoundToTarget );
			return false;
		}
	}

	State.ActiveContext->BufferTargets[ ( size_t )a_BufferTarget ] = a_Buffer;
	return true;
}

bool ConsoleGL::DeleteBuffers( const uint32_t a_Count, const BufferHandle* a_Buffers )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( !a_Buffers )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	for ( uint32_t i = 0u; i < a_Count; ++i )
	{
		const BufferHandle Buffer = a_Buffers[ i ];

		if ( !State.ActiveContext->Buffers.Destroy( Buffer ) )
		{
			return false;
		}

		// If this buffer was bound to any buffer targets, destroy the binding.
		for ( uint32_t j = 0u; j < ( size_t )EBufferTarget::MAX; ++j )
		{
			if ( State.ActiveContext->BufferTargets[ j ] == Buffer )
			{
				State.ActiveContext->BufferTargets[ j ] = nullptr;
			}
		}

		// If this buffer was bound to any vertex attribs, destroy the binding.
		for ( uint32_t j = 0u; j < SHADER_VERTEX_ARRAY_MAX_COUNT; ++j )
		{
			if ( const VertexArrayHandle VertexArray = State.ActiveContext->VertexArrays[ i ] )
			{
				for ( uint32_t k = 0u; k < MAX_SHADER_ATTRIBUTES; ++k )
				{
					if ( VertexArray->Attribs[ k ].Buffer == Buffer )
					{
						VertexArray->Attribs[ k ].Buffer = nullptr;
					}
				}
			}
		}
	}

	return true;
}

//...

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

bool ConsoleGL::BufferData( const EBufferTarget a_BufferTarget, const size_t a_Size, const void* a_Data/*, DataUsage a_DataUsage */)
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	const BufferHandle Buffer = State.ActiveContext->BufferTargets[ ( size_t )a_BufferTarget ];

	if ( !Buffer )
	{
		Error::SetLastError( Error_BufferNotBoundToTarget );
		return false;
	}

	if ( a_Size == 0u || !a_Data )
	{
		Buffer->Data.clear();
		return false;
	}

	Buffer->Data.resize( a_Size );
	( void )memcpy( Buffer->Data.data(), a_Data, a_Size );
	return true;
}

//...

bool ConsoleGL::CreateVertexArrays( const uint32_t a_Count, VertexArrayHandle* o_VertexArrays )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( !o_VertexArrays )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	if ( State.ActiveContext->VertexArrays.Free() < a_Count )
	{
		Error::SetLastError( Error_BufferCapacityReached );
		return false;
	}

	for ( uint32_t i = 0u; i < a_Count; ++i )
	{
		o_VertexArrays[ i ] = State.ActiveContext->VertexArrays.Create();
	}

	return true;
}

bool ConsoleGL::BindVertexArray( const VertexArrayHandle a_VertexArray )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	State.ActiveContext->ActiveVertexArray = a_VertexArray;
	return true;
}

bool ConsoleGL::DeleteVertexArrays( const uint32_t a_Count, const VertexArrayHandle* a_VertexArrays )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( !a_VertexArrays )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	for ( uint32_t i = 0u; i < a_Count; ++i )
	{
		const VertexArrayHandle VertexArray = a_VertexArrays[ i ];

		if ( !State.ActiveContext->VertexArrays.Destroy( VertexArray ) )
		{
			return false;
		}

		// If this vertex array was bound, destroy the binding.
		if ( State.ActiveContext->ActiveVertexArray == VertexArray )
		{
			State.ActiveContext->ActiveVertexArray = nullptr;
		}
	}

	return true;
}

bool ConsoleGL::EnableVertexAttribArray( const uint32_t a_Location )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( a_Location >= SHADER_VERTEX_ARRAY_MAX_COUNT )
	{
		Error::SetLastError( Error_InvalidAttrributeLocation );
		return false;
	}

	const VertexArrayHandle VertexArray = State.ActiveContext->ActiveVertexArray ? State.ActiveContext->ActiveVertexArray : &State.ActiveContext->DefaultVertexArray;
	VertexArray->Attribs[ a_Location ].Enabled = true;
	return true;
}

bool ConsoleGL::DisableVertexAttribArray( const uint32_t a_Location )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( a_Location >= SHADER_VERTEX_ARRAY_MAX_COUNT )
	{
		Error::SetLastError( Error_InvalidAttrributeLocation );
		return false;
	}

	const VertexArrayHandle VertexArray = State.ActiveContext->ActiveVertexArray ? State.ActiveContext->ActiveVertexArray : &State.ActiveContext->DefaultVertexArray;
	VertexArray->Attribs[ a_Location ].Enabled = false;
	return true;
}

bool ConsoleGL::VertexAttribPointer( const uint32_t a_Location, const uint32_t a_Size, const EDataType a_DataType, const bool a_Normalize, const uint32_t a_Stride, const uint32_t a_Offset )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	if ( a_Location >= SHADER_VERTEX_ARRAY_MAX_COUNT )
	{
		Error::SetLastError( Error_InvalidAttrributeLocation );
		return false;
	}

	if ( a_DataType == EDataType::Custom )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
	}

	const VertexArrayHandle VertexArray = State.ActiveContext->ActiveVertexArray ? State.ActiveContext->ActiveVertexArray : &State.ActiveContext->DefaultVertexArray;
	VertexAttrib& AttribPointer = VertexArray->Attribs[ a_Location ];
	AttribPointer.DataType = a_DataType;
	AttribPointer.Offset = a_Offset;
	AttribPointer.Size = a_Size;
	AttribPointer.Stride = a_Stride;
	AttribPointer.Normalize = a_Normalize;
	AttribPointer.Buffer = State.ActiveContext->BufferTargets[ ( size_t )EBufferTarget::ArrayBuffer ];
	return true;
}

#pragma region TESTING_DELETE_LATER

using AttribWalkerFn = void( * )( const void*&, void*, const uint32_t );

AttribWalkerFn GetAttribWalkerFn( const ConsoleGL::EDataType a_DataType )
{
#define FUNC_RETURN( Type ) \
	return +[]( const void*& a_Input, void* a_Output, const uint32_t a_Stride ) { \
		*static_cast< Type* >( a_Output ) = *static_cast< const Type* >( a_Input ); \
		a_Input = static_cast< const uint8_t* >( a_Input ) + a_Stride; \
		};

	switch ( a_DataType )
	{
	case ConsoleGL::EDataType::Custom:
		return nullptr;
	case ConsoleGL::EDataType::Int8:
	case ConsoleGL::EDataType::Int16:
		return nullptr;
	case ConsoleGL::EDataType::Int32:
	case ConsoleGL::EDataType::Int64:
	case ConsoleGL::EDataType::Uint8:
	case ConsoleGL::EDataType::Uint16:
	case ConsoleGL::EDataType::Uint32:
	case ConsoleGL::EDataType::Uint64:
	case ConsoleGL::EDataType::Float:
	case ConsoleGL::EDataType::Double:
	case ConsoleGL::EDataType::Bool:
	case ConsoleGL::EDataType::Mat2:
	case ConsoleGL::EDataType::Mat3:
	case ConsoleGL::EDataType::Mat4:
	case ConsoleGL::EDataType::Mat2x3:
	case ConsoleGL::EDataType::Mat2x4:
	case ConsoleGL::EDataType::Mat3x2:
	case ConsoleGL::EDataType::Mat3x4:
	case ConsoleGL::EDataType::Mat4x2:
	case ConsoleGL::EDataType::Mat4x3:
	case ConsoleGL::EDataType::Dmat2:
	case ConsoleGL::EDataType::Dmat3:
	case ConsoleGL::EDataType::Dmat4:
	case ConsoleGL::EDataType::Dmat2x3:
	case ConsoleGL::EDataType::Dmat2x4:
	case ConsoleGL::EDataType::Dmat3x2:
	case ConsoleGL::EDataType::Dmat3x4:
	case ConsoleGL::EDataType::Dmat4x2:
	case ConsoleGL::EDataType::Dmat4x3:
	case ConsoleGL::EDataType::Vec2:
		return nullptr;
	case ConsoleGL::EDataType::Vec3:
		FUNC_RETURN( glm::vec3 );
	case ConsoleGL::EDataType::Vec4:
	case ConsoleGL::EDataType::Dvec2:
	case ConsoleGL::EDataType::Dvec3:
	case ConsoleGL::EDataType::Dvec4:
	case ConsoleGL::EDataType::Ivec2:
	case ConsoleGL::EDataType::Ivec3:
	case ConsoleGL::EDataType::Ivec4:
	case ConsoleGL::EDataType::Uvec2:
	case ConsoleGL::EDataType::Uvec3:
	case ConsoleGL::EDataType::Uvec4:
		return nullptr;
	default: return nullptr;
	}
}

struct AttribWalker
{
	uint32_t Stride = 0u;
	const void* Input = nullptr;
	void* Output = nullptr;
	AttribWalkerFn Fn = nullptr;
};

struct ParamWalker
{
	uint32_t Size = 0u;
	uint32_t Stride = 0u;
	const void* Input = nullptr;
	void* Output = nullptr;
};

void IncrementParamWalker( ParamWalker* pw )
{
	memcpy( pw->Output, pw->Input, pw->Size );
	pw->Output = static_cast< uint8_t* >( pw->Output ) + pw->Stride;
}

using IndexWalkerFn = uint64_t(*)( const void*& );

IndexWalkerFn GetIndexWalkerFn( const ConsoleGL::EDataType a_DataType )
{
#define FN_RETURN( Type ) \
	return +[]( const void*& a_Data ) { uint64_t Index = *static_cast< const Type* >( a_Data ); a_Data = static_cast< const Type* >( a_Data ) + 1; return Index; };

	switch ( a_DataType )
	{
	case ConsoleGL::EDataType::Int8:
		FN_RETURN( int8_t );
	case ConsoleGL::EDataType::Int16:
		FN_RETURN( int16_t );
	case ConsoleGL::EDataType::Int32:
		FN_RETURN( int32_t );
	case ConsoleGL::EDataType::Int64:
		FN_RETURN( int64_t );
	case ConsoleGL::EDataType::Uint8:
		FN_RETURN( uint8_t );
	case ConsoleGL::EDataType::Uint16:
		FN_RETURN( uint16_t );
	case ConsoleGL::EDataType::Uint32:
		FN_RETURN( uint32_t );
	case ConsoleGL::EDataType::Uint64:
		FN_RETURN( uint64_t );
	default: return nullptr;
	}
	
#undef FN_RETURN
}

struct DrawContext
{
	ConsoleGL::ShaderProcRunFn Run;
	ConsoleGL::ShaderProcRunFn FragRun;
	const glm::vec4* VertPos;
	AttribWalker Walkers[ MAX_SHADER_ATTRIBUTES ];
	uint32_t AttribCount;
	uint32_t VertexCount;
	uint32_t TotalParamSize;
	ParamWalker ParamWalkers[ MAX_SHADER_PARAMETERS ];
	uint32_t ParamCount;
	const void* Indices;
	IndexWalkerFn IndexWalker;
	uint32_t ParamStride;
	std::vector< glm::vec4 > Verts;
	std::vector< float > Data;
	uint32_t w,h;
};

void ProcessTriangleIndices( DrawContext& Context )
{
	Context.Verts.resize( Context.VertexCount );
	
	for ( uint32_t j = 0u; j < Context.VertexCount; ++j )
	{
		for ( uint32_t i = 0u; i < Context.AttribCount; ++i )
		{
			auto& walker = Context.Walkers[ i ];
			walker.Fn( walker.Input, walker.Output, walker.Stride );
		}

		Context.Run();
		Context.Verts[ j ] = *Context.VertPos;

		for ( uint32_t i = 0u; i < Context.AttribCount; ++i )
		{
			IncrementParamWalker( &Context.ParamWalkers[ i ] );
		}
	}
}

void ProcessTriangles( DrawContext& Context )
{
	const uint32_t TriangleCount = Context.VertexCount / 3;

	for ( uint32_t i = 0; i < TriangleCount; ++i )
	{
		const uint32_t I0 = Context.IndexWalker(Context.Indices);
		const uint32_t I1 = Context.IndexWalker(Context.Indices);
		const uint32_t I2 = Context.IndexWalker(Context.Indices);

		float* Params0 = Context.ParamStride == 0u ? nullptr : &Context.Data[ I0 * Context.ParamStride / sizeof( float ) ];
		float* Params1 = Context.ParamStride == 0u ? nullptr : &Context.Data[ I1 * Context.ParamStride / sizeof( float ) ];
		float* Params2 = Context.ParamStride == 0u ? nullptr : &Context.Data[ I2 * Context.ParamStride / sizeof( float ) ];

		// Verts in clip space.
		glm::vec4 VertPos0 = Context.Verts[ I0 ];
		glm::vec4 VertPos1 = Context.Verts[ I1 ];
		glm::vec4 VertPos2 = Context.Verts[ I2 ];

		const float inv_w0 = 1.0f / VertPos0.w;
		const float inv_w1 = 1.0f / VertPos1.w;
		const float inv_w2 = 1.0f / VertPos2.w;

		for ( uint32_t j = 0u; j < Context.ParamStride / sizeof( float ); ++j )
		{
			//Params0[ j ] *= inv_w0;
			//Params1[ j ] *= inv_w1;
			//Params2[ j ] *= inv_w2;
		}

		// Verts in NDC space.
		VertPos0 *= inv_w0;
		VertPos1 *= inv_w1;
		VertPos2 *= inv_w2;

		VertPos0.w = inv_w0;
		VertPos1.w = inv_w1;
		VertPos2.w = inv_w2;

		// Clip the triangle.
		// ...

		// Convert to screen space.
		VertPos0 += glm::vec4{ 1.0f, 1.0f, 0.0f, 0.0f };
		VertPos1 += glm::vec4{ 1.0f, 1.0f, 0.0f, 0.0f };
		VertPos2 += glm::vec4{ 1.0f, 1.0f, 0.0f, 0.0f };
		
		VertPos0 *= glm::vec4{ 0.5f * ( float )Context.w, 0.5f * ( float )Context.h, 1.0f, 1.0f };
		VertPos1 *= glm::vec4{ 0.5f * ( float )Context.w, 0.5f * ( float )Context.h, 1.0f, 1.0f };
		VertPos2 *= glm::vec4{ 0.5f * ( float )Context.w, 0.5f * ( float )Context.h, 1.0f, 1.0f };

		ConsoleGL::Tri t;
		t.p0 = ConsoleGL::Coord( VertPos0.x, VertPos0.y );
		t.p1 = ConsoleGL::Coord( VertPos1.x, VertPos1.y );
		t.p2 = ConsoleGL::Coord( VertPos2.x, VertPos2.y );

		t.p0.y = Context.h - t.p0.y;
		t.p1.y = Context.h - t.p1.y;
		t.p2.y = Context.h - t.p2.y;



		

		struct Payload
		{
			float y1;
			float y2;
			float y3;
			float x1;
			float x2;
			float x3;

			float denom;
			glm::vec3 c1;
			glm::vec3 c2;
			glm::vec3 c3;

		} payload;

		
		payload.y1 = t.p0.y;
		payload.y2 = t.p1.y;
		payload.y3 = t.p2.y;
		payload.x1 = t.p0.x;
		payload.x2 = t.p1.x;
		payload.x3 = t.p2.x;
		payload.denom = (payload.y2-payload.y3)*(payload.x1-payload.x3)+(payload.x3-payload.x2)*(payload.y1-payload.y3);
		payload.c1 = *(glm::vec3*)Params0;
		payload.c2 = *(glm::vec3*)Params1;
		payload.c3 = *(glm::vec3*)Params2;

		ConsoleGL::FragmentFn FragFn = +[]( ConsoleGL::Coord c, void* s) -> ConsoleGL::Pixel
		{
			Payload* sp = ( Payload* )s;

			const float Lam1 = ((sp->y2-sp->y3)*((float)c.x-sp->x3)+(sp->x3-sp->x2)*((float)c.y-sp->y3))/sp->denom;
			const float Lam2 = ((sp->y3-sp->y1)*((float)c.x-sp->x3)+(sp->x1-sp->x3)*((float)c.y-sp->y3))/sp->denom;
			const float Lam3 = 1.0f - Lam1 - Lam2;

			glm::vec3 Col = Lam1 * sp->c1 + Lam2 * sp->c2 + Lam3 * sp->c3;
			return *ConsoleGL::MapColourToPixel({ Col.x, Col.y, Col.z, 1.0f});
		};

		ConsoleGL::DrawTriangleFilledFn( ConsoleGL::GetWindowBuffer( ConsoleGL::GetActiveWindow() ), t, FragFn, &payload );

		float Params[ 256 ];

		if ( VertPos0.x > 0 )
		{
			
		}
	}
}

#pragma endregion

bool ConsoleGL::DrawElements( const ERenderMode a_Mode, const uint32_t a_Count, const EDataType a_DataType, const void* a_Indices )
{
	if ( !State.ActiveContext )
	{
		Error::SetLastError( Error_NoActiveContext );
		return false;
	}

	const IndexWalkerFn IndexWalker = GetIndexWalkerFn( a_DataType );

	if ( !IndexWalker )
	{
		Error::SetLastError( Error_UnsupportedDataType );
		return false;
	}

	DrawContext c;
	c.Run = State.ActiveContext->ActiveShaderProgram->Entries[ ( size_t )EShaderType::Vertex ].Proc->Run;
	c.FragRun = State.ActiveContext->ActiveShaderProgram->Entries[ ( size_t )EShaderType::Fragment ].Proc->Run;
	c.VertPos = ( const glm::vec4* )State.ActiveContext->ActiveShaderProgram->InbuiltVars[ ( size_t )EShaderInbuiltVar::out_vec4_VertPos ];
	c.AttribCount = 0u;
	c.VertexCount = a_Count;

	const VertexArrayHandle VA = State.ActiveContext->ActiveVertexArray ? State.ActiveContext->ActiveVertexArray : &State.ActiveContext->DefaultVertexArray;
	uint32_t CurrentOffset = 0u;
	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( VA->Attribs[ i ].Enabled )
		{
			auto& attr = VA->Attribs[ i ];
			auto& walk = c.Walkers[ c.AttribCount++ ];

			walk.Fn = GetAttribWalkerFn( attr.DataType );
			walk.Input = attr.Buffer->Data.data() + CurrentOffset;
			walk.Output = State.ActiveContext->ActiveShaderProgram->Attributes[ i ].value().Data;
			walk.Stride = attr.Stride;

			CurrentOffset += attr.Size * 12;
		}
	}

	uint32_t Stride = 0u;
	uint32_t Offset = 0u;
	c.TotalParamSize = 0u;
	c.ParamCount = 0u;

	for ( uint32_t i = 0; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( !State.ActiveContext->ActiveShaderProgram->Parameters[ i ].has_value() )
		{
			break;
		}

		auto& param = State.ActiveContext->ActiveShaderProgram->Parameters[ i ].value();

		c.TotalParamSize+= param.Size;
	}

	c.Data.resize( c.TotalParamSize * c.VertexCount / sizeof( float ) );

	for ( uint32_t i = 0; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( !State.ActiveContext->ActiveShaderProgram->Parameters[ i ].has_value() )
		{
			break;
		}

		auto& param = State.ActiveContext->ActiveShaderProgram->Parameters[ i ].value();

		auto& pw = c.ParamWalkers[ c.ParamCount++ ];
		pw.Input = param.DataVertex;
		pw.Size = param.Size;

		Stride += pw.Size;
		c.ParamWalkers[ i ].Output = ( uint8_t* )c.Data.data() + Offset;
		Offset += pw.Size;
	}

	for ( uint32_t i = 0u; i < c.ParamCount; ++i )
	{
		c.ParamWalkers[ i ].Stride = Stride;
	}

	c.ParamStride = Stride;
	c.IndexWalker = GetIndexWalkerFn( a_DataType );
	c.Indices = a_Indices;

	c.w = ConsoleGL::GetWindowWidth( ConsoleGL::GetActiveWindow() ) - 1;
	c.h = ConsoleGL::GetWindowHeight( ConsoleGL::GetActiveWindow() ) - 1;

	ProcessTriangleIndices( c );
	ProcessTriangles( c );

	return true;
}

//...

int32_t ConsoleGL::GetUniformLocation( const ShaderProgramHandle a_ShaderProgram, const char* a_Name )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return -1;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return -1;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return -1;
	}

	for ( int32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && a_ShaderProgram->Uniforms[ i ].value().Name == a_Name )
		{
			return i;
		}
	}

	return -1;
}

uint32_t ConsoleGL::GetUniformCount( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return 0u;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return -1;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return -1;
	}

	uint32_t Count = 0u;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() )
		{
			++Count;
		}
	}

	return Count;
}

const char* ConsoleGL::GetUniformName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Uniforms[ i ].value().Name.c_str();
		}
	}

	Error::SetLastError( Error_UniformIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetUniformType( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, ConsoleGL::EDataType& o_DataType )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && Count-- == 0u )
		{
			o_DataType = a_ShaderProgram->Uniforms[ i ].value().DataType;
			return true;
		}
	}

	Error::SetLastError( Error_UniformIndexOutOfRange );
	return false;
}

const char* ConsoleGL::GetUniformTypeName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Uniforms[ i ].value().DataTypeName.c_str();
		}
	}

	Error::SetLastError( Error_UniformIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetUniformArrayLength( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_ArrayLength )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && Count-- == 0u )
		{
			o_ArrayLength = a_ShaderProgram->Uniforms[ i ].value().Length;
			return true;
		}
	}

	Error::SetLastError( Error_UniformIndexOutOfRange );
	return false;
}

bool ConsoleGL::GetUniformSize( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_Size )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Uniforms[ i ].has_value() && Count-- == 0u )
		{
			o_Size = a_ShaderProgram->Uniforms[ i ].value().Size;
			return true;
		}
	}

	Error::SetLastError( Error_UniformIndexOutOfRange );
	return false;
}

#define UNIFORM_CHECK() \
	if ( !State.ActiveContext ) \
	{ \
		Error::SetLastError( Error_NoActiveContext ); \
		return; \
	} \
 \
	if ( !State.ActiveContext->ActiveShaderProgram ) \
	{ \
		Error::SetLastError( Error_NoActiveShaderProgram ); \
		return; \
	} \
 \
	const auto& Uniform = State.ActiveContext->ActiveShaderProgram->Uniforms[ a_Location ]; \
 \
	if ( !Uniform.has_value() ) \
	{ \
		Error::SetLastError( Error_InvalidUniformLocation ); \
		return; \
	} \

#define SET_UNIFORM_VAL( Type, Index, Val ) \
	static_cast< Type* >( Uniform.value().Data )[ Index ] = Val

void ConsoleGL::Uniform1f( const int32_t a_Location, const float a_V0 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( float, 0u, a_V0 );
}

void ConsoleGL::Uniform2f( const int32_t a_Location, const float a_V0, const float a_V1 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( float, 0u, a_V0 );
	SET_UNIFORM_VAL( float, 1u, a_V1 );
}

void ConsoleGL::Uniform3f( const int32_t a_Location, const float a_V0, const float a_V1, const float a_V2 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( float, 0u, a_V0 );
	SET_UNIFORM_VAL( float, 1u, a_V1 );
	SET_UNIFORM_VAL( float, 2u, a_V2 );
}

void ConsoleGL::Uniform4f( const int32_t a_Location, const float a_V0, const float a_V1, const float a_V2, const float a_V3 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( float, 0u, a_V0 );
	SET_UNIFORM_VAL( float, 1u, a_V1 );
	SET_UNIFORM_VAL( float, 2u, a_V2 );
	SET_UNIFORM_VAL( float, 3u, a_V3 );
}

void ConsoleGL::Uniform1i( const int32_t a_Location, const int32_t a_V0 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( int32_t, 0u, a_V0 );
}

void ConsoleGL::Uniform2i( const int32_t a_Location, const int32_t a_V0, const int32_t a_V1 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( int32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( int32_t, 1u, a_V1 );
}

void ConsoleGL::Uniform3i( const int32_t a_Location, const int32_t a_V0, const int32_t a_V1, const int32_t a_V2 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( int32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( int32_t, 1u, a_V1 );
	SET_UNIFORM_VAL( int32_t, 2u, a_V2 );
}

void ConsoleGL::Uniform4i( const int32_t a_Location, const int32_t a_V0, const int32_t a_V1, const int32_t a_V2, const int32_t a_V3 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( int32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( int32_t, 1u, a_V1 );
	SET_UNIFORM_VAL( int32_t, 2u, a_V2 );
	SET_UNIFORM_VAL( int32_t, 3u, a_V3 );
}

void ConsoleGL::Uniform1ui( const int32_t a_Location, const uint32_t a_V0 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( uint32_t, 0u, a_V0 );
}

void ConsoleGL::Uniform2ui( const int32_t a_Location, const uint32_t a_V0, const uint32_t a_V1 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( uint32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( uint32_t, 1u, a_V1 );
}

void ConsoleGL::Uniform3ui( const int32_t a_Location, const uint32_t a_V0, const uint32_t a_V1, const uint32_t a_V2 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( uint32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( uint32_t, 1u, a_V1 );
	SET_UNIFORM_VAL( uint32_t, 2u, a_V2 );
}

void ConsoleGL::Uniform4ui( const int32_t a_Location, const uint32_t a_V0, const uint32_t a_V1, const uint32_t a_V2, const uint32_t a_V3 )
{
	UNIFORM_CHECK();
	SET_UNIFORM_VAL( uint32_t, 0u, a_V0 );
	SET_UNIFORM_VAL( uint32_t, 1u, a_V1 );
	SET_UNIFORM_VAL( uint32_t, 2u, a_V2 );
	SET_UNIFORM_VAL( uint32_t, 3u, a_V3 );
}

void ConsoleGL::Uniform1fv( const int32_t a_Location, const uint32_t a_Count, const float* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 1u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::Uniform2fv( const int32_t a_Location, const uint32_t a_Count, const float* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 2u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::Uniform3fv( const int32_t a_Location, const uint32_t a_Count, const float* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 3u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::Uniform4fv( const int32_t a_Location, const uint32_t a_Count, const float* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 4u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::Uniform1iv( const int32_t a_Location, const uint32_t a_Count, const int32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 1u; ++i )
		SET_UNIFORM_VAL( int32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform2iv( const int32_t a_Location, const uint32_t a_Count, const int32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 2u; ++i )
		SET_UNIFORM_VAL( int32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform3iv( const int32_t a_Location, const uint32_t a_Count, const int32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 3u; ++i )
		SET_UNIFORM_VAL( int32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform4iv( const int32_t a_Location, const uint32_t a_Count, const int32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 4u; ++i )
		SET_UNIFORM_VAL( int32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform1uiv( const int32_t a_Location, const uint32_t a_Count, const uint32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 1u; ++i )
		SET_UNIFORM_VAL( uint32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform2uiv( const int32_t a_Location, const uint32_t a_Count, const uint32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 2u; ++i )
		SET_UNIFORM_VAL( uint32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform3uiv( const int32_t a_Location, const uint32_t a_Count, const uint32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 3u; ++i )
		SET_UNIFORM_VAL( uint32_t, i, a_Value[ i ] );
}

void ConsoleGL::Uniform4uiv( const int32_t a_Location, const uint32_t a_Count, const uint32_t* a_Value )
{
	UNIFORM_CHECK();
	for ( uint32_t i = 0u; i < a_Count * 4u; ++i )
		SET_UNIFORM_VAL( uint32_t, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix2fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat2, i, glm::transpose( glm::make_mat2( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 4u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix3fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat3, i, glm::transpose( glm::make_mat3( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 9u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix4fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat4, i, glm::transpose( glm::make_mat4( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 16u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix2x3fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat3x2, i, glm::transpose( glm::make_mat2x3( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 6u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix3x2fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat2x3, i, glm::transpose( glm::make_mat3x2( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 6u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix2x4fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat4x2, i, glm::transpose( glm::make_mat2x4( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 8u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix4x2fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat2x4, i, glm::transpose( glm::make_mat4x2( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 8u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix3x4fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat4x3, i, glm::transpose( glm::make_mat3x4( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 12u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

void ConsoleGL::UniformMatrix4x3fv( const int32_t a_Location, const uint32_t a_Count, const bool a_Transpose, const float* a_Value )
{
	UNIFORM_CHECK();

	if ( a_Transpose )
	{
		for ( uint32_t i = 0u; i < a_Count; ++i )
		{
			SET_UNIFORM_VAL( glm::mat3x4, i, glm::transpose( glm::make_mat4x3( a_Value ) ) );
		}

		return;
	}

	for ( uint32_t i = 0u; i < a_Count * 12u; ++i )
		SET_UNIFORM_VAL( float, i, a_Value[ i ] );
}

#undef UNIFORM_CHECK
#undef SET_UNIFORM_VAL

int32_t ConsoleGL::GetAttribLocation( const ShaderProgramHandle a_ShaderProgram, const char* a_Name )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return -1;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return -1;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return -1;
	}

	for ( int32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && a_ShaderProgram->Attributes[ i ].value().Name == a_Name )
		{
			return i;
		}
	}

	return -1;
}

uint32_t ConsoleGL::GetAttribCount( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return 0u;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return -1;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return -1;
	}

	uint32_t Count = 0u;

	for ( uint32_t i = 0u; i < MAX_SHADER_UNIFORMS; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() )
		{
			++Count;
		}
	}

	return Count;
}

const char* ConsoleGL::GetAttribName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Attributes[ i ].value().Name.c_str();
		}
	}

	Error::SetLastError( Error_AttributeIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetAttribType( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, ConsoleGL::EDataType& o_DataType )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && Count-- == 0u )
		{
			o_DataType = a_ShaderProgram->Attributes[ i ].value().DataType;
			return true;
		}
	}

	Error::SetLastError( Error_AttributeIndexOutOfRange );
	return false;
}

const char* ConsoleGL::GetAttribTypeName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Attributes[ i ].value().DataTypeName.c_str();
		}
	}

	Error::SetLastError( Error_AttributeIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetAttribArrayLength( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_ArrayLength )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && Count-- == 0u )
		{
			o_ArrayLength = a_ShaderProgram->Attributes[ i ].value().Length;
			return true;
		}
	}

	Error::SetLastError( Error_AttributeIndexOutOfRange );
	return false;
}

bool ConsoleGL::GetAttribSize( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_Size )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_ATTRIBUTES; ++i )
	{
		if ( a_ShaderProgram->Attributes[ i ].has_value() && Count-- == 0u )
		{
			o_Size = a_ShaderProgram->Attributes[ i ].value().Size;
			return true;
		}
	}

	Error::SetLastError( Error_AttributeIndexOutOfRange );
	return false;
}

uint32_t ConsoleGL::GetParamCount( const ShaderProgramHandle a_ShaderProgram )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return 0u;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return -1;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return -1;
	}

	uint32_t Count = 0u;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() )
		{
			++Count;
		}
	}

	return Count;
}

bool ConsoleGL::GetParamInterpType( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, EShaderInterpQual& o_InterpQual )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			o_InterpQual = a_ShaderProgram->Parameters[ i ].value().InterpQual;
			return true;
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return false;
}

const char* ConsoleGL::GetParamName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Parameters[ i ].value().Name.c_str();
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetParamType( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, ConsoleGL::EDataType& o_DataType )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			o_DataType = a_ShaderProgram->Parameters[ i ].value().DataType;
			return true;
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return false;
}

const char* ConsoleGL::GetParamTypeName( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return nullptr;
	}

	if ( !State.ShaderPrograms.IsValid( a_ShaderProgram ) )
	{
		Error::SetLastError( Error_InvalidShaderProgramHandle );
		return nullptr;
	}

	if ( !a_ShaderProgram->IsLinked )
	{
		Error::SetLastError( Error_ShaderProgramNotLinked );
		return nullptr;
	}

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			return a_ShaderProgram->Parameters[ i ].value().DataTypeName.c_str();
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return nullptr;
}

bool ConsoleGL::GetParamArrayLength( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_ArrayLength )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			o_ArrayLength = a_ShaderProgram->Parameters[ i ].value().Length;
			return true;
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return false;
}

bool ConsoleGL::GetParamSize( const ShaderProgramHandle a_ShaderProgram, const uint32_t a_Index, uint32_t& o_Size )
{
	if ( !a_ShaderProgram )
	{
		Error::SetLastError( Error_ArgumentError );
		return false;
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

	uint32_t Count = a_Index;

	for ( uint32_t i = 0u; i < MAX_SHADER_PARAMETERS; ++i )
	{
		if ( a_ShaderProgram->Parameters[ i ].has_value() && Count-- == 0u )
		{
			o_Size = a_ShaderProgram->Parameters[ i ].value().Size;
			return true;
		}
	}

	Error::SetLastError( Error_ParameterIndexOutOfRange );
	return false;
}

#pragma endregion






















#include <glm/gtc/matrix_transform.hpp>

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
uniform(2) mat4 M;

attrib(0) vec3 Position;
attrib(1) vec3 Colour;

out vec4 VertPos;
affn out vec3 VertexColour;

void run()
{
	VertPos = P * V * M * vec4(Position, 1.0f);
	VertexColour = Colour;
}
)";

	ConsoleGL::SetShaderSourceFromString( DefaultVertexShader, VertexShaderSource.c_str(), VertexShaderSource.size() );
	ConsoleGL::CompileShader( DefaultVertexShader );
	ConsoleGL::AttachShader( DefaultShaderProgram, DefaultVertexShader );

	// Create fragment shader.
	ConsoleGL::Shader* DefaultFragmentShader = ConsoleGL::CreateShader( ConsoleGL::EShaderType::Fragment );
	const std::string FragmentShaderSource = R"(

out vec4 FragColour;
in vec3 VertexColour;

void run()
{
	FragColour = vec4(VertexColour, 1.0f);
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

	const float Vertices[] = {
		-0.5f, +0.5f, +0.5f, 0.0f, 1.0f, 1.0f, // Top left front
		+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f, // Top right front
		+0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Top right back
		-0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Top left back
		-0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 1.0f, // Bot left front
		+0.5f, -0.5f, +0.5f, 1.0f, 0.0f, 1.0f, // Bot right front
		+0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Bot right back
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bot left back
	};

	const int32_t Indices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 1, 4, 1, 0,
		7, 6, 5, 7, 5, 4,
		3, 2, 6, 3, 6, 7,
		4, 0, 3, 4, 3, 7,
		1, 5, 6, 1, 6, 2
	};

	// Create a vbo.
	BufferHandle VBO;
	ConsoleGL::CreateBuffers( 1, &VBO );
	ConsoleGL::BindBuffer( EBufferTarget::ArrayBuffer, VBO );
	ConsoleGL::BufferData( EBufferTarget::ArrayBuffer, sizeof( Vertices ), Vertices );
	ConsoleGL::BindBuffer( EBufferTarget::ArrayBuffer, nullptr );
;
	VertexArrayHandle VAO;
	ConsoleGL::CreateVertexArrays( 1, &VAO );
	ConsoleGL::BindVertexArray( VAO );
	ConsoleGL::BindBuffer( EBufferTarget::ArrayBuffer, VBO );
	ConsoleGL::VertexAttribPointer( 0u, 1u, EDataType::Vec3, false, sizeof( glm::vec3 ) * 2u, 0u );
	ConsoleGL::VertexAttribPointer( 1u, 1u, EDataType::Vec3, false, sizeof( glm::vec3 ) * 2u, sizeof( glm::vec3 ) );
	ConsoleGL::BindBuffer( EBufferTarget::ArrayBuffer, nullptr );
	ConsoleGL::EnableVertexAttribArray( 0u );
	ConsoleGL::EnableVertexAttribArray( 1u );
	ConsoleGL::BindVertexArray( nullptr );

	float CameraX = 0.0f, CameraY = 0.0f, CameraZ = 3.0f;
	float ObjRotX = 0.0f, ObjRotY = 0.0f, ObjRotZ = 0.0f;

	// Use the program now for current context.
	while ( true )
	{
		ConsoleGL::PollEvents();
		
		ObjRotZ += 0.005;
		ObjRotY += 0.01;
		ObjRotX += 0.001;

		if ( ConsoleGL::IsKeyDown( EKeyboardKey::W ) ) CameraZ -= 0.01f;
		if ( ConsoleGL::IsKeyDown( EKeyboardKey::S ) ) CameraZ += 0.01f;
		if ( ConsoleGL::IsKeyDown( EKeyboardKey::A ) ) CameraX -= 0.01f;
		if ( ConsoleGL::IsKeyDown( EKeyboardKey::D ) ) CameraX += 0.01f;
		if ( ConsoleGL::IsKeyDown( EKeyboardKey::E ) ) CameraY += 0.01f;
		if ( ConsoleGL::IsKeyDown( EKeyboardKey::Q ) ) CameraY -= 0.01f;

		float AspectRatio = ( float )Width / Height;
		float FOV = glm::radians(75.0f);
		float ZNear = 0.1f;
		float ZFar = 1000.0f;
		glm::vec3 CameraPos = { CameraX, CameraY, CameraZ };
		glm::vec3 ObjectPos = { 0.0f, 0.0f, 0.0f };
		glm::vec3 ObjectRot = { ObjRotX, ObjRotY, ObjRotZ };
		glm::vec3 ObjectSca = { 1.0f, 1.0f, 1.0f };

		glm::mat4 M = glm::translate( glm::mat4( 1.0f ), ObjectPos ) * glm::mat4_cast(glm::quat(ObjectRot));
		glm::mat4 V = glm::lookAt( CameraPos, CameraPos + glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } );
		glm::mat4 P = glm::perspective( FOV, AspectRatio, ZNear, ZFar );

		int32_t MLoc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "M" );
		int32_t VLoc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "V" );
		int32_t PLoc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "P" );

		ConsoleGL::UseProgram( DefaultShaderProgram );

		ConsoleGL::UniformMatrix4fv( MLoc, 1, false, &M[ 0 ][ 0 ] );
		ConsoleGL::UniformMatrix4fv( VLoc, 1, false, &V[ 0 ][ 0 ] );
		ConsoleGL::UniformMatrix4fv( PLoc, 1, false, &P[ 0 ][ 0 ] );

		ConsoleGL::BindVertexArray( VAO );
		ConsoleGL::SetBuffer( ConsoleGL::GetWindowBuffer(ConsoleGL::GetActiveWindow()), *ConsoleGL::MapColourToPixel({ 1.0f, 1.0f, 1.0f, 1.0f }));
		ConsoleGL::DrawElements( ERenderMode::Triangles, 36u, EDataType::Int32, Indices );
		ConsoleGL::BindVertexArray( nullptr );
		ConsoleGL::SwapWindowBuffer();
		ConsoleGL::UseProgram( nullptr );
	}
}
