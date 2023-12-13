# AbstractGPU cmake build script.
cmake_minimum_required(VERSION 3.5)
project(Tuuvm C)

if(MSVC)
    set(CMAKE_C_FLAGS "/std:c11 /experimental:c11atomics  ${CMAKE_C_FLAGS}")
    #SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /STACK:0x2000000")
else()
    set(CMAKE_C_FLAGS "-Wall -Wextra ${CMAKE_C_FLAGS}")
endif()

if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

include(CTest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    .
)

set(MainOutputDirectory "${Tuuvm_BINARY_DIR}/dist")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MainOutputDirectory} )

set(SYSBVM_DEP_LIBS)
if(UNIX)
    set(SYSBVM_DEP_LIBS m)
endif()

add_definitions(-DBUILD_SYSBVM_STATIC)

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
