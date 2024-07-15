#pragma once

#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include <ConsoleGL.hpp>
#include <Event.hpp>
#include <FileMap.hpp>
#include <PixelBuffer.hpp>

namespace ConsoleGL
{
class WindowDock
{
    enum class ECommand
    {
        Release,
        Attach,
        Terminate
    };
    
    struct CommandBuffer
    {
        ECommand Command;
        uint32_t Value;
    };
    
    WindowDock();
    WindowDock( const WindowDock& ) = delete;

public:

    WindowDock( WindowDock&& a_WindowDock ) noexcept;
    WindowDock& operator=( WindowDock&& a_WindowDock ) noexcept;
    ~WindowDock();

private:

    WindowDock& operator=( const WindowDock& ) = delete;

    static WindowDock* Create();
    static bool Destroy( const WindowDock* a_WindowDock );

public:

    static int RunListener( const std::string& a_Name );

private:

    bool IsBorrowed() const;
    bool Borrow() const;
    bool Return() const;
    bool IsValid() const { return m_InternalInfo.get(); }

    friend class Window;
    struct InternalInfo;

    std::shared_ptr< InternalInfo > m_InternalInfo;
    FileMap                         m_CommandBuffer;
	Event                           m_CommandReady;
    Event                           m_CommandComplete;
    Event                           m_ProcessStarted;
    Window*                         m_Docked;

    static const WindowDock*        s_CurrentlyBorrowed;
    static std::list< WindowDock >  s_WindowDocks;
};



class Window
{
public:

    static const Colour ColourSetDefault[ 16 ];
    static const Colour ColourSetGreyscale[ 16 ];
    static const Colour ColourSetSepia[ 16 ];

    Window( const Window& ) = delete;
    Window( Window&& ) = delete;
    Window& operator=( const Window& ) = delete;
    Window& operator=( Window&& ) = delete;

    static Window* Create( const std::string& a_Title, uint32_t a_Width, uint32_t a_Height, uint32_t a_PixelWidth = 8, uint32_t a_PixelHeight = 8, uint32_t a_BufferCount = 2 );
    static void Destroy( Window* a_Window );
    static void SetActive( Window* a_ConsoleWindow );
    static Window* GetActive();
    static void SetTitle( const std::string& a_Title );
    static void SetColours( const Colour* a_Colours );
    static void SetColours( const EColourSet a_ColourSet );
    static void SwapBuffer();
    static void SwapBuffer( const uint32_t a_Index );

    const std::string& GetTitle() const { return m_Title; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetArea() const { return m_Width * m_Height; }
    uint32_t GetBufferIndex() const { return m_ActiveBuffer; }
    uint32_t GetBufferCount() const { return ( uint32_t )m_Buffers.size(); }
    const PixelBuffer* GetBuffer() const { return &m_Buffers[ m_ActiveBuffer ]; }
    PixelBuffer* GetBuffer() { return &m_Buffers[ m_ActiveBuffer ]; }
    PixelBuffer* GetBuffer( const uint32_t a_Index ) { return &m_Buffers[ a_Index ]; }
    const PixelBuffer* GetBuffer( const uint32_t a_Index ) const { return &m_Buffers[ a_Index ]; }
    bool HasFocus() const;

	static bool IsKeyDown( const EKeyboardKey a_KeyboardKey );
	static bool IsKeyUp( const EKeyboardKey a_KeyboardKey );
	static bool IsKeyPressed( const EKeyboardKey a_KeyboardKey );
	static bool IsKeyReleased( const EKeyboardKey a_KeyboardKey );
	static bool IsMouseDown( const EMouseButton a_MouseButton );
	static bool IsMouseUp( const EMouseButton a_MouseButton );
	static bool IsMousePressed( const EMouseButton a_MouseButton );
	static bool IsMouseReleased( const EMouseButton a_MouseButton );
	static void GetMousePosition( float& o_X, float& o_Y );
	static void GetMouseDelta( float& o_X, float& o_Y );
	static void PollEvents();

private:

    friend class WindowDock;
    
    Window( WindowDock& a_Dock );
    ~Window();

    uint32_t					m_Width;
    uint32_t					m_Height;
    uint32_t					m_PixelWidth;
    uint32_t					m_PixelHeight;
    std::vector< PixelBuffer >	m_Buffers;
    uint32_t					m_ActiveBuffer;
    std::string					m_Title;
    WindowDock*					m_Dock;

	static std::bitset< 99 >	s_KeyStates;
	static std::bitset< 3  >	s_MouseStates;
	static float				s_MouseX;
	static float				s_MouseY;
	static float				s_MouseDeltaX;
	static float				s_MouseDeltaY;
};
} // namespace ConsoleGL