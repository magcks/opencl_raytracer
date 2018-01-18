cmake_minimum_required(VERSION 3.1)

project(OpenCLRaytracer)
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to 'RELEASE' as none was specified.")
	set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(OpenCL REQUIRED)

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