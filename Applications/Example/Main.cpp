#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#undef CreateWindow
#include <ConsoleGL.hpp>

#include <detail/func_matrix.hpp>
#include <gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
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
prsp out vec3 VertexColour;

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
	ConsoleGL::BufferHandle VBO;
	ConsoleGL::CreateBuffers( 1, &VBO );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, VBO );
	ConsoleGL::BufferData(ConsoleGL::EBufferTarget::ArrayBuffer, sizeof( Vertices ), Vertices );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );
;
	ConsoleGL::VertexArrayHandle VAO;
	ConsoleGL::CreateVertexArrays( 1, &VAO );
	ConsoleGL::BindVertexArray( VAO );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, VBO );
	ConsoleGL::VertexAttribPointer( 0u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( glm::vec3 ) * 2u, 0u );
	ConsoleGL::VertexAttribPointer( 1u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( glm::vec3 ) * 2u, sizeof( glm::vec3 ) );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );
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

		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::W ) ) CameraZ -= 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::S ) ) CameraZ += 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::A ) ) CameraX -= 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::D ) ) CameraX += 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::E ) ) CameraY += 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::Q ) ) CameraY -= 0.01f;

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
		ConsoleGL::DrawElements( ConsoleGL::EPrimitiveType::Triangles, 8u, 36u, Indices, ConsoleGL::EDataType::Int32 );
		ConsoleGL::BindVertexArray( nullptr );
		ConsoleGL::SwapWindowBuffer();
		ConsoleGL::UseProgram( nullptr );
	}
}
