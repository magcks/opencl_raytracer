#pragma once
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <CL/cl.hpp>
#include "color.h"
#include "compiler_options.h"
#include "ray_tracer.h"
#include "info.h"
#include "vec3.h"
class OpenCLHost {
	public:
		static int deviceTypePriority(const cl_device_type &type);
		// TODO: not tested (because I only have one device...)
		static bool devicePriority(const cl::Device a, const cl::Device b);
		static std::string deviceType(const cl_device_type &type) {
			switch (type) {
				case CL_DEVICE_TYPE_CPU:
					return "CPU";
				case CL_DEVICE_TYPE_GPU:
					return "GPU";
				case CL_DEVICE_TYPE_ACCELERATOR:
					return "Accelerator";
				case CL_DEVICE_TYPE_DEFAULT:
					return "Default";
			}
			return "Unknown";
		}
		static void check(cl_int err) {
			if (err != CL_SUCCESS) {
				std::cout << "OpenCL error: " << getErrorString(err) << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		static std::string getErrorString(cl_int err) {
			switch(err) {
				case 0:
					return "CL_SUCCESS";
				case -1:
					return "CL_DEVICE_NOT_FOUND";
				case -2:
					return "CL_DEVICE_NOT_AVAILABLE";
				case -3:
					return "CL_COMPILER_NOT_AVAILABLE";
				case -4:
					return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
				case -5:
					return "CL_OUT_OF_RESOURCES";
				case -6:
					return "CL_OUT_OF_HOST_MEMORY";
				case -7:
					return "CL_PROFILING_INFO_NOT_AVAILABLE";
				case -8:
					return "CL_MEM_COPY_OVERLAP";
				case -9:
					return "CL_IMAGE_FORMAT_MISMATCH";
				case -10:
					return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
				case -11:
					return "CL_BUILD_PROGRAM_FAILURE";
				case -12:
					return "CL_MAP_FAILURE";
				case -30:
					return "CL_INVALID_VALUE";
				case -31:
					return "CL_INVALID_DEVICE_TYPE";
				case -32:
					return "CL_INVALID_PLATFORM";
				case -33:
					return "CL_INVALID_DEVICE";
				case -34:
					return "CL_INVALID_CONTEXT";
				case -35:
					return "CL_INVALID_QUEUE_PROPERTIES";
				case -36:
					return "CL_INVALID_COMMAND_QUEUE";
				case -37:
					return "CL_INVALID_HOST_PTR";
				case -38:
					return "CL_INVALID_MEM_OBJECT";
				case -39:
					return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
				case -40:
					return "CL_INVALID_IMAGE_SIZE";
				case -41:
					return "CL_INVALID_SAMPLER";
				case -42:
					return "CL_INVALID_BINARY";
				case -43:
					return "CL_INVALID_BUILD_OPTIONS";
				case -44:
					return "CL_INVALID_PROGRAM";
				case -45:
					return "CL_INVALID_PROGRAM_EXECUTABLE";
				case -46:
					return "CL_INVALID_KERNEL_NAME";
				case -47:
					return "CL_INVALID_KERNEL_DEFINITION";
				case -48:
					return "CL_INVALID_KERNEL";
				case -49:
					return "CL_INVALID_ARG_INDEX";
				case -50:
					return "CL_INVALID_ARG_VALUE";
				case -51:
					return "CL_INVALID_ARG_SIZE";
				case -52:
					return "CL_INVALID_KERNEL_ARGS";
				case -53:
					return "CL_INVALID_WORK_DIMENSION";
				case -54:
					return "CL_INVALID_WORK_GROUP_SIZE";
				case -55:
					return "CL_INVALID_WORK_ITEM_SIZE";
				case -56:
					return "CL_INVALID_GLOBAL_OFFSET";
				case -57:
					return "CL_INVALID_EVENT_WAIT_LIST";
				case -58:
					return "CL_INVALID_EVENT";
				case -59:
					return "CL_INVALID_OPERATION";
				case -60:
					return "CL_INVALID_GL_OBJECT";
				case -61:
					return "CL_INVALID_BUFFER_SIZE";
				case -62:
					return "CL_INVALID_MIP_LEVEL";
				case -63:
					return "CL_INVALID_GLOBAL_WORK_SIZE";
				default:
					return "UNKNOWN";
			}
		}
		// Constructor
		OpenCLHost(const RayTracer &rt) : rt(rt) {}
		void prepare(const std::vector<uint32_t> &faces, const std::vector<uint32_t> &nodes, const std::vector<Vec3f> &aabbs, const std::vector<Vec3f> &vertices, const std::vector<Vec3f> &vnormals);
		bool operator()();
		void loadMemory(float *image);
		static void printInfo();
	private:
		const RayTracer &rt;
		cl::Program program;
		cl::CommandQueue queue;
		// Buffers
		cl::Buffer facesBuffer;
		cl::Buffer nodesBuffer;
		cl::Buffer aabbsBuffer;
		cl::Buffer verticesBuffer;
		cl::Buffer vnormalsBuffer;
// 		cl::Image2D imageBuffer;
		cl::Buffer imageBuffer;
};