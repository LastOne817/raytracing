# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Graphics)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
  message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
  message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
  message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Compile external dependencies 
add_subdirectory (external)

if(INCLUDE_DISTRIB)
  add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
  external/glm-0.9.7.1/
  .
)

set(PNG_INCLUDE_DIR external/libpng-1.6.29/)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

set(ALL_LIBS
  )

add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
)

aux_source_directory(common COMMON_SOURCES)

add_executable(hw5
  hw5/main.cpp
  ${COMMON_SOURCES})
target_link_libraries(hw5
  ${ALL_LIBS}
  ${PNG_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

add_custom_command(
  TARGET hw5 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/hw5${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/hw5/"
)
