#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <Windows.h>

TODO( "All this crap needs to be cleaned up properly." );

std::string GetVisualStudioVersion()
{
	auto exec = [](std::string cmd) -> std::string{
		HANDLE hPipeRead, hPipeWrite;
		SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
		saAttr.bInheritHandle = TRUE; // Allows the pipe handles to be inherited by the child process
		saAttr.lpSecurityDescriptor = nullptr;

		// Create a pipe for the child process's STDOUT
		if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
		    return "";
		}

		// Ensure the read handle to the pipe for STDOUT is not inherited
		SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);

		// Create the child process
		PROCESS_INFORMATION piProcInfo = {};
		STARTUPINFOA siStartInfo = {};
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdOutput = hPipeWrite;
		siStartInfo.hStdError = hPipeWrite;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		std::vector<char> cmdline(cmd.begin(), cmd.end());
		cmdline.push_back(0);

		if (!CreateProcessA(nullptr, cmdline.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &siStartInfo, &piProcInfo)) {
		    CloseHandle(hPipeWrite);
		    CloseHandle(hPipeRead);
		    return "";
		}

		// Close the pipe handle so that no more data is written to the pipe
		CloseHandle(hPipeWrite);

		// Read the output of the child process
		char buffer[128];
		DWORD bytesRead;
		std::string result;
		while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) != FALSE) {
		    buffer[bytesRead] = '\0';
		    result += buffer;
		}

		// Clean up
		CloseHandle(hPipeRead);
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		return result;
	};

	std::string Response = exec("where devenv");

    std::regex pattern("\\\\[0-9][0-9][0-9][0-9]\\\\");
    std::smatch matches;

    if (!std::regex_search(Response, matches, pattern)) {
        return "";
    }

	std::string VSYear = matches[ 0 ].str().substr( 1u, 4u );

	return "vs" + VSYear;
}

std::string StripComments( const std::string& a_Input )
{
    const std::regex CommentRegex( R"(//.*$)" );
    return std::regex_replace( a_Input, CommentRegex, "" );
}

bool ExtractLayout( const std::string& a_Input, std::string& o_Output )
{
	const std::string CommentStrippedInput = StripComments( a_Input );
	const std::regex LayoutRegex( R"(struct\s+Layout\s*\{([^}]*)\})" );
    
    if ( std::smatch LayoutMatch; std::regex_search( CommentStrippedInput, LayoutMatch, LayoutRegex ) ) 
	{
        o_Output = LayoutMatch[ 1 ].str();
		return true;
    }

	// Error
    return false;
}

struct FieldInfo
{
	std::string Type;
	std::string Prefix;
	std::string Name;
};

bool ExtractFields( const std::string& a_Input, std::vector< FieldInfo >& o_Output )
{
	// Supported types:
	// - int8|int16|int32|int64|uint8|uint16|uint32|uint64|float|double|bool
	// - mat2|mat3|mat4
	// - mat2x3|mat2x4|mat3x2|mat3x4|mat4x2|mat4x3
	// - dmat2|dmat3|dmat4
	// - dmat2x3|dmat2x4|dmat3x2|dmat3x4|dmat4x2|dmat4x3
	// - vec2|vec3|vec4
	// - dvec2|dvec3|dvec4
	// - ivec2|ivec3|ivec4
	// - uvec2|uvec3|uvec4

	std::regex FieldRegex( R"((int8|int16|int32|int64|uint8|uint16|uint32|uint64|float|double|bool|mat2|mat3|mat4|mat2x3|mat2x4|mat3x2|mat3x4|mat4x2|mat4x3|dmat2|dmat3|dmat4|dmat2x3|dmat2x4|dmat3x2|dmat3x4|dmat4x2|dmat4x3|vec2|vec3|vec4|dvec2|dvec3|dvec4|ivec2|ivec3|ivec4|uvec2|uvec3|uvec4)\s+(\w+)_(\w+)\s*;)" );
	auto Begin = std::sregex_iterator( a_Input.begin(), a_Input.end(), FieldRegex );
    auto End = std::sregex_iterator();

    for ( std::sregex_iterator i = Begin; i != End; ++i ) 
	{
        std::smatch Match = *i;

		if ( Match.size() < 4u )
		{
			// Error
			return false;
		}

        o_Output.push_back( { Match[ 1u ].str(), Match[ 2u ].str(), Match[ 3u ].str() } );
    }

    return true;
}

enum class EIOType
{
	Uniform,
	In,
	Vin,
	Out,
	Vout,
	Attr,
};

enum class EDataType
{
	DT_int8,
	DT_int16,
	DT_int32,
	DT_int64,
	DT_uint8,
	DT_uint16,
	DT_uint32,
	DT_uint64,
	DT_float,
	DT_double,
	DT_bool,
	DT_mat2,
	DT_mat3,
	DT_mat4,
	DT_mat2x3,
	DT_mat2x4,
	DT_mat3x2,
	DT_mat3x4,
	DT_mat4x2,
	DT_mat4x3,
	DT_dmat2,
	DT_dmat3,
	DT_dmat4,
	DT_dmat2x3,
	DT_dmat2x4,
	DT_dmat3x2,
	DT_dmat3x4,
	DT_dmat4x2,
	DT_dmat4x3,
	DT_vec2,
	DT_vec3,
	DT_vec4,
	DT_dvec2,
	DT_dvec3,
	DT_dvec4,
	DT_ivec2,
	DT_ivec3,
	DT_ivec4,
	DT_uvec2,
	DT_uvec3,
	DT_uvec4,
};

enum class EInbuiltVarType
{
	IF_None,

	// The clip-space output position of the current vertex
	IF_vec4_out_Position,

	// The location of the fragment in window space
	IF_vec4_in_FragCoord,

	// The colour to set the fragment
	IF_vec4_out_FragColour,

	// The depth to assign to the fragment.
	IF_float_out_FragDepth
};

struct Field
{
	EDataType DataType;
	std::string DataTypeString;
	EIOType IOType;
	EInbuiltVarType InbuiltVarType = EInbuiltVarType::IF_None;
	std::string Name;
	std::string FieldString;
	uint32_t Slot = -1;
};

bool ProcessField( const FieldInfo& a_Input, Field& o_Field )
{
	bool ValidType = true;

	if ( a_Input.Type == "" ) ValidType = false;
	else if ( a_Input.Type == "int8"	) { o_Field.DataType = EDataType::DT_int8	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "int16"	) { o_Field.DataType = EDataType::DT_int16	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "int32"	) { o_Field.DataType = EDataType::DT_int32	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "int64"	) { o_Field.DataType = EDataType::DT_int64	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uint8"	) { o_Field.DataType = EDataType::DT_uint8	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uint16"	) { o_Field.DataType = EDataType::DT_uint16	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uint32"	) { o_Field.DataType = EDataType::DT_uint32	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uint64"	) { o_Field.DataType = EDataType::DT_uint64	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "float"	) { o_Field.DataType = EDataType::DT_float	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "double"	) { o_Field.DataType = EDataType::DT_double	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "bool"	) { o_Field.DataType = EDataType::DT_bool	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat2"	) { o_Field.DataType = EDataType::DT_mat2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat3"	) { o_Field.DataType = EDataType::DT_mat3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat4"	) { o_Field.DataType = EDataType::DT_mat4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat2x3"	) { o_Field.DataType = EDataType::DT_mat2x3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat2x4"	) { o_Field.DataType = EDataType::DT_mat2x4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat3x2"	) { o_Field.DataType = EDataType::DT_mat3x2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat3x4"	) { o_Field.DataType = EDataType::DT_mat3x4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat4x2"	) { o_Field.DataType = EDataType::DT_mat4x2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "mat4x3"	) { o_Field.DataType = EDataType::DT_mat4x3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat2"	) { o_Field.DataType = EDataType::DT_dmat2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat3"	) { o_Field.DataType = EDataType::DT_dmat3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat4"	) { o_Field.DataType = EDataType::DT_dmat4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat2x3" ) { o_Field.DataType = EDataType::DT_dmat2x3; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat2x4" ) { o_Field.DataType = EDataType::DT_dmat2x4; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat3x2" ) { o_Field.DataType = EDataType::DT_dmat3x2; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat3x4" ) { o_Field.DataType = EDataType::DT_dmat3x4; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat4x2" ) { o_Field.DataType = EDataType::DT_dmat4x2; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dmat4x3" ) { o_Field.DataType = EDataType::DT_dmat4x3; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "vec2"	) { o_Field.DataType = EDataType::DT_vec2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "vec3"	) { o_Field.DataType = EDataType::DT_vec3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "vec4"	) { o_Field.DataType = EDataType::DT_vec4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dvec2"	) { o_Field.DataType = EDataType::DT_dvec2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dvec3"	) { o_Field.DataType = EDataType::DT_dvec3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "dvec4"	) { o_Field.DataType = EDataType::DT_dvec4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "ivec2"	) { o_Field.DataType = EDataType::DT_ivec2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "ivec3"	) { o_Field.DataType = EDataType::DT_ivec3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "ivec4"	) { o_Field.DataType = EDataType::DT_ivec4	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uvec2"	) { o_Field.DataType = EDataType::DT_uvec2	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uvec3"	) { o_Field.DataType = EDataType::DT_uvec3	; o_Field.DataTypeString = a_Input.Type; }
	else if ( a_Input.Type == "uvec4"	) { o_Field.DataType = EDataType::DT_uvec4	; o_Field.DataTypeString = a_Input.Type; }
	else ValidType = false;

	if ( !ValidType )
	{
		// Error
		return false;
	}

	bool ValidIOType = true;

	if ( a_Input.Prefix == "" ) ValidIOType = false;
	else if ( a_Input.Prefix == "uni"	) { o_Field.IOType = EIOType::Uniform	; }
	else if ( a_Input.Prefix == "in"	) { o_Field.IOType = EIOType::In		; }
	else if ( a_Input.Prefix == "vin"	) { o_Field.IOType = EIOType::Vin		; }
	else if ( a_Input.Prefix == "out"	) { o_Field.IOType = EIOType::Out		; }
	else if ( a_Input.Prefix == "vout"	) { o_Field.IOType = EIOType::Vout		; }
	else if ( a_Input.Prefix.starts_with( "attr" ) )
	{
		const std::string SlotString = a_Input.Prefix.substr( 4u );

		if ( SlotString.find_first_not_of( "01234567890" ) != std::string::npos )
		{
			// Error, Invalid attribute slot number
			return false;
		}

		o_Field.Slot = std::stoi( SlotString );
		o_Field.IOType = EIOType::Attr;
	}
	else ValidIOType = false;

	if ( !ValidIOType )
	{
		// Error
		return false;
	}

	o_Field.Name = a_Input.Name;

	// Check if this is an inbuilt variable.

	// Is this a position variable?
	if ( a_Input.Name == "Position" )
	{
		// It must be either Vout or Out, since this is 
		if ( o_Field.IOType != EIOType::Out )
		{
			// Error, Inbuilt Variable 'Position' must be Out
			return false;
		}

		if ( o_Field.DataType != EDataType::DT_vec4 )
		{
			// Error, Inbuilt Variable 'Position' must be vec4.
			return false;
		}

		o_Field.InbuiltVarType = EInbuiltVarType::IF_vec4_out_Position;
	}
	else if ( a_Input.Name == "FragCoord" )
	{
		// It must be either Vout or Out, since this is 
		if ( o_Field.IOType != EIOType::In )
		{
			// Error, Inbuilt Variable 'FragCoord' must be In
			return false;
		}

		if ( o_Field.DataType != EDataType::DT_vec4 )
		{
			// Error, Inbuilt Variable 'FragCoord' must be vec4.
			return false;
		}

		o_Field.InbuiltVarType = EInbuiltVarType::IF_vec4_in_FragCoord;
	}
	else if ( a_Input.Name == "FragColour" )
	{
		// It must be Out
		if ( o_Field.IOType != EIOType::Out )
		{
			// Error, Inbuilt Variable 'FragColour' must be Out
			return false;
		}

		if ( o_Field.DataType != EDataType::DT_vec4 )
		{
			// Error, Inbuilt Variable 'FragColour' must be vec4.
			return false;
		}

		o_Field.InbuiltVarType = EInbuiltVarType::IF_vec4_out_FragColour;
	}
	else if ( a_Input.Name == "FragDepth" )
	{
		// It must be Out
		if ( o_Field.IOType != EIOType::Out )
		{
			// Error, Inbuilt Variable 'FragDepth' must be Out
			return false;
		}

		if ( o_Field.DataType != EDataType::DT_float )
		{
			// Error, Inbuilt Variable 'FragDepth' must be vec4.
			return false;
		}

		o_Field.InbuiltVarType = EInbuiltVarType::IF_float_out_FragDepth;
	}

	o_Field.FieldString = a_Input.Prefix + "_" + a_Input.Name;

	return true;
}

bool ProcessFields( const std::vector< FieldInfo >& a_Input, std::vector< Field >& o_Output )
{
	for ( auto& Input : a_Input )
	{
		Field ProcessedField;

		if ( ProcessField( Input, ProcessedField ) )
		{
			o_Output.push_back( ProcessedField );
		}
		else
		{
			// Error on this line.
			return false;
		}
	}

	return true;
}

//struct Field_
//{
//	const char* Name;
//	uint64_t Offset;
//	uint64_t Size;
//	uint16_t DataType;
//	uint16_t IOType;
//	uint16_t BuiltinVar;
//	uint16_t Slot;
//};
//
//struct LayoutInfo
//{
//	Field_* Fields;
//	size_t FieldCount;
//};

std::string CreateInfoFunction( const std::vector< Field >& a_Input )
{
	std::string Result;
	Result += R"(
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
	size_t Size;
};

)";

	Result += "extern \"C\" __declspec(dllexport) LayoutInfo* info()\n{\n";
	Result += "\tstatic Field Fields[" + std::to_string( a_Input.size() ) + "];\n\n";

	for ( size_t i = 0u; i < a_Input.size(); ++i )
	{
		std::string Index = std::to_string( i );
		Result += "\tFields[" + Index + "].Name = \"" + a_Input[ i ].Name + "\";\n";
		Result += "\tFields[" + Index + "].Offset = offsetof(Layout, " + a_Input[ i ].FieldString + ");\n";
		Result += "\tFields[" + Index + "].Size = sizeof(" + a_Input[ i ].DataTypeString + ");\n";
		Result += "\tFields[" + Index + "].DataType = " + std::to_string( ( size_t )a_Input[ i ].DataType ) + ";\n";
		Result += "\tFields[" + Index + "].IOType = " + std::to_string( ( size_t )a_Input[ i ].IOType ) + ";\n";
		Result += "\tFields[" + Index + "].BuiltinVar = " + std::to_string( ( size_t )a_Input[ i ].InbuiltVarType ) + ";\n";
		Result += "\tFields[" + Index + "].Slot = " + std::to_string( ( int16_t )a_Input[ i ].Slot ) + ";\n\n";
	}

	Result += "\tstatic LayoutInfo Info;\n";
	Result += "\tInfo.Fields = Fields;\n";
	Result += "\tInfo.FieldCount = " + std::to_string( a_Input.size() ) + ";\n";
	Result += "\tInfo.Size = sizeof(Layout);\n";

	Result += "\treturn &Info;\n";
	Result += "}\n";

	return Result;
}

std::string CreateTypedefs()
{
	return R"(
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using mat2x3 = glm::mat2x3;
using mat2x4 = glm::mat2x4;
using mat3x2 = glm::mat3x2;
using mat3x4 = glm::mat3x4;
using mat4x2 = glm::mat4x2;
using mat4x3 = glm::mat4x3;
using dmat2 = glm::dmat2;
using dmat3 = glm::dmat3;
using dmat4 = glm::dmat4;
using dmat2x3 = glm::dmat2x3;
using dmat2x4 = glm::dmat2x4;
using dmat3x2 = glm::dmat3x2;
using dmat3x4 = glm::dmat3x4;
using dmat4x2 = glm::dmat4x2;
using dmat4x3 = glm::dmat4x3;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using dvec2 = glm::dvec2;
using dvec3 = glm::dvec3;
using dvec4 = glm::dvec4;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;
using uvec2 = glm::uvec2;
using uvec3 = glm::uvec3;
using uvec4 = glm::uvec4;
)";
}

bool ProcessSource( const std::string& a_Input, std::string& o_Output )
{
	std::string ExtractedLayout;

	if ( !ExtractLayout( a_Input, ExtractedLayout ) )
	{
		return false;
	}

	std::vector< FieldInfo > ExtractedFields;

	if ( !ExtractFields( ExtractedLayout, ExtractedFields ) )
	{
		return false;
	}

	std::vector< Field > ProcessedFields;

	if ( !ProcessFields( ExtractedFields, ProcessedFields ) )
	{
		return false;
	}

	std::string DeclspecInput = a_Input;
    const std::regex RunDeclaration( R"(\bvoid\s+run\b)" );
    const std::string DeclspecRunDeclaration = R"(extern "C" __declspec(dllexport) void run)";

    DeclspecInput = std::regex_replace( DeclspecInput, RunDeclaration, DeclspecRunDeclaration );

    o_Output = 
		"#include <cstdint>\n"
		"#include <cstddef>\n"
		"#include <glm.hpp>\n\n"
	+ CreateTypedefs() + DeclspecInput + CreateInfoFunction( ProcessedFields );

	return true;
}

#include <glm.inl>

int main( int a_ArgCount, const char** a_Args )
{
	// "--source"
	std::string SourceFile;

	// "--output"
	std::string OutputFile;

	bool Error = false;
	bool SourceFileDoesntExist = false;

	for ( int i = 0; i < a_ArgCount; ++i )
	{
		std::cout << a_Args[ i ] << std::endl;
	}

	for ( int i = 1u; i < a_ArgCount; ++i )
	{
		const std::string Arg = a_Args[ i ];

		if ( Arg.starts_with( "--" ) )
		{
			if ( i + 1 == a_ArgCount )
			{
				Error = true;
				break;
			}

			const std::string Option = Arg.substr( 2u );

			if ( Option == "source" )
			{
				SourceFile = a_Args[ i + 1 ];
				++i;
				continue;
			}

			if ( Option == "output" )
			{
				OutputFile = a_Args[ i + 1 ];
				++i;
				continue;
			}

			Error = true;
			break;
		}
	}

	if ( SourceFile.empty() || !std::filesystem::exists( SourceFile ) )
	{
		Error = true;
		SourceFileDoesntExist = true;
	}

	if ( Error )
	{
		if ( SourceFileDoesntExist )
		{
			std::cerr << "Source file not found.\n";
		}
		else
		{
			std::cerr << "Usage: ShaderCompiler.exe --source <SourceFile> --destination <Destination>\n";
		}

		return 1;
	}

	std::string SourceName = std::filesystem::path{ SourceFile }.stem().string();
	std::string BinaryFile = SourceName + ".dll";

	if ( OutputFile.empty() )
	{
		OutputFile = SourceName + ".cglshader";
	}

	const std::string IntermediateSourceFile = "intermediate_" + SourceFile;

	std::string Premake = std::vformat(
///////////////////////////////////////////////////////////////
"workspace \"Shader\"										\n"
"    configurations {{ \"Debug\", \"Release\" }}			\n"
"    platforms {{ \"x64\", \"x86\" }}						\n"
"															\n"
"project \"Shader\"											\n"
"    kind \"SharedLib\"										\n"
"    language \"C++\"										\n"
"	 cppdialect \"C++20\"									\n"
"    targetdir \"bin\"										\n"
"    objdir \"obj\"											\n"
"	 includedirs {{ \"$(SolutionDir)glm\" }}				\n"
"    files {{ \"{}\" }}										\n"
"															\n"
"    filter \"configurations:Debug\"						\n"
"        defines {{ \"DEBUG\" }}							\n"
"        symbols \"On\"										\n"
"															\n"
"    filter \"configurations:Release\"						\n"
"        defines {{ \"NDEBUG\" }}							\n"
"        optimize \"On\"									\n"
///////////////////////////////////////////////////////////////
		, std::make_format_args( IntermediateSourceFile ) );

	// We need to open the source file, read it, process it, and write out the intermediate source file instead.
	const size_t SourceFileTextSize = std::filesystem::file_size( SourceFile );
	std::string SourceFileText;
	SourceFileText.resize( SourceFileTextSize );

	{
		std::ifstream SourceFileStream( SourceFile, std::ios::binary | std::ios::in );

		if ( !SourceFileStream.is_open() )
		{
			return 1;
		}

		SourceFileStream.read( SourceFileText.data(), SourceFileText.size() );
	}

	// Create premake directory.
	{
		std::filesystem::remove_all( "Temp" );
		std::filesystem::create_directories( "Temp" );

		std::ofstream PremakeFile( "Temp/premake5.lua", std::ios::binary | std::ios::out );

		if ( !PremakeFile.is_open() )
		{
			return 1;
		}

		PremakeFile.write( Premake.c_str(), Premake.size() );
	}

	// Create intermediate source file
	std::string SourceFileTextProcessed;

	if ( !ProcessSource( SourceFileText, SourceFileTextProcessed ) )
	{
		return 1;
	}

	// Write out the intermediate source file.
	{
		std::ofstream IntermediateSourceFileStream( std::vformat( "Temp/{}", std::make_format_args( IntermediateSourceFile ) ), std::ios::binary | std::ios::out );

		if ( !IntermediateSourceFileStream.is_open() )
		{
			return 1;
		}

		IntermediateSourceFileStream.write( SourceFileTextProcessed.data(), SourceFileTextProcessed.size() );
	}

	// Write out the glm library to be included.
	{
		std::ofstream GLMZipStream( "Temp/glm.zip", std::ios::binary | std::ios::out );

		if ( !GLMZipStream.is_open() )
		{
			return 1;
		}

		GLMZipStream.write( ( const char* )GLM, sizeof( GLM ) );
	}

	// Extract the glm folder.
	std::filesystem::create_directories( "Temp/glm/" );
	int ReturnCode = 0;
	ReturnCode = system( "tar -xf Temp/glm.zip -C Temp/glm" ); if ( ReturnCode != 0 ) return 1;

	// Figure out what version of visual studio is available.

	std::string VisualStudioVersion = GetVisualStudioVersion();

	if ( VisualStudioVersion.empty() )
	{
		return 1;
	}

	ReturnCode = system( std::vformat( "premake5 --file=\"Temp\\premake5.lua\" vs2022", std::make_format_args( VisualStudioVersion ) ).c_str() ); if ( ReturnCode != 0 ) return 1;
	ReturnCode = system( std::vformat( "msbuild Temp\\Shader.vcxproj /p:Configuration=Debug /p:Platform=x64", std::make_format_args( SourceName ) ).c_str() ); if ( ReturnCode != 0 ) return 1;
	ReturnCode = std::filesystem::remove_all( BinaryFile );
	ReturnCode = std::filesystem::remove_all( OutputFile );
	ReturnCode = std::filesystem::copy_file( "Temp\\bin\\Shader.dll", BinaryFile ); if ( ReturnCode != 1 ) return 1;
	std::filesystem::path ParentDirectory = std::filesystem::path{ OutputFile }.remove_filename();

	if ( !ParentDirectory.empty() )
	{
		std::filesystem::create_directories( ParentDirectory  );
	}

	std::filesystem::rename( BinaryFile, OutputFile );
	std::filesystem::remove_all( "Temp" );

	return 0;
}