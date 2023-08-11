#####################################################
# SETUP PROJECT
#####################################################
cmake_minimum_required(VERSION 3.10)
project(ConsoleGL)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#####################################################
# SETUP CONSOLEGL-CORE
#####################################################
file(GLOB CONSOLEGL_CORE_SOURCE_FILES ./ConsoleGL-Core/Include/ConsoleGL/*.cpp)
file(GLOB CONSOLEGL_CORE_HEADER_FILES ./ConsoleGL-Core/Include/ConsoleGL/*.hpp)
source_group("Source Files" FILES ${CONSOLEGL_CORE_SOURCE_FILES})
source_group("Header Files" FILES ${CONSOLEGL_CORE_HEADER_FILES})
add_compile_definitions(NOMINMAX)
add_compile_definitions(UNICODE)

add_library(ConsoleGL-Core STATIC 
	${CONSOLEGL_CORE_SOURCE_FILES}
	${CONSOLEGL_CORE_HEADER_FILES})

#####################################################
# SETUP CONSOLEDOCK
#####################################################
file(GLOB CONSOLE_DOCK_SOURCE_FILES ./ConsoleDock/*.cpp)
source_group("Source Files" FILES CONSOLE_DOCK_SOURCE_FILES})
add_compile_definitions(NOMINMAX)
add_compile_definitions(UNICODE)

add_executable(ConsoleDock
	${CONSOLE_DOCK_SOURCE_FILES})


#####################################################
# SETUP CONSOLEGL-SHADERS
#####################################################
file(GLOB CONSOLEGL_SHADER_FILES ./ConsoleGL-Shaders/Shaders/*.cpp)
file(GLOB CONSOLEGL_SHADERS_SOURCE_FILES ./ConsoleGL-Core/Include/ConsoleGL/*.cpp)
file(GLOB CONSOLEGL_SHADERS_HEADER_FILES ./ConsoleGL-Core/Include/ConsoleGL/*.hpp)
source_group("Shader Files" FILES ${CONSOLEGL_SHADER_FILES})
source_group("Source Files" FILES ${CONSOLEGL_CORE_SOURCE_FILES})
source_group("Header Files" FILES ${CONSOLEGL_CORE_HEADER_FILES})
add_compile_definitions(NOMINMAX)
add_compile_definitions(UNICODE)

add_library(ConsoleGL-Shaders SHARED 
	${CONSOLEGL_SHADER_FILES}
	${CONSOLEGL_CORE_SOURCE_FILES}
	${CONSOLEGL_CORE_HEADER_FILES})


#####################################################
# SETUP CONSOLEGL-EXAMPLE
#####################################################
file(GLOB CONSOLEGL_EXAMPLE_SOURCE_FILES ./ConsoleGL-Example/*.cpp)
file(GLOB CONSOLEGL_EXAMPLE_HEADER_FILES ./ConsoleGL-Example/*.hpp)
source_group("Source Files" FILES ${CONSOLEGL_EXAMPLE_SOURCE_FILES})
source_group("Header Files" FILES ${CONSOLEGL_EXAMPLE_HEADER_FILES})
add_executable(ConsoleGL-Example WIN32
	${CONSOLEGL_EXAMPLE_SOURCE_FILES}
	${CONSOLEGL_EXAMPLE_HEADER_FILES})
target_link_libraries(ConsoleGL-Example PUBLIC
	ConsoleGL-Core)
target_include_directories(ConsoleGL-Example PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/ConsoleGL-Example
	${PROJECT_SOURCE_DIR}/ConsoleGL-Core/Include)


#####################################################
# FINAL
#####################################################
if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0") 
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'ConsoleGL-Example' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ConsoleGL-Example )
    endif()
endif()
