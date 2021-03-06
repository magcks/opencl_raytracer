#include <stdexcept>
#include <sstream>
#include <cstddef>
#include "color.h"
#include "compiler_options.h"
#include "info.h"
#include "opencl_host.h"

struct Resource {
	const char *data;
	const std::size_t size;
};
extern "C" Resource INTERSECT_KERNEL(void);

OpenCLHost::OpenCLHost(const RayTracer &rt) : rt(rt) {
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl::Device device;
	bool device_available = false;
	for (auto i = 0u; i < platforms.size(); ++i) {
		std::vector<cl::Device> devices;
		platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (auto j = 0u; j < devices.size(); ++j) {
			device = devices[j];
			device_available = true;
			if (device.getInfo<CL_DEVICE_AVAILABLE>() && device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
				goto DEVICE_FOUND;
		}
	}
	if (!device_available)
		throw std::runtime_error("No device found");
DEVICE_FOUND:
	std::string deviceName = device.getInfo<CL_DEVICE_NAME>();
	std::cout << Color::WHITE << "Using Device \"" << deviceName << "\"." << Color::RESET << std::endl << std::endl;
	context = cl::Context(std::vector<cl::Device>{ device });
	cl::Program::Sources sources;
	std::cout << Color::BLUE << "<- " << Color::GREEN << "OpenCL log section" << Color::BLUE << " ->" << std::endl;

	// kernel parameters
	Resource kernel = INTERSECT_KERNEL();
	sources.push_back(std::pair<const char *, std::size_t>(kernel.data, kernel.size));
	CompilerOptions co;
	co.add("WIDTH", rt.totalWidth);
	co.add("HEIGHT", rt.totalHeight);
	co.add("FOCAL_LENGTH", rt.options.focalLength);
	co.add("NSUPERSAMPLES", rt.options.nSuperSamples);
	co.add("SHADING_ENABLE", rt.options.enableShading);
	co.add("AO_ENABLE", rt.options.enableAO);
	co.add("AO_MAX_DISTANCE", rt.options.aoMaxDistance);
	co.add("AO_NUM_SAMPLES", rt.options.aoNumSamples);
	co.add("AO_METHOD", (std::size_t) rt.options.aoMethod);
	co.add("AO_ALPHA_MIN", rt.options.aoAlphaMin);
	co.add("AO_ALPHA_MAX", rt.options.aoAlphaMax);
	std::string options(co.str());
	Info::measure("Compiling kernel", [&] {
		std::cout << "Build options: " << options << std::endl;
		program = cl::Program(context, sources);
		cl_int status;
		status = program.build(std::vector<cl::Device>{ device }, options.c_str());
		if (status != CL_SUCCESS) {
			std::cerr
				<< std::endl
				<< "Build log:"
				<< std::endl << std::endl
				<< Color::RED
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
				<< Color::RESET
				<< std::endl;
			return false;
		}
		return true;
	}, true);

	queue = cl::CommandQueue(context, device);
}
void OpenCLHost::printInfo() {
	std::vector<cl::Platform> allPlatforms;
	std::vector<cl::Device> allDevices;
	cl::Platform::get(&allPlatforms);
	Info info;
	info.setTitle("Hardware information");
	for (auto i = 0u; i < allPlatforms.size(); ++i) {
		cl::Platform platform = allPlatforms[i];
		Info platformInfo;
		std::stringstream platformTitle;
		platformTitle << "Platform #" << i;
		platformInfo.setTitle(platformTitle.str());
		platformInfo.add("Profile", platform.getInfo<CL_PLATFORM_PROFILE>());
		platformInfo.add("Version", platform.getInfo<CL_PLATFORM_VERSION>());
		platformInfo.add("Name", platform.getInfo<CL_PLATFORM_NAME>());
		platformInfo.add("Vendor", platform.getInfo<CL_PLATFORM_VENDOR>());
		platformInfo.add("Extensions", platform.getInfo<CL_PLATFORM_EXTENSIONS>());
		std::vector<cl::Device> platformDevices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);
		for (auto j = 0u; j < platformDevices.size(); ++j) {
			cl::Device device = platformDevices[j];
			Info deviceInfo;
			std::stringstream deviceTitle;
			deviceTitle << "Device #" << j;
			deviceInfo.setTitle(deviceTitle.str());
			deviceInfo.add("Extensions", device.getInfo<CL_DEVICE_EXTENSIONS>());
			deviceInfo.add("Max work item sizes", Info::vec2str(device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()));
			deviceInfo.add("Name", device.getInfo<CL_DEVICE_NAME>());
			deviceInfo.add("Profile", device.getInfo<CL_DEVICE_PROFILE>());
			deviceInfo.add("Vendor", device.getInfo<CL_DEVICE_VENDOR>());
			deviceInfo.add("Version", device.getInfo<CL_DEVICE_VERSION>());
			deviceInfo.add("Driver version", device.getInfo<CL_DRIVER_VERSION>());
			deviceInfo.add("Available", device.getInfo<CL_DEVICE_AVAILABLE>() ? "Yes" : "No");
			deviceInfo.add("Type", deviceType(device.getInfo<CL_DEVICE_TYPE>()));
			deviceInfo.add("Max compute units", device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>());
			deviceInfo.add("Local memory size (B)", device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>());
			platformInfo.add(deviceInfo);
			allDevices.push_back(device);
		}
		info.add(platformInfo);
	}
	std::cout << std::endl;
	std::cout << info.str();
}
void OpenCLHost::upload(const std::vector<uint32_t> &faces, const std::vector<uint32_t> &nodes, const std::vector<Vec3f> &aabbs, const std::vector<Vec3f> &vertices, const std::vector<Vec3f> &vnormals) {
	auto mem = 0u;
	facesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, mem += faces.size() * sizeof(uint32_t));
	nodesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, mem +=nodes.size() * sizeof(uint32_t));
	aabbsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, mem +=aabbs.size() * sizeof(Vec3f));
	verticesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, mem +=vertices.size() * sizeof(Vec3f));
	vnormalsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, mem +=vnormals.size() * sizeof(Vec3f));
	imageBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, mem +=rt.totalWidth * rt.totalHeight * sizeof(float));
	std::cout << "Requested " << mem / 1024 << " kB of memory." << std::endl;
	// Write data to GPU
	check(queue.enqueueWriteBuffer(facesBuffer, CL_TRUE, 0, faces.size() * sizeof(uint32_t), faces.data()));
	check(queue.enqueueWriteBuffer(nodesBuffer, CL_TRUE, 0, nodes.size() * sizeof(uint32_t), nodes.data()));
	check(queue.enqueueWriteBuffer(aabbsBuffer, CL_TRUE, 0, aabbs.size() * sizeof(Vec3f), aabbs.data()));
	check(queue.enqueueWriteBuffer(verticesBuffer, CL_TRUE, 0, vertices.size() * sizeof(Vec3f), vertices.data()));
	check(queue.enqueueWriteBuffer(vnormalsBuffer, CL_TRUE, 0, vnormals.size() * sizeof(Vec3f), vnormals.data()));
	check(queue.finish());
}
bool OpenCLHost::operator()() {
	cl::Kernel kernel(program, "intersect");
	kernel.setArg(0, facesBuffer);
	kernel.setArg(1, nodesBuffer);
	kernel.setArg(2, aabbsBuffer);
	kernel.setArg(3, verticesBuffer);
	kernel.setArg(4, vnormalsBuffer);
	kernel.setArg(5, imageBuffer);
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(rt.totalWidth, rt.totalHeight), cl::NDRange(16, 16));
	cl_int err;
	check(err = queue.finish());
	return err == CL_SUCCESS;
}
void OpenCLHost::download(float *image) {
	queue.enqueueReadBuffer(imageBuffer, CL_TRUE, 0, rt.totalWidth * rt.totalHeight * sizeof(float), image);
	check(queue.finish());
}