#include <ConsoleGL.hpp>
#include <Window.hpp>

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

//void ConsoleGL::SetWindowBuffer( Window* a_Window, const Pixel a_Pixel )
//{
//	a_Window->SetBuffer( a_Pixel );
//}
//
//void ConsoleGL::SetWindowPixelByIndex( Window* a_Window, const uint32_t a_Index, const Pixel a_Pixel )
//{
//	a_Window->SetPixel( a_Index, a_Pixel );
//}
//
//void ConsoleGL::SetWindowPixelByPosition( Window* a_Window, const uint32_t a_X, const uint32_t a_Y, const Pixel a_Pixel )
//{
//	a_Window->SetPixel( a_X, a_Y, a_Pixel );
//}
//
//void ConsoleGL::SetWindowPixelsByIndex( Window* a_Window, const uint32_t a_Index, const uint32_t a_Count, const Pixel a_Pixel )
//{
//	a_Window->SetPixels( a_Index, a_Count, a_Pixel );
//}
//
//void ConsoleGL::SetWindowPixelsByPosition( Window* a_Window, const uint32_t a_X, const uint32_t a_Y, const uint32_t a_Count, const Pixel a_Pixel )
//{
//	a_Window->SetPixels( a_X, a_Y, a_Count, a_Pixel );
//}

#pragma endregion

#pragma region Pixel map functions

#if IS_CONSOLEGL

MESSAGE("PixelMap.inl found.");
#include <PixelMap.inl>
const ConsoleGL::Pixel* ConsoleGL::GetPixelMap()
{
	return ( const Pixel* )PixelMap;
}

const ConsoleGL::Pixel* ConsoleGL::MapColourToPixel( const Colour a_Colour )
{
	const uint8_t R = a_Colour.r >> PIXEL_MAP_MIP_LEVEL;
	const uint8_t G = a_Colour.g >> PIXEL_MAP_MIP_LEVEL;
	const uint8_t B = a_Colour.b >> PIXEL_MAP_MIP_LEVEL;

	return ( const Pixel* )PixelMap + ( R * PIXEL_MAP_SIZE * PIXEL_MAP_SIZE ) + ( G * PIXEL_MAP_SIZE ) + B;
}

size_t ConsoleGL::GetPixelMapSize()
{
	return PIXEL_MAP_VOLUME;
}

#else

const ConsoleGL::Pixel* ConsoleGL::GetPixelMap()
{
	static const Pixel Empty;
	return &Empty;
}

const ConsoleGL::Pixel* ConsoleGL::MapColourToPixel( const Colour a_Colour )
{
	return GetPixelMap();
}

size_t ConsoleGL::GetPixelMapSize()
{
	return 1u;
}

#endif

#pragma endregion

#pragma region Context functions

ConsoleGL::Context* ConsoleGL::CreateContext()
{
	return nullptr;
}

void ConsoleGL::DestroyContext( Context* a_Context )
{
	
}

void ConsoleGL::SetActiveContext( Context* a_Context )
{
	
}

ConsoleGL::Context* ConsoleGL::GetActiveContext()
{
	return nullptr;
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

void ConsoleGL::SetPixelByPosition( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, Pixel a_Pixel )
{
	a_Buffer->SetPixel( a_X, a_Y, a_Pixel );
}

void ConsoleGL::SetPixels( PixelBuffer* a_Buffer, uint32_t a_Index, uint32_t a_Count, Pixel a_Pixel )
{
	a_Buffer->SetPixels( a_Index, a_Count, a_Pixel );
}

void ConsoleGL::SetPixelsByPosition( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Count, Pixel a_Pixel )
{
	a_Buffer->SetPixels( a_X, a_Y, a_Count, a_Pixel );
}

void ConsoleGL::SetBuffer( PixelBuffer* a_Buffer, Pixel a_Pixel )
{
	a_Buffer->SetBuffer( a_Pixel );
}

void ConsoleGL::SetBufferFn( PixelBuffer* a_Buffer, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->SetBuffer( a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawLine( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_XEnd, uint32_t a_YBegin, uint32_t a_YEnd, Pixel a_Pixel )
{
	a_Buffer->DrawLine( a_XBegin, a_XEnd, a_YBegin, a_YEnd, a_Pixel );
}

void ConsoleGL::DrawLineFn( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_XEnd, uint32_t a_YBegin, uint32_t a_YEnd, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawLine( a_XBegin, a_XEnd, a_YBegin, a_YEnd, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawHorizontalLine( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_YBegin, uint32_t a_Length, Pixel a_Pixel )
{
	a_Buffer->DrawHorizontalLine( a_XBegin, a_YBegin, a_Length, a_Pixel );
}

void ConsoleGL::DrawHorizontalLineFn( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_YBegin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawHorizontalLine( a_XBegin, a_YBegin, a_Length, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawVerticalLine( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_YBegin, uint32_t a_Length, Pixel a_Pixel )
{
	a_Buffer->DrawVerticalLine( a_XBegin, a_YBegin, a_Length, a_Pixel );
}

void ConsoleGL::DrawVerticalLineFn( PixelBuffer* a_Buffer, uint32_t a_XBegin, uint32_t a_YBegin, uint32_t a_Length, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawVerticalLine( a_XBegin, a_YBegin, a_Length, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawTriangle( PixelBuffer* a_Buffer, uint32_t a_X0, uint32_t a_X1, uint32_t a_X2, uint32_t a_Y0, uint32_t a_Y1, uint32_t a_Y2, Pixel a_Pixel )
{
	return a_Buffer->DrawTriangle( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, a_Pixel );
}

void ConsoleGL::DrawTriangleFn( PixelBuffer* a_Buffer, uint32_t a_X0, uint32_t a_X1, uint32_t a_X2, uint32_t a_Y0, uint32_t a_Y1, uint32_t a_Y2, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	return a_Buffer->DrawTriangle( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawTriangleFilled( PixelBuffer* a_Buffer, uint32_t a_X0, uint32_t a_X1, uint32_t a_X2, uint32_t a_Y0, uint32_t a_Y1, uint32_t a_Y2, Pixel a_Pixel )
{
	return a_Buffer->DrawTriangleFilled( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, a_Pixel );
}

void ConsoleGL::DrawTriangleFilledFn( PixelBuffer* a_Buffer, uint32_t a_X0, uint32_t a_X1, uint32_t a_X2, uint32_t a_Y0, uint32_t a_Y1, uint32_t a_Y2, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	return a_Buffer->DrawTriangleFilled( a_X0, a_X1, a_X2, a_Y0, a_Y1, a_Y2, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRect( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel )
{
	a_Buffer->DrawRect( a_X, a_Y, a_Width, a_Height, a_Pixel );
}

void ConsoleGL::DrawRectFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRect( a_X, a_Y, a_Width, a_Height, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectRotated( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, float a_Radians, Pixel a_Pixel )
{
	a_Buffer->DrawRect( a_X, a_Y, a_Width, a_Height, a_Radians, a_Pixel );
}

void ConsoleGL::DrawRectRotatedFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRect( a_X, a_Y, a_Width, a_Height, a_Radians, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectFilled( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, Pixel a_Pixel )
{
	a_Buffer->DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_Pixel );
}

void ConsoleGL::DrawRectFilledFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawRectFilledRotated( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, float a_Radians, Pixel a_Pixel )
{
	a_Buffer->DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_Radians, a_Pixel );
}

void ConsoleGL::DrawRectFilledRotatedFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, float a_Radians, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawRectFilled( a_X, a_Y, a_Width, a_Height, a_Radians, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawCircle( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawCircle( a_X, a_Y, a_Radius, a_Pixel );
}

void ConsoleGL::DrawCircleFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawCircle( a_X, a_Y, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawCircleFilled( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Radius, Pixel a_Pixel )
{
	a_Buffer->DrawCircleFilled( a_X, a_Y, a_Radius, a_Pixel );
}

void ConsoleGL::DrawCircleFilledFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Radius, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawCircleFilled( a_X, a_Y, a_Radius, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawEllipse( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_RadiusMinor, uint32_t a_RadiusMajor, Pixel a_Pixel )
{
	a_Buffer->DrawEllipse( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, a_Pixel );
}

void ConsoleGL::DrawEllipseFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_RadiusMinor, uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawEllipse( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, a_FragmentFn, a_FragmentFnPayload );
}

void ConsoleGL::DrawEllipseFilled( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_RadiusMinor, uint32_t a_RadiusMajor, Pixel a_Pixel )
{
	a_Buffer->DrawEllipseFilled( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, a_Pixel );
}

void ConsoleGL::DrawEllipseFilledFn( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_RadiusMinor, uint32_t a_RadiusMajor, FragmentFn a_FragmentFn, void* a_FragmentFnPayload )
{
	a_Buffer->DrawEllipseFilled( a_X, a_Y, a_RadiusMinor, a_RadiusMajor, a_FragmentFn, a_FragmentFnPayload );
}

#pragma endregion





void ConsoleGL::DrawTestImage( PixelBuffer* a_Buffer, uint32_t a_X, uint32_t a_Y, uint32_t a_Width, uint32_t a_Height, float a_Radians, uint32_t a_SourceWidth, uint32_t a_SourceHeight, const Colour* a_Source )
{
	
}

















































































////#include "Rendering.hpp"
//#include "ConsoleWindow.hpp"
//#include "ConsoleGL.hpp"
//#include "Shader.hpp"
//
//void ConsoleGL::Init()
//{
//	s_DepthBuffer.Init( ConsoleWindow::GetCurrentContext()->GetSize() );
//}
//
//void ConsoleGL::GenBuffers( uint32_t a_Count, BufferHandle* a_Handles )
//{
//	while ( a_Count-- > 0 ) a_Handles[ a_Count ] = s_BufferRegistry.Create();
//}
//
//void ConsoleGL::BindBuffer( BufferTarget a_BufferTarget, BufferHandle a_Handle )
//{
//	s_BufferTargets[ ( uint32_t )a_BufferTarget ] = a_Handle;
//}
//
//void ConsoleGL::DeleteBuffers( uint32_t a_Count, BufferHandle* a_Handles )
//{
//	while ( a_Count-- > 0 )
//	{
//		// Unbind from buffer target if exists.
//		for ( size_t i = 0; i < 14; ++i )
//		{
//			if ( s_BufferTargets[ i ] == a_Handles[ a_Count ] )
//			{
//				s_BufferTargets[ i ] = 0;
//				break;
//			}
//		}
//
//		// Destroy buffer.
//		s_BufferRegistry.Destroy( a_Handles[ a_Count ] );
//	}
//}
//
//bool ConsoleGL::IsBuffer( BufferHandle a_Handle )
//{
//	return s_BufferRegistry.Valid( a_Handle );
//}
//
//void ConsoleGL::UseProgram( ShaderProgramHandle a_ShaderProgramHandle )
//{
//	s_ActiveShaderProgram = a_ShaderProgramHandle;
//}
//
//void ConsoleGL::Clear( uint8_t a_Flags )
//{
//	if ( a_Flags & static_cast< uint8_t >( BufferFlag::COLOUR_BUFFER_BIT ) )
//	{
//		ConsoleWindow::GetCurrentContext()->GetScreenBuffer().SetBuffer( s_ClearColour );
//	}
//
//	if ( a_Flags & static_cast< uint8_t >( BufferFlag::DEPTH_BUFFER_BIT ) )
//	{
//		s_DepthBuffer.Reset( s_ClearDepth );
//	}
//
//	if ( a_Flags & static_cast< uint8_t >( BufferFlag::ACCUM_BUFFER_BIT ) )
//	{
//
//	}
//
//	if ( a_Flags & static_cast< uint8_t >( BufferFlag::STENCIL_BUFFER_BIT ) )
//	{
//
//	}
//}
//
//void ConsoleGL::ClearColour( float a_R, float a_G, float a_B, float a_A )
//{
//	s_ClearColour = PixelColourMap::Get().ConvertColour( {
//		static_cast< unsigned char >( 255u * a_R ),
//		static_cast< unsigned char >( 255u * a_G ),
//		static_cast< unsigned char >( 255u * a_B ),
//		static_cast< unsigned char >( 255u * a_A ) } );
//}
//
//void ConsoleGL::ClearDepth( float a_ClearDepth )
//{
//	s_ClearDepth = a_ClearDepth;
//}
//
//void ConsoleGL::DrawArrays( RenderMode a_Mode, uint32_t a_Begin, uint32_t a_Count )
//{
//	s_AttributeRegistry.UnsetIndices();
//	UpdateDrawProcessor();
//
//	switch ( a_Mode )
//	{
//		case RenderMode::POINT:
//			break;
//		case RenderMode::LINE:
//			break;
//		case RenderMode::TRIANGLE:
//		{
//			s_DrawProcessorFunc( a_Begin, a_Count );
//			break;
//		}
//		default:
//			break;
//	}
//}
//
//void ConsoleGL::BufferData( BufferTarget a_BufferTarget, size_t a_Size, const void* a_Data, DataUsage a_DataUsage )
//{
//	Buffer& TargetBuffer = s_BufferRegistry[ s_BufferTargets[ ( uint32_t )a_BufferTarget ] ];
//	//TargetBuffer.resize( a_Size );
//	
//	if ( a_Data )
//	{
//		//memcpy( TargetBuffer.data(), a_Data, a_Size );
//		TargetBuffer = static_cast< const uint8_t* >( a_Data );
//	}
//}
//
//void ConsoleGL::NamedBufferData( BufferHandle a_Handle, size_t a_Size, const void* a_Data, DataUsage a_DataUsage )
//{
//	Buffer& TargetBuffer = s_BufferRegistry[ a_Handle ];
//	//TargetBuffer.resize( a_Size );
//
//	if ( a_Data )
//	{
//		//memcpy( TargetBuffer.data(), a_Data, a_Size );
//		TargetBuffer = static_cast< const uint8_t* >( a_Data );
//	}
//}
//
//void ConsoleGL::GenVertexArrays( uint32_t a_Count, ArrayHandle* a_Handles )
//{
//	while ( a_Count-- > 0 ) a_Handles[ a_Count ] = s_ArrayRegistry.Create();
//}
//
//void ConsoleGL::BindVertexArray( ArrayHandle a_Handle )
//{
//	if ( !a_Handle )
//	{
//		s_ActiveArray = 0;
//		return;
//	}
//
//	s_ActiveArray = a_Handle;
//	auto& ActiveArray = s_ArrayRegistry[ a_Handle ];
//	
//	for ( uint8_t i = 0; i < 8; ++i )
//	{
//		if ( ActiveArray[ i ].Enabled )
//		{
//			s_AttributeRegistry[ i ] = ActiveArray[ i ];
//		}
//	}
//}
//
//void ConsoleGL::DeleteVertexArrays( uint32_t a_Count, ArrayHandle* a_Handles )
//{
//	while ( a_Count-- > 0 )
//	{
//		// Unbind bound vertex array if exists.
//		if ( s_ActiveArray == a_Handles[ a_Count ] )
//		{
//			s_ActiveArray = 0;
//		}
//
//		// Destroy buffer.
//		s_ArrayRegistry.Destroy( a_Handles[ a_Count ] );
//	}
//}
//
//void ConsoleGL::EnableVertexAttribArray( uint32_t a_Position )
//{
//	s_ArrayRegistry[ s_ActiveArray ][ a_Position ].Enabled = true;
//}
//
//void ConsoleGL::DisableVertexAttribArray( uint32_t a_Position )
//{
//	s_ArrayRegistry[ s_ActiveArray ][ a_Position ].Enabled = false;
//}
//
//void ConsoleGL::VertexAttribPointer( uint32_t a_Index, uint32_t a_Size, DataType a_DataType, bool a_Normalized, size_t a_Stride, void* a_Offset )
//{
//	auto& Attributes = s_ArrayRegistry[ s_ActiveArray ][ a_Index ];
//	Attributes.Buffer = s_BufferTargets[ ( uint32_t )BufferTarget::ARRAY_BUFFER ];
//	Attributes.Size = a_Size - 1;
//	Attributes.Normalized = a_Normalized;
//	Attributes.Type = ( uint32_t )a_DataType;
//	Attributes.Stride = a_Stride;
//	Attributes.Enabled = false;
//	Attributes.Offset = ( uint32_t )a_Offset;
//}
//
//void ConsoleGL::DrawElements( RenderMode a_Mode, uint32_t a_Count, DataType a_DataType, const void* a_Indices )
//{
//	const void* Indices = nullptr;
//	auto Handle = s_BufferTargets[ ( uint32_t )BufferTarget::ELEMENT_ARRAY_BUFFER ];
//	Indices = s_BufferRegistry.Valid( Handle ) ? ( s_BufferRegistry[ Handle ] + ( uint32_t )a_Indices ) : a_Indices;
//
//	switch ( a_DataType )
//	{
//		case DataType::UNSIGNED_BYTE:  s_AttributeRegistry.SetIndices( reinterpret_cast< const uint8_t*  >( Indices ) ); break;
//		case DataType::UNSIGNED_SHORT: s_AttributeRegistry.SetIndices( reinterpret_cast< const uint16_t* >( Indices ) ); break;
//		case DataType::UNSIGNED_INT:   s_AttributeRegistry.SetIndices( reinterpret_cast< const uint32_t* >( Indices ) ); break;
//		default: break;
//	}
//
//	UpdateDrawProcessor();
//
//	s_DrawProcessorFunc( 0, a_Count );
//}
//
//void ConsoleGL::Enable( RenderSetting a_RenderSetting )
//{
//	switch ( a_RenderSetting )
//	{
//		case RenderSetting::DEPTH_TEST:
//		{
//			s_RenderState.DepthTest = true;
//			break;
//		}
//		case RenderSetting::CULL_FACE:
//		{
//			s_RenderState.CullFace = true;
//			break;
//		}
//		default:
//			break;
//	}
//}
//
//void ConsoleGL::Disable( RenderSetting a_RenderSetting )
//{
//	switch ( a_RenderSetting )
//	{
//		case RenderSetting::DEPTH_TEST:
//		{
//			s_RenderState.DepthTest = false;
//			break;
//		}
//		case RenderSetting::CULL_FACE:
//		{
//			s_RenderState.CullFace = false;
//			break;
//		}
//		default:
//			break;
//	}
//}
//
//void ConsoleGL::CullFace( CullFaceMode a_CullFaceMode )
//{
//	switch ( a_CullFaceMode )
//	{
//		case CullFaceMode::FRONT:
//		{
//			s_RenderState.FrontCull = true;
//			s_RenderState.BackCull = false;
//			break;
//		}
//		case CullFaceMode::BACK:
//		{
//			s_RenderState.FrontCull = false;
//			s_RenderState.BackCull = true;
//			break;
//		}
//		case CullFaceMode::FRONT_AND_BACK:
//		{
//			s_RenderState.FrontCull = true;
//			s_RenderState.BackCull = true;
//			break;
//		}
//		default:
//			break;
//	}
//}
//
//void ConsoleGL::DepthFunc( TextureSetting a_TextureSetting )
//{
//	switch ( a_TextureSetting )
//	{
//		case TextureSetting::LEQUAL:    s_DepthCompareFunc = DepthCompare_LEQUAL;    break;
//		case TextureSetting::GEQUAL:    s_DepthCompareFunc = DepthCompare_GEQUAL;    break;
//		case TextureSetting::LESS:      s_DepthCompareFunc = DepthCompare_LESS;      break;
//		case TextureSetting::GREATER:   s_DepthCompareFunc = DepthCompare_GREATER;   break;
//		case TextureSetting::EQUAL:     s_DepthCompareFunc = DepthCompare_EQUAL;     break;
//		case TextureSetting::NOT_EQUAL: s_DepthCompareFunc = DepthCompare_NOT_EQUAL; break;
//		case TextureSetting::ALWAYS:    s_DepthCompareFunc = DepthCompare_ALWAYS;    break;
//		case TextureSetting::NEVER:     s_DepthCompareFunc = DepthCompare_NEVER;     break;
//		default: break;
//	}
//}
//
//void ConsoleGL::GetBooleanv( RenderSetting a_RenderSetting, bool* a_Value )
//{
//	switch ( a_RenderSetting )
//	{
//		case RenderSetting::DEPTH_TEST: *a_Value = s_RenderState.DepthTest;
//		case RenderSetting::CULL_FACE: *a_Value = s_RenderState.CullFace;
//	}
//}
//
//int32_t ConsoleGL::GetUniformLocation( ShaderProgramHandle a_ShaderProgramHandle, const char* a_Name )
//{
//	auto& ShaderProgram = s_ShaderProgramRegistry[ a_ShaderProgramHandle ];
//	auto UniformName = CRC32_RT( a_Name );
//	auto LocationEntry = ShaderProgram.m_UniformLocations.find( UniformName );
//	return LocationEntry == ShaderProgram.m_UniformLocations.end() ? -1 : LocationEntry->second;
//}
//
//void ConsoleGL::Uniform1f( int32_t a_Location, float a_V0 )
//{
//	*reinterpret_cast< float* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0 };
//}
//
//void ConsoleGL::Uniform2f( int32_t a_Location, float a_V0, float a_V1 )
//{
//	*reinterpret_cast< Vector2* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1 };
//}
//
//void ConsoleGL::Uniform3f( int32_t a_Location, float a_V0, float a_V1, float a_V2 )
//{
//	*reinterpret_cast< Vector3* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2 };
//}
//
//void ConsoleGL::Uniform4f( int32_t a_Location, float a_V0, float a_V1, float a_V2, float a_V3 )
//{
//	*reinterpret_cast< Vector4* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2, a_V3 };
//}
//
//void ConsoleGL::Uniform1i( int32_t a_Location, int32_t a_V0 )
//{
//	*reinterpret_cast< int32_t* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0 };
//}
//
//void ConsoleGL::Uniform2i( int32_t a_Location, int32_t a_V0, int32_t a_V1 )
//{
//	*reinterpret_cast< Vector2Int* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1 };
//}
//
//void ConsoleGL::Uniform3i( int32_t a_Location, int32_t a_V0, int32_t a_V1, int32_t a_V2 )
//{
//	*reinterpret_cast< Vector3Int* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2 };
//}
//
//void ConsoleGL::Uniform4i( int32_t a_Location, int32_t a_V0, int32_t a_V1, int32_t a_V2, int32_t a_V3 )
//{
//	*reinterpret_cast< Vector4Int* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2, a_V3 };
//}
//
//void ConsoleGL::Uniform1ui( int32_t a_Location, uint32_t a_V0 )
//{
//	*reinterpret_cast< uint32_t* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0 };
//}
//
//void ConsoleGL::Uniform2ui( int32_t a_Location, uint32_t a_V0, uint32_t a_V1 )
//{
//	*reinterpret_cast< Vector2UInt* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1 };
//}
//
//void ConsoleGL::Uniform3ui( int32_t a_Location, uint32_t a_V0, uint32_t a_V1, uint32_t a_V2 )
//{
//	*reinterpret_cast< Vector3UInt* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2 };
//}
//
//void ConsoleGL::Uniform4ui( int32_t a_Location, uint32_t a_V0, uint32_t a_V1, uint32_t a_V2, uint32_t a_V3 )
//{
//	*reinterpret_cast< Vector4UInt* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = { a_V0, a_V1, a_V2, a_V3 };
//}
//
//void ConsoleGL::Uniform1fv( int32_t a_Location, uint32_t a_Count, const float* a_Value )
//{
//	typedef std::array< float, 1 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) = 
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform2fv( int32_t a_Location, uint32_t a_Count, const float* a_Value )
//{
//	typedef std::array< float, 2 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform3fv( int32_t a_Location, uint32_t a_Count, const float* a_Value )
//{
//	typedef std::array< float, 3 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform4fv( int32_t a_Location, uint32_t a_Count, const float* a_Value )
//{
//	typedef std::array< float, 4 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform1iv( int32_t a_Location, uint32_t a_Count, const int32_t* a_Value )
//{
//	typedef std::array< int32_t, 1 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform2iv( int32_t a_Location, uint32_t a_Count, const int32_t* a_Value )
//{
//	typedef std::array< int32_t, 2 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform3iv( int32_t a_Location, uint32_t a_Count, const int32_t* a_Value )
//{
//	typedef std::array< int32_t, 3 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform4iv( int32_t a_Location, uint32_t a_Count, const int32_t* a_Value )
//{
//	typedef std::array< int32_t, 4 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform1uiv( int32_t a_Location, uint32_t a_Count, const uint32_t* a_Value )
//{
//	typedef std::array< uint32_t, 1 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform2uiv( int32_t a_Location, uint32_t a_Count, const uint32_t* a_Value )
//{
//	typedef std::array< uint32_t, 2 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform3uiv( int32_t a_Location, uint32_t a_Count, const uint32_t* a_Value )
//{
//	typedef std::array< uint32_t, 3 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::Uniform4uiv( int32_t a_Location, uint32_t a_Count, const uint32_t* a_Value )
//{
//	typedef std::array< uint32_t, 4 > Type;
//	*reinterpret_cast< Type* >( s_ShaderProgramRegistry[ s_ActiveShaderProgram ].m_Uniforms[ a_Location ] ) =
//		*reinterpret_cast< const Type* >( a_Value );
//}
//
//void ConsoleGL::UniformMatrix2fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Matrix2* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Matrix2* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Matrix2* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix3fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Matrix3* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Matrix3* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Matrix3* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix4fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Matrix4* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Matrix4* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Matrix4* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix2x3fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix2x3 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix3x2fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix3x2 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix2x4fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix2x4 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix4x2fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix4x2 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix3x4fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix3x4 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//void ConsoleGL::UniformMatrix4x3fv( uint32_t a_Location, uint32_t a_Count, bool a_Transpose, const float* a_Value )
//{
//	typedef Matrix4x3 Type;
//	auto& ActiveProgram = s_ShaderProgramRegistry[ s_ActiveShaderProgram ];
//	auto* UniformValue = reinterpret_cast< Type* >( ActiveProgram.m_Uniforms[ a_Location ] );
//
//	if ( !a_Transpose )
//	{
//		auto& Value = *reinterpret_cast< const Type* >( a_Value );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = Value;
//		}
//	}
//	else
//	{
//		auto Value = Math::Transpose( *reinterpret_cast< const Type* >( a_Value ) );
//
//		for ( uint32_t i = 0; i < a_Count; ++i )
//		{
//			UniformValue[ i ] = reinterpret_cast< Type& >( Value );
//		}
//	}
//}
//
//ShaderHandle ConsoleGL::CreateShader( ShaderType a_ShaderType )
//{
//	ShaderHandle NewHandle = s_ShaderRegistry.Create();
//	s_ShaderRegistry[ NewHandle ].Type = a_ShaderType;
//	return NewHandle;
//}
//
//void ConsoleGL::ShaderSource( ShaderHandle a_ShaderHandle, uint32_t a_Count, const void** a_Sources, uint32_t* a_Lengths )
//{
//	if ( a_Count < 1 )
//	{
//		return;
//	}
//
//	s_ShaderRegistry[ a_ShaderHandle ].Callback = ( void(*)() )a_Sources[ 0 ];
//}
//
//void ConsoleGL::CompileShader( ShaderHandle a_ShaderHandle )
//{
//	ShaderObject& ActiveShader = s_ShaderRegistry[ a_ShaderHandle ];
//
//	if ( !ActiveShader.Callback )
//	{
//		ActiveShader.Callback = ShaderObject::Empty;
//	}
//}
//
//void ConsoleGL::GetShaderIV( ShaderHandle a_ShaderHandle, ShaderInfo a_ShaderInfo, void* a_Value )
//{
//	switch ( a_ShaderInfo )
//	{
//		case ShaderInfo::COMPILE_STATUS:
//		{
//			break;
//		}
//		case ShaderInfo::INFO_LOG_LENGTH:
//		{
//			break;
//		}
//	}
//}
//
//void ConsoleGL::GetShaderInfoLog( ShaderHandle a_ShaderHandle, size_t a_BufferSize, size_t* a_Length, char* a_InfoLog )
//{}
//
//ShaderProgramHandle ConsoleGL::CreateProgram()
//{
//	return s_ShaderProgramRegistry.Create();
//}
//
//void ConsoleGL::AttachShader( ShaderProgramHandle a_ShaderProgramHandle, ShaderHandle a_ShaderHandle )
//{
//	auto& Program = s_ShaderProgramRegistry[ a_ShaderProgramHandle ];
//	auto& ShaderObject  = s_ShaderRegistry[ a_ShaderHandle ];
//	Program.m_Shaders[ ( uint32_t )ShaderObject.Type ].Handle = a_ShaderHandle;
//	Program.m_Shaders[ ( uint32_t )ShaderObject.Type ].Set = true;
//}
//
//void ConsoleGL::LinkProgram( ShaderProgramHandle a_ShaderProgramHandle )
//{
//	auto& Program = s_ShaderProgramRegistry[ a_ShaderProgramHandle ];
//	
//	for ( uint32_t i = 0; i < 2; ++i )
//	{
//		auto& Entry = Program.m_Shaders[ i ];
//
//		if ( !Entry.Set )
//		{
//			Entry.Callback = ShaderObject::Empty;
//			continue;
//		}
//
//		auto Callback = s_ShaderRegistry[ Entry.Handle ].Callback;
//		Program.m_Shaders[ i ].Callback = Callback;
//
//		// Get the vector of uniforms registered to the given function.
//		auto& Uniforms = s_UniformMap[ Callback ];
//		
//		for ( auto& Pair : Uniforms )
//		{
//			// First check if that uniform is already a part of the program.
//			if ( Program.m_UniformLocations.find( Pair.first ) == Program.m_UniformLocations.end() )
//			{
//				Program.m_UniformLocations[ Pair.first ] = Program.m_Uniforms.size();
//				Program.m_Uniforms.push_back( Pair.second );
//			}
//		}
//	}
//
//	// Populate uniforms.
//	
//}
//
//void ConsoleGL::GetProgramIV( ShaderProgramHandle a_ShaderProgramHandle, ShaderInfo a_ShaderInfo, void* a_Value )
//{}
//
//void ConsoleGL::GetProgramInfoLog( ShaderProgramHandle a_ShaderProgramHandle, size_t a_BufferSize, size_t * a_Length, char* a_InfoLog )
//{}
//
//void ConsoleGL::DetachShader( ShaderProgramHandle a_ShaderProgramHandle, ShaderHandle a_ShaderHandle )
//{
//	auto& Program = s_ShaderProgramRegistry[ a_ShaderProgramHandle ];
//	auto& ShaderObject  = s_ShaderRegistry[ a_ShaderHandle ];
//	auto& Entry   = Program.m_Shaders[ ( uint32_t )ShaderObject.Type ];
//	Entry.Handle = 0;
//	Entry.Set = false;
//}
//
//void ConsoleGL::DeleteShader( ShaderHandle a_ShaderHandle )
//{
//	s_ShaderRegistry.Destroy( a_ShaderHandle );
//}
//
//void ConsoleGL::DeleteProgram( ShaderProgramHandle a_ShaderProgramHandle )
//{
//	s_ShaderProgramRegistry.Destroy( a_ShaderProgramHandle );
//}
//
//void ConsoleGL::ActiveTexture( uint32_t a_ActiveTexture )
//{
//	s_ActiveTextureUnit = a_ActiveTexture;
//}
//
//void ConsoleGL::TexParameterf( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, float a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TexParameterfv( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, const float* a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TexParameteri( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, int32_t a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TexParameteri( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, const int32_t* a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TexParameterui( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, uint32_t a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TexParameterui( TextureTarget a_TextureTarget, TextureParameter a_TextureParameter, const uint32_t* a_Value )
//{
//	TexParameterImpl( a_TextureTarget, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameterf( TextureHandle a_Handle, TextureParameter a_TextureParameter, float a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameterfv( TextureHandle a_Handle, TextureParameter a_TextureParameter, const float* a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameteri( TextureHandle a_Handle, TextureParameter a_TextureParameter, int32_t a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameteri( TextureHandle a_Handle, TextureParameter a_TextureParameter, const int32_t* a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameterui( TextureHandle a_Handle, TextureParameter a_TextureParameter, uint32_t a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::TextureParameterui( TextureHandle a_Handle, TextureParameter a_TextureParameter, const uint32_t* a_Value )
//{
//	TextureParameterImpl( a_Handle, a_TextureParameter, a_Value );
//}
//
//void ConsoleGL::GenTextures( size_t a_Count, TextureHandle* a_Handles )
//{
//	while ( a_Count-- > 0 ) a_Handles[ a_Count ] = s_TextureRegistry.Create();
//}
//
//void ConsoleGL::BindTexture( TextureTarget a_TextureTarget, TextureHandle a_Handle )
//{
//	if ( s_TextureRegistry.Bind( a_TextureTarget, a_Handle ) )
//	{
//		s_ActiveTextureTarget = ( uint32_t )a_TextureTarget;
//	}
//
//	auto& ActiveTextureUnit = s_TextureUnits[ s_ActiveTextureUnit ];
//	ActiveTextureUnit[ s_ActiveTextureTarget ] = a_Handle;
//}
//
//void ConsoleGL::TexImage2D( TextureTarget a_TextureTarget, uint8_t a_MipMapLevel, TextureFormat a_InternalFormat, int32_t a_Width, int32_t a_Height, int32_t a_Border, TextureFormat a_TextureFormat, TextureSetting a_DataLayout, const void* a_Data )
//{
//	//TextureBuffer& Target = s_TextureRegistry[ s_TextureTargets[ ( uint32_t )a_TextureTarget ] ];
//	auto Handle = s_TextureUnits[ s_ActiveTextureUnit ][ ( uint32_t )a_TextureTarget ];
//	auto& Target = s_TextureRegistry[ Handle ];
//	Target.Data = a_Data;
//	Target.Dimensions = { a_Width, a_Height };
//
//	// Need to implement rest of all the settings.
//}