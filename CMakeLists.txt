project(OpenCLRaytracer)
cmake_minimum_required(VERSION 3.1)
find_package(OpenCL REQUIRED)
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to \"RELEASE\" as none was specified.")
	set(CMAKE_BUILD_TYPE RELEASE)
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}
)
set(COMMON_CXX_FLAGS
	"\
	-fno-builtin\
	-pedantic\
	-Wall\
	-Wcast-align\
	-Wextra\
	-Wmissing-declarations\
	-Woverloaded-virtual\
	-Wredundant-decls\
	-Wundef\
	-Wwrite-strings\
	-Wno-parentheses\
	"
)
set(COMMON_CXX_RELEASE_FLAGS
	"\
	${COMMON_CXX_FLAGS}\
	"
)
set(COMMON_CXX_DEBUG_FLAGS
	"\
	${COMMON_CXX_FLAGS}\
	-fsanitize=address\
	-fsanitize=undefined\
	"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_DEBUG_FLAGS}")
include_directories(include)
get_filename_component(INTERSECT_KERNEL_CL_PATH "src/intersect_kernel.cl" REALPATH)
add_definitions(-DINTERSECT_KERNEL_CL="${INTERSECT_KERNEL_CL_PATH}")
add_executable(render
	src/aabb.cc
	src/bvh.cc
	src/color.cc
	src/info.cc
	src/mesh.cc
	src/opencl_host.cc
	src/ray_tracer.cc
	src/render.cc
	src/timer.cc
	src/triangle.cc
)
if(OpenCL_FOUND)
	target_link_libraries(render ${OpenCL_LIBRARIES})
	target_include_directories(render PUBLIC ${OpenCL_INCLUDE_DIRS})
endif()