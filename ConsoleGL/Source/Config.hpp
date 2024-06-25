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
// Output decorators.
//==========================================================================
#if IS_SHARED_LIB
	#ifdef __GNUC__
		#define CONSOLEGL_API extern "C" __attribute__((dllexport))
	#else
		#define CONSOLEGL_API extern "C" __declspec(dllexport)
	#endif
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