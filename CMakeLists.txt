#This scripts assumes that your Ogre source code is in Dependencies/Ogre and that:
# In Windows you built Ogre into Dependencies/Ogre/build
# In Linux you built Release into Dependencies/Ogre/build/Release
# In Linux you built Debug into Dependencies/Ogre/build/Debug
#
# If your source code is not in "Dependencies/Ogre" you can use "mklink /D" to create
# a symbolic link to where the source code is located on Windows.
# On Linux, you can use "ln -s"

cmake_minimum_required( VERSION 3.0 )
project( TerrainDemo )


set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )

include( CMake/Bootstrap.cmake )
#include( CMake/FindPkgMacros.cmake )
#include( CMake/FindRapidjson.cmake )
#include( CMake/FindSDL2.cmake )
include( CMake/Dependencies/OGRE.cmake )


#  root dir
get_filename_component(DIR_ONE_ABOVE ../ ABSOLUTE)
message(STATUS ${DIR_ONE_ABOVE})

set(OGRE_NEXT ${DIR_ONE_ABOVE}/Ogre/ogre-next)

## Fix 1
set(OGRE_SOURCE ${OGRE_NEXT}/)
set(OGRE_BINARIES ${OGRE_NEXT}/build/Release)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(OGRE_BINARIES ${OGRE_NEXT}/build/Debug)
else()
	set(OGRE_BINARIES ${OGRE_NEXT}/build/Release)
endif()

#This scripts will add all the cpp and h files from src and include folders
setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES, FALSE, FALSE )

## Fix 2 sdl
include_directories("/usr/include/SDL2")
include_directories(${DIR_ONE_ABOVE}/Ogre/ogre-next-deps/build/ogredeps/include)  #for rapidjson


# Setup our application
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )
if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	if( NOT PLATFORM_X64 )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()
	add_definitions( -DUNICODE -D_UNICODE )
endif()


macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()


include_directories( "./include" )

add_recursive( ./src SOURCES )
add_recursive( ./include HEADERS )


add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES} )
target_link_libraries( ${PROJECT_NAME} ${OGRE_LIBRARIES} )
## Fix 3 sdl
target_link_libraries( ${PROJECT_NAME} "-lSDL2" )
