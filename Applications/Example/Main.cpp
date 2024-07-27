#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#undef CreateWindow
#include <ConsoleGL.hpp>

#include <detail/func_matrix.hpp>
#include <gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gtx/quaternion.hpp"
#include "gtx/rotate_vector.hpp"


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

// Uniforms
uniform(0) mat4 P;
uniform(1) mat4 V;
uniform(2) mat4 M;
uniform(3) vec3 L;

// Attributes
attrib(0) vec3 Position;
attrib(1) vec4 Colour;
attrib(2) vec3 Normal;

// Inbuilts
out vec4 VertPos;

// Outs
out vec4 fragColour;
out vec3 fragPos;
out vec3 fragNormal;
out vec3 lightDir;

void run()
{
	fragColour = Colour;
	
	// Transform the vertex position to world space
	vec4 worldPos = M * vec4(Position, 1.0f);
	fragPos = vec3(worldPos.x, worldPos.y, worldPos.z);
	
	// Transform the normal to world space and normalize it
	fragNormal = mat3(glm::transpose(glm::inverse(M))) * Normal;
	
	// Pass the light direction to the fragment shader
	lightDir = glm::normalize(L);

	// Transform the vertex position to clip space
	VertPos = P * V * worldPos;
}
)";

	ConsoleGL::SetShaderSourceFromString( DefaultVertexShader, VertexShaderSource.c_str(), VertexShaderSource.size() );
	ConsoleGL::CompileShader( DefaultVertexShader );
	ConsoleGL::AttachShader( DefaultShaderProgram, DefaultVertexShader );

	// Create fragment shader.
	ConsoleGL::Shader* DefaultFragmentShader = ConsoleGL::CreateShader( ConsoleGL::EShaderType::Fragment );
	const std::string FragmentShaderSource = R"(

out vec4 FragColour;

// Ins
in vec4 fragColour;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 lightDir;

uniform(4) vec3 viewPos;
uniform(5) vec3 lightColour;
uniform(6) vec3 objectColour;

void run()
{
	// Ambient lighting
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColour;

    // Diffuse lighting
    vec3 norm = glm::normalize(fragNormal);
    float diff = glm::max(glm::dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColour;

    // Specular lighting
    float specularStrength = 0.5f;
    vec3 viewDir = glm::normalize(viewPos - fragPos);
    vec3 reflectDir = glm::reflect(-lightDir, norm);
    float spec = glm::pow(glm::max(glm::dot(viewDir, reflectDir), 0.0f), 32.0f);
    vec3 specular = specularStrength * spec * lightColour;

    // Combine all lighting components
    vec4 result = vec4((ambient + diffuse + specular) * objectColour, 1.0f) * fragColour;
    FragColour = glm::clamp(result, vec4(0.0f), vec4(1.0f));
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

	struct VertexData
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec3 Normal;
	};

	size_t s = sizeof( VertexData );

	const VertexData CubeVertices[] = {
		{ /*Vertex*/ { -0.5f, +0.5f, +0.5f }, /*Colour*/ { 0.0f, 1.0f, 1.0f, 1.0f }, /*Normals*/ { -0.376f, +0.376f, +0.376f } }, // Top left front
		{ /*Vertex*/ { +0.5f, +0.5f, +0.5f }, /*Colour*/ { 1.0f, 1.0f, 1.0f, 1.0f }, /*Normals*/ { +0.376f, +0.376f, +0.376f } }, // Top right front
		{ /*Vertex*/ { +0.5f, +0.5f, -0.5f }, /*Colour*/ { 1.0f, 1.0f, 0.0f, 1.0f }, /*Normals*/ { +0.376f, +0.376f, -0.376f } }, // Top right back
		{ /*Vertex*/ { -0.5f, +0.5f, -0.5f }, /*Colour*/ { 0.0f, 1.0f, 0.0f, 1.0f }, /*Normals*/ { -0.376f, +0.376f, -0.376f } }, // Top left back
		{ /*Vertex*/ { -0.5f, -0.5f, +0.5f }, /*Colour*/ { 0.0f, 0.0f, 1.0f, 1.0f }, /*Normals*/ { -0.376f, -0.376f, +0.376f } }, // Bot left front
		{ /*Vertex*/ { +0.5f, -0.5f, +0.5f }, /*Colour*/ { 1.0f, 0.0f, 1.0f, 1.0f }, /*Normals*/ { +0.376f, -0.376f, +0.376f } }, // Bot right front
		{ /*Vertex*/ { +0.5f, -0.5f, -0.5f }, /*Colour*/ { 1.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { +0.376f, -0.376f, -0.376f } }, // Bot right back
		{ /*Vertex*/ { -0.5f, -0.5f, -0.5f }, /*Colour*/ { 0.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { -0.376f, -0.376f, -0.376f } }, // Bot left back
	};

	const VertexData FloorVertices[] = {
		{ /*Vertex*/ { -1.5f, -1.5f, +1.5f }, /*Colour*/ { 1.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { 0.0f, 1.0f, 0.0f } }, // Floor left front
		{ /*Vertex*/ { +1.5f, -1.5f, +1.5f }, /*Colour*/ { 1.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { 0.0f, 1.0f, 0.0f } }, // Floor right front
		{ /*Vertex*/ { +1.5f, -1.5f, -1.5f }, /*Colour*/ { 1.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { 0.0f, 1.0f, 0.0f } }, // Floor right back
		{ /*Vertex*/ { -1.5f, -1.5f, -1.5f }, /*Colour*/ { 1.0f, 0.0f, 0.0f, 1.0f }, /*Normals*/ { 0.0f, 1.0f, 0.0f } }, // Floor left back
	};

	const int32_t CubeIndices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 1, 4, 1, 0,
		7, 6, 5, 7, 5, 4,
		3, 2, 6, 3, 6, 7,
		4, 0, 3, 4, 3, 7,
		1, 5, 6, 1, 6, 2,
	};

	const int32_t FloorIndices[] = {
		0, 1, 2, 0, 2, 3,
		0, 2, 1, 0, 3, 2
	};

	// Create a vbo.
	ConsoleGL::BufferHandle VBO[ 2u ];
	ConsoleGL::CreateBuffers( 2, VBO );

	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, VBO[ 0u ] );
	ConsoleGL::BufferData(ConsoleGL::EBufferTarget::ArrayBuffer, sizeof( CubeVertices ), CubeVertices );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );

	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, VBO[ 1u ] );
	ConsoleGL::BufferData(ConsoleGL::EBufferTarget::ArrayBuffer, sizeof( FloorVertices ), FloorVertices );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );
;
	ConsoleGL::VertexArrayHandle VAO[ 2u ];
	ConsoleGL::CreateVertexArrays( 2, VAO );

	// Cube VAO
	ConsoleGL::BindVertexArray( VAO[ 0u ] );
	ConsoleGL::BindBuffer( ConsoleGL::EBufferTarget::ArrayBuffer, VBO[ 0u ] );
	ConsoleGL::VertexAttribPointer( 0u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( VertexData ), offsetof( VertexData, Position ) );
	ConsoleGL::VertexAttribPointer( 1u, 1u, ConsoleGL::EDataType::Vec4, false, sizeof( VertexData ), offsetof( VertexData, Colour ) );
	ConsoleGL::VertexAttribPointer( 2u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( VertexData ), offsetof( VertexData, Normal ) );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );
	ConsoleGL::EnableVertexAttribArray( 0u );
	ConsoleGL::EnableVertexAttribArray( 1u );
	ConsoleGL::EnableVertexAttribArray( 2u );
	ConsoleGL::BindVertexArray( nullptr );

	// Floor VAO
	ConsoleGL::BindVertexArray( VAO[ 1u ] );
	ConsoleGL::BindBuffer( ConsoleGL::EBufferTarget::ArrayBuffer, VBO[ 1u ] );
	ConsoleGL::VertexAttribPointer( 0u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( VertexData ), offsetof( VertexData, Position ) );
	ConsoleGL::VertexAttribPointer( 1u, 1u, ConsoleGL::EDataType::Vec4, false, sizeof( VertexData ), offsetof( VertexData, Colour ) );
	ConsoleGL::VertexAttribPointer( 2u, 1u, ConsoleGL::EDataType::Vec3, false, sizeof( VertexData ), offsetof( VertexData, Normal ) );
	ConsoleGL::BindBuffer(ConsoleGL::EBufferTarget::ArrayBuffer, nullptr );
	ConsoleGL::EnableVertexAttribArray( 0u );
	ConsoleGL::EnableVertexAttribArray( 1u );
	ConsoleGL::EnableVertexAttribArray( 2u );
	ConsoleGL::BindVertexArray( nullptr );

	ConsoleGL::SetClearColour( { 0.0f, 0.0f, 0.0f, 1.0f } );
	ConsoleGL::SetClearDepth( 10000.0f );
	ConsoleGL::Enable( ConsoleGL::ERenderSetting::DepthTest );
	ConsoleGL::Enable( ConsoleGL::ERenderSetting::AlphaBlend );
	ConsoleGL::Enable( ConsoleGL::ERenderSetting::Clipping );
	ConsoleGL::SetDepthTest( ConsoleGL::EDepthTest::Lesser );

	glm::vec3 CamPos{ 0.0f, 0.0f, 10.0f };
	//float CamRotX = 0.0f, CamRotY = 0.0f, CamRotZ = 0.0f;
	float ObjRotX = 0.0f, ObjRotY = 0.0f, ObjRotZ = 0.0f;
	
	float yaw = 180.0f; // Initialized to -90 degrees to look along the negative Z-axis
	float pitch = 0.0f;
	float sensitivity =200.1f; // Mouse sensitivity

	// Use the program now for current context.
	while ( true )
	{
		ConsoleGL::PollEvents();
		ConsoleGL::Clear();
		
		ObjRotZ += 0.005;
		ObjRotY += 0.01;
		ObjRotX += 0.001;

		float MouseX, MouseY;
		ConsoleGL::GetMouseDelta( MouseX, MouseY );

		if ( ConsoleGL::IsMouseDown( ConsoleGL::EMouseButton::RightMouse ) )
		{
			yaw += MouseX * sensitivity;
			pitch -= MouseY * sensitivity;
		}
		
		// Constrain the pitch angle to prevent screen flipping
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		/*glm::vec3 Forward;
		Forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		Forward.y = sin(glm::radians(pitch));
		Forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));*/

		glm::mat4 CamRot = glm::rotate(glm::radians(-yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(-pitch), glm::vec3(1.0f, 0.0f, 0.0f));


		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::W ) ) CamPos += glm::vec3(CamRot[ 2u ]) * 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::S ) ) CamPos -= glm::vec3(CamRot[ 2u ]) * 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::A ) ) CamPos += glm::vec3(CamRot[ 0u ]) * 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::D ) ) CamPos -= glm::vec3(CamRot[ 0u ]) * 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::E ) ) CamPos += glm::vec3(CamRot[ 1u ]) * 0.01f;
		if ( ConsoleGL::IsKeyDown( ConsoleGL::EKeyboardKey::Q ) ) CamPos -= glm::vec3(CamRot[ 1u ]) * 0.01f;
		
		float AspectRatio = ( float )Width / Height;
		float FOV = glm::radians( 75.0f );
		float ZNear = 3.0f;
		float ZFar = 1000.0f;

		glm::vec3 ObjectPos = { 0.0f, 0.0f, 0.0f };
		glm::vec3 ObjectRot = { ObjRotX, ObjRotY, ObjRotZ };
		glm::vec3 ObjectSca = { 1.0f, 1.0f, 1.0f };

		glm::vec3 FloorPos = { 0.0f, 2.0f, 0.0f };
		glm::vec3 FloorRot = { ObjRotX, ObjRotY, ObjRotZ };
		glm::vec3 FloorSca = { 1.0f, 1.0f, 1.0f };

		glm::mat4 M0 = glm::translate( glm::mat4( 1.0f ), ObjectPos ) * glm::mat4_cast(glm::quat(ObjectRot));
		glm::mat4 M1 = glm::translate( glm::mat4( 1.0f ), FloorPos );
		glm::mat4 V = glm::lookAt( CamPos, CamPos + glm::vec3(CamRot[2]), glm::vec3{0.0f, 1.0f, 0.0f});
		glm::mat4 P = glm::perspective( FOV, AspectRatio, ZNear, ZFar );
		
		int32_t P_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "P" );
		int32_t V_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "V" );
		int32_t M_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "M" );
		int32_t L_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "L" );
		int32_t viewPos_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "viewPos" );
		int32_t lightColour_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "lightColour" );
		int32_t objectColour_Loc = ConsoleGL::GetUniformLocation( DefaultShaderProgram, "objectColour" );

		ConsoleGL::UseProgram( DefaultShaderProgram );

		// Light direction.
		ConsoleGL::Uniform3f( L_Loc, 0.376, 0.376, 0.376 );

		ConsoleGL::UniformMatrix4fv( V_Loc, 1, false, &V[ 0 ][ 0 ] );
		ConsoleGL::UniformMatrix4fv( P_Loc, 1, false, &P[ 0 ][ 0 ] );

		ConsoleGL::Uniform3f( viewPos_Loc, CamPos.x, CamPos.y, CamPos.z );
		ConsoleGL::Uniform3f( lightColour_Loc, 1.0f, 1.0f, 1.0f );
		ConsoleGL::Uniform3f( objectColour_Loc, 1.0f, 1.0f, 1.0f );

		// Bind the cube VAO and draw
		/*ConsoleGL::UniformMatrix4fv( M_Loc, 1, false, &M0[ 0 ][ 0 ] );
		ConsoleGL::BindVertexArray( VAO[ 0u ] );
		ConsoleGL::DrawElements( ConsoleGL::EPrimitiveType::Triangles, sizeof( CubeIndices ) / sizeof( int32_t ), CubeIndices, ConsoleGL::EDataType::Int32);
		ConsoleGL::BindVertexArray( nullptr );*/

		// Bind the floor VAO and draw
		ConsoleGL::UniformMatrix4fv( M_Loc, 1, false, &M1[ 0 ][ 0 ] );
		ConsoleGL::BindVertexArray( VAO[ 1u ] );
		ConsoleGL::DrawElements( ConsoleGL::EPrimitiveType::Triangles, sizeof( FloorIndices ) / sizeof( int32_t ), FloorIndices, ConsoleGL::EDataType::Int32);
		ConsoleGL::BindVertexArray( nullptr );

		ConsoleGL::SwapWindowBuffer();
		ConsoleGL::UseProgram( nullptr );
	}
}















































//#include <array>
//
//template < uint8_t _WidthAndHeight >
//void Raster()
//{
//	constexpr uint32_t W = 1u << ( ( _WidthAndHeight & 0b11110000 ) >> 4u );
//	constexpr uint32_t H = 1u << ( ( _WidthAndHeight & 0b00001111 ) >> 0u );
//
//	Raster( std::make_integer_sequence< uint8_t, W >{}, std::make_integer_sequence< uint8_t, H >{} );
//}
//
//template < uint8_t... _WIdxs, uint8_t... _HIdxs >
//void Raster( const std::integer_sequence< uint8_t, _WIdxs... >, const std::integer_sequence< uint8_t, _HIdxs... > )
//{
//	static auto For = []( uint8_t x, uint8_t y )
//	{
//		// Do something here for given x,y.
//	};
//
//	auto ForEachX = []( uint8_t y ) { ( For( _WIdxs, y ), ... ); };
//
//	( ForEachX( _HIdxs ), ... );
//}
//
//using RasterFn = void(*)();
//
//template < uint8_t... _WidthsAndHeights >
//auto GetRasterFns( const std::integer_sequence< uint8_t, _WidthsAndHeights... > )
//{
//	std::array< RasterFn, 256u > Fns{ Raster< _WidthsAndHeights >... };
//	return Fns;
//}
//
//uint32_t RoundToNextPowerOf2( uint32_t Val ) { /*... Do something here to round to next power of 2.*/ return 0u; }
//uint32_t LogBase2( uint32_t Val ) { /*Find log2 of number, ie. How many times I need to bitshift left.*/ return 0u; }
//
//auto GetRasterFn( const uint32_t Width, const uint32_t Height )
//{
//	const uint32_t WidthBitShift = LogBase2( RoundToNextPowerOf2( Width ) );
//	const uint32_t HeightBitShift = LogBase2( RoundToNextPowerOf2( Height ) );
//	const uint8_t WidthAndHeight = WidthBitShift << 4u | HeightBitShift;
//
//	return GetRasterFns( std::make_integer_sequence< uint8_t, 256u >{} )[ WidthAndHeight ];
//}
//
//void Raster( const uint32_t Width, const uint32_t Height )
//{
//	GetRasterFn( Width, Height )();
//}