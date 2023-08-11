#include <ConsoleGL/ConsoleWindow.hpp>
#include <iostream> 
#include <filesystem>

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
	//auto window1 = ConsoleGL::Window::Create( "Test_Title1", 100, 100, 8, 8 );
	//auto window2 = ConsoleGL::Window::Create( "Test Title2", 50, 50, 8, 8 );
	//auto window3 = ConsoleGL::Window::Create( "Test Title3", 75, 75, 8, 8 );
	//auto window4 = ConsoleGL::Window::Create( "Test Title4", 125, 125, 8, 8 );

	//ConsoleGL::Window::SetActive( window2 );
	//std::cout << "Hi there2!" << std::endl;
	//ConsoleGL::Window::SetActive( window3 );
	//std::cout << "Hi there3!" << std::endl;

	//ConsoleGL::Window::Destroy( window1 );
	//ConsoleGL::Window::Destroy( window2 );
	//ConsoleGL::Window::Destroy( window3 );
	//ConsoleGL::Window::Destroy( window4 );

	auto wake_event_name = L"WakeEvent_ConsoleDock1";
	auto sleep_event_name = L"SleepEvent_ConsoleDock1";
	auto awake_event = CreateEvent( NULL, true, false, wake_event_name );
	auto sleep_event = CreateEvent( NULL, true, false, sleep_event_name );

	{

#define BUF_SIZE 256

		TCHAR szName[] = TEXT( "Global\\MyFileMappingObject" );
		TCHAR szMsg[] = TEXT( "Message from first process." );

		HANDLE hMapFile;
		LPCTSTR pBuf;

		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			BUF_SIZE,                // maximum object size (low-order DWORD)
			szName );                 // name of mapping object

		if ( hMapFile == NULL )
		{
			_tprintf( TEXT( "Could not create file mapping object (%d).\n" ),
				GetLastError() );
			return 1;
		}
		pBuf = ( LPTSTR )MapViewOfFile( hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			BUF_SIZE );

		if ( pBuf == NULL )
		{
			_tprintf( TEXT( "Could not map view of file (%d).\n" ),
				GetLastError() );

			CloseHandle( hMapFile );

			return 1;
		}


		CopyMemory( ( PVOID )pBuf, szMsg, ( _tcslen( szMsg ) * sizeof( TCHAR ) ) );
		_getch();

		UnmapViewOfFile( pBuf );

		CloseHandle( hMapFile );
	}
	
	if ( awake_event == NULL || sleep_event == NULL )
	{
		return false;
	}

	auto window1 = ConsoleGL::Window::Create( "Test_Title1", 100, 100, 8, 8 );
	SetEvent( awake_event );
	SetEvent( sleep_event );
	return 0;
}