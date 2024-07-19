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
	TODO( "Could this be just provided by a main arg instead?" );
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

		std::vector cmdline(cmd.begin(), cmd.end());
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

	const std::string Response = exec("where devenv");

    const std::regex pattern("\\\\[0-9][0-9][0-9][0-9]\\\\");
    std::smatch matches;

    if (!std::regex_search(Response, matches, pattern)) {
        return "";
    }

	std::string VSYear = matches[ 0 ].str().substr( 1u, 4u );

	return "vs" + VSYear;
}

void StripComments( std::string& a_String )
{
    // Replace all matches with an empty string
    a_String = std::regex_replace( a_String, std::regex( R"((//(.|\n)*?$)|(/\*(.|\n)*?\*/))" ), "" );
}

struct ShaderVariableInfo
{
    std::string InterpQualifier;
	std::string StorageQual;
    int32_t Location = -1;
    std::string Type;
    std::string Name;
};

std::vector< ShaderVariableInfo > GetGlobalVariables( const std::string& a_String )
{
    std::smatch Matches;
	std::regex VariablePattern( R"(^\s*(prsp|flat|affn)?\s*(uniform|attrib|in|out)(\(\s*(\d+)\s*\))?\s*(\w+)\s*(\w+)\s*(\[\s*(\d+)\s*\])?\s*;)" );
	std::vector< ShaderVariableInfo > Result;

    std::sregex_iterator Iter( a_String.begin(), a_String.end(), VariablePattern ), End;

    while ( Iter != End )
    {
        ShaderVariableInfo VariableInfo;
        std::smatch Match = *Iter;

        VariableInfo.InterpQualifier = Match[ 1u ];
        VariableInfo.StorageQual = Match[ 2u ];
        VariableInfo.Location = Match[ 4u ].matched ? std::stoi( Match[ 4u ] ) : -1;
        VariableInfo.Type = Match[ 5u ];
        VariableInfo.Name = Match[ 6u ];
        Result.emplace_back( std::move( VariableInfo ) );
        ++Iter;
    }

	return Result;
}

std::string CreateInfoFunction( const std::vector< ShaderVariableInfo >& a_Input )
{
	std::string Result;
	Result += R"(
struct ShaderVariableInfo
{
	const char* InterpQual = nullptr;
	const char* StorageQual = nullptr;
	const char* Type = nullptr;
	const char* Name = nullptr;
	const void* Data = nullptr;
	uint64_t Size = 0u;
	uint64_t Length = 1u;
	uint64_t Location = -1;
};

struct ShaderInfo
{
	ShaderVariableInfo* Variables;
	size_t VariableCount;
};

)";

	Result += "extern \"C\" __declspec(dllexport) ShaderInfo* info()\n{\n";
	Result += "\tstatic ShaderVariableInfo Variables[" + std::to_string( a_Input.size() ) + "] {};\n\n";

	for ( size_t i = 0u; i < a_Input.size(); ++i )
	{
		std::string Index = std::to_string( i );
		Result += std::vformat( "\tVariables[{0}].InterpQual = \"{1}\";\n", std::make_format_args( Index, a_Input[ i ].InterpQualifier ) );
		Result += std::vformat( "\tVariables[{0}].StorageQual = \"{1}\";\n", std::make_format_args( Index, a_Input[ i ].StorageQual ) );
		Result += std::vformat( "\tVariables[{0}].Type = \"{1}\";\n", std::make_format_args( Index, a_Input[ i ].Type ) );
		Result += std::vformat( "\tVariables[{0}].Name = \"{1}\";\n", std::make_format_args( Index, a_Input[ i ].Name ) );
		Result += std::vformat( "\tVariables[{0}].Data = &{1};\n", std::make_format_args( Index, a_Input[ i ].Name ) );
		Result += std::vformat( "\tVariables[{0}].Size = sizeof({1});\n", std::make_format_args( Index, a_Input[ i ].Name ) );
		Result += std::vformat( "\tVariables[{0}].Length = sizeof({1}) / sizeof({2});\n", std::make_format_args( Index, a_Input[ i ].Name, a_Input[ i ].Type ) );
		Result += std::vformat( "\tVariables[{0}].Location = {1};\n", std::make_format_args( Index, std::to_string( a_Input[ i ].Location ) ) );
	}
	
	Result += "\tstatic ShaderInfo Info;\n";
	Result += "\tInfo.Variables = Variables;\n";
	Result += "\tInfo.VariableCount = " + std::to_string( a_Input.size() ) + ";\n";

	Result += "\treturn &Info;\n";
	Result += "}\n";

	return Result;
}

void ProcessSource( const std::string& a_Input, std::string& o_Output )
{
	// Include headers:
	o_Output += R"(
#include <cstdint>
#include <cstddef>
#include <glm.hpp>
)";

	// Typedefs:
	o_Output += R"(
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

	// Keyword defines:
	o_Output += R"(
#define affn
#define flat
#define prsp
#define uniform(x)
#define attrib(x)
#define in
#define out

)";

	std::string Source = a_Input;
	
	// Strip all comments:
	StripComments( Source );

	// Replace run function with dllexport declaration:
	std::string DeclspecInput = a_Input;
    const std::regex RunDeclaration( R"(\bvoid\s+run\b)" );
    const std::string DeclspecRunDeclaration = R"(extern "C" __declspec(dllexport) void run)";
    o_Output += std::regex_replace( Source, RunDeclaration, DeclspecRunDeclaration );

	// Add info function:
	const std::vector VariableInfos = GetGlobalVariables( Source );

	o_Output += CreateInfoFunction( VariableInfos );
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
	ProcessSource( SourceFileText, SourceFileTextProcessed );

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
	ReturnCode = system( "msbuild Temp\\Shader.vcxproj /p:Configuration=Debug /p:Platform=x64" ); if ( ReturnCode != 0 ) return 1;
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