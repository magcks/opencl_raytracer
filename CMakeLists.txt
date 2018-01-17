cmake_minimum_required(VERSION 3.1)

project(OpenCLRaytracer)

include(ExternalProject)
ExternalProject_Add(apx-git
	GIT_REPOSITORY git@github.com:kdex/apx.git
	PREFIX ${CMAKE_BINARY_DIR}/apx
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	UPDATE_DISCONNECTED true
)
ExternalProject_Get_Property(apx-git INSTALL_DIR)
include_directories(${INSTALL_DIR}/include)
link_directories(${INSTALL_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'RELEASE' as none was specified.")
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

# c++14 support
if(CMAKE_VERSION VERSION_LESS "3.1")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "--std=gnu++14 ${CMAKE_CXX_FLAGS}")
	endif()
else()
	set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(OpenCL)
find_package(OpenMP)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

file(GLOB src "src/*.cc")
file(GLOB srccl "src/*.cl")

foreach(f ${srccl})
	get_filename_component(F ${f} NAME)
	string(TOUPPER ${F} F)
	STRING(REGEX REPLACE "\\." "_" F ${F})
	add_definitions(-D${F}="${f}")
endforeach()

add_executable(render ${src})

if(OpenCL_FOUND)
	target_link_libraries(render ${OpenCL_LIBRARIES})
	target_include_directories(render PUBLIC ${OpenCL_INCLUDE_DIRS})
endif()
add_dependencies(render apx-git)
target_link_libraries(render apx)