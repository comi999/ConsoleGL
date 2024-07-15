#pragma once
#include <Macro.hpp>

//==========================================================================
// Output specifiers.
//==========================================================================

// Is the output of this project a shared library? ie. .dll, .so
#ifndef IS_SHARED_LIB
	#define IS_SHARED_LIB 0
#endif

// Is the output of this project a static library? ie. .lib, .a
#ifndef IS_STATIC_LIB
	#define IS_STATIC_LIB 0
#endif

// Is the output of this project an executable? ie. .exe
#ifndef IS_EXE
	#define IS_EXE 0
#endif

//==========================================================================
// Configuration specifiers.
//==========================================================================

#ifndef CONFIG_DEBUG
	#define CONFIG_DEBUG 0
#endif

#ifndef CONFIG_RELEASE
	#define CONFIG_RELEASE 0
#endif

//==========================================================================
// Application specifiers.
//==========================================================================
#ifndef IS_CONSOLEGL
	#define IS_CONSOLEGL 0
#endif

#ifndef IS_CONSOLE_DOCK
	#define IS_CONSOLE_DOCK 0
#endif

#ifndef IS_PIXEL_MAP
	#define IS_PIXEL_MAP 0
#endif

#ifndef IS_SHADER_COMPILER
	#define IS_SHADER_COMPILER 0
#endif

//==========================================================================
// Output decorators.
//==========================================================================
#ifdef __GNUC__
	#define CONSOLEGL_API_EXPORT extern "C" __attribute__((dllexport))
	#define CONSOLEGL_API_IMPORT extern "C" __attribute__((dllimport))
#else
	#define CONSOLEGL_API_EXPORT extern "C" __declspec(dllexport)
	#define CONSOLEGL_API_IMPORT extern "C" __declspec(dllimport)
#endif

#if IS_STATIC_LIB && IS_CONSOLEGL
	#define CONSOLEGL_API
#elif IS_SHARED_LIB && IS_CONSOLEGL
	#define CONSOLEGL_API CONSOLEGL_API_EXPORT
#elif defined CONSOLEGL_DLL_IMPORT
	#define CONSOLEGL_API CONSOLEGL_API_IMPORT
#else
	#define CONSOLEGL_API
#endif

//==========================================================================
// Function decorators.
//==========================================================================
#define NO_INLINE __declspec(noinline)

#if CONFIG_DEBUG
	#define FORCE_INLINE
#else
	#define FORCE_INLINE __forceinline
#endif