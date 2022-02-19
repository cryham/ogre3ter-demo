cmake_minimum_required( VERSION 3.0 )
project( EmptyProject )


set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )

include( CMake/Bootstrap.cmake )
include( CMake/Dependencies/OGRE.cmake )


## Fix 1
set(OGRE_SOURCE "/home/ch/_dev/og23/Ogre/ogre-next/")
set(OGRE_BINARIES "/home/ch/_dev/og23/Ogre/ogre-next/build/Release")

#This scripts will add all the cpp and h files under src and include folders
setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES, FALSE, FALSE )

## Fix 2 sdl
include_directories("/usr/include/SDL2")
include_directories("/home/ch/_dev/og23/Ogre/ogre-next-deps/build/ogredeps/include")  #for rapidjson
#message( ${SDL2_INCLUDE_DIR} )
#print_all_variables()


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
