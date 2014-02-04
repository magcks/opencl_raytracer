#include "opencl_host.h"

void OpenCLHost::printInfo() {
	std::vector<cl::Platform> allPlatforms;
	std::vector<cl::Device> allDevices;
	cl::Platform::get(&allPlatforms);

	Info info;
	info.setTitle("Hardware information");

	for (std::size_t i = 0; i < allPlatforms.size(); ++i) {
		cl::Platform platform = allPlatforms[i];

		Info platformInfo;
		std::stringstream platformTitle;
		platformTitle << "PLATFORM #" << i;
		platformInfo.setTitle(platformTitle.str());
		platformInfo.add("Profile",	platform.getInfo<CL_PLATFORM_PROFILE>());
		platformInfo.add("Version",	platform.getInfo<CL_PLATFORM_VERSION>());
		platformInfo.add("Name",	platform.getInfo<CL_PLATFORM_NAME>());
		platformInfo.add("Vendor",	platform.getInfo<CL_PLATFORM_VENDOR>());
		platformInfo.add("Extensions",	platform.getInfo<CL_PLATFORM_EXTENSIONS>());

		std::vector<cl::Device> platformDevices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

		for (std::size_t j = 0; j < platformDevices.size(); ++j) {
			cl::Device device = platformDevices[j];

			Info deviceInfo;
			std::stringstream deviceTitle;
			deviceTitle << "DEVICE #" << j;
			deviceInfo.setTitle(deviceTitle.str());
// 			deviceInfo.add("Extensions",		device.getInfo<CL_DEVICE_EXTENSIONS>());
			deviceInfo.add("Max work item sizes",	Info::vec2str(device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()));
			deviceInfo.add("Name",			device.getInfo<CL_DEVICE_NAME>());
			deviceInfo.add("Profile",		device.getInfo<CL_DEVICE_PROFILE>());
			deviceInfo.add("Vendor",		device.getInfo<CL_DEVICE_VENDOR>());
			deviceInfo.add("Version",		device.getInfo<CL_DEVICE_VERSION>());
			deviceInfo.add("Driver version",	device.getInfo<CL_DRIVER_VERSION>());
			deviceInfo.add("Available",		device.getInfo<CL_DEVICE_AVAILABLE>() ? "Yes" : "No");
			deviceInfo.add("Type",			deviceType(device.getInfo<CL_DEVICE_TYPE>()));
			deviceInfo.add("Max compute units",	device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>());
			deviceInfo.add("Local memory size (B)",	device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>());

			platformInfo.add(deviceInfo);

			allDevices.push_back(device);
		}

		info.add(platformInfo);
	}

	std::cout << std::endl;
	std::cout << info.str();

	if (allPlatforms.size() == 0)
		std::cout << Color::red << "Error: No platforms found." << Color::reset << std::endl;

	if (allDevices.size() == 0)
		std::cout << Color::red <<  "Error: No devices found." << Color::reset << std::endl;
}

void OpenCLHost::prepare(std::vector<cl_uint> &faces, std::vector<cl_uint> &nodes, std::vector<Vec3f> &aabbs, std::vector<Vec3f> &vertices, std::vector<Vec3f> &vnormals) {
	cl_int err;

	std::vector<cl::Platform> allPlatforms;
	cl::Platform::get(&allPlatforms);
	std::vector<cl::Device> allDevices;

	for (std::size_t i = 0; i < allPlatforms.size(); ++i) {
		cl::Platform platform = allPlatforms[i];
		std::vector<cl::Device> platformDevices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

		for (std::size_t j = 0; j < platformDevices.size(); ++j) {
			cl::Device device = platformDevices[j];
			allDevices.push_back(device);
		}
	}

	if (allDevices.size() == 0)
		return;

	// Sort devices
	std::sort(allDevices.begin(), allDevices.end(), devicePriority);

	cl::Device device = allDevices[0];

	std::string deviceName = device.getInfo<CL_DEVICE_NAME>();
	Info::trim(deviceName);

	std::cout << Color::red << "Using Device \"" << deviceName << "\"." << Color::reset << std::endl;

	std::vector<cl::Device> devices;
	devices.push_back(device);
// 		devices.push_back(allDevices[1]);

	cl::Context context(devices);

	cl::Program::Sources sources;

	std::cout << Color::blue << "<- " << Color::red << "OPENCL LOG SECTION" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Loading kernel source file..." << std::endl;
	std::fstream input("intersect_kernel.cl");
	std::stringstream kernelSourceSs;
	std::string line;

	while (std::getline(input, line))
		kernelSourceSs << line << std::endl;

	input.close();
	std::string kernelSource = kernelSourceSs.str();

	sources.push_back(std::pair<const char *, size_t>(kernelSource.c_str(), kernelSource.length()));

	// kernel parameters
	CompilerOptions co;
	co.add("WIDTH", this->rt.totalWidth);
	co.add("HEIGHT", this->rt.totalHeight);
	co.add("FOCALLENGTH", this->rt.opts.focalLength);
	co.add("NSUPERSAMPLES", this->rt.opts.nSuperSamples);
	co.add("SHADING", this->rt.opts.shading);
	co.add("AO", this->rt.opts.ambientOcclusion);
	co.add("AO_MAXDISTANCE", this->rt.opts.aoMaxDistance);
	co.add("AO_NUMSAMPLES", this->rt.opts.aoNumSamples);
	co.add("AO_METHOD", (unsigned int) this->rt.opts.aoMethod);
	co.add("AO_ALPHA_MIN", this->rt.opts.aoAlphaMin);
	co.add("AO_ALPHA_MAX", this->rt.opts.aoAlphaMax);


	std::string options(co.str());
	std::cout << "Build options: " << options << std::endl;

	std::cout << "Compiling kernel..." << std::endl;
	this->program = cl::Program(context, sources);
	err = this->program.build(devices, options.c_str(), NULL, NULL);

	std::cout << "Build log: " << std::endl << this->program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl << std::endl;

	check(err);

	std::size_t mem = 0;
	mem += faces.size() * sizeof(cl_uint);
	mem += nodes.size() * sizeof(cl_uint);
	mem += aabbs.size() * sizeof(Vec3f);
	mem += vertices.size() * sizeof(Vec3f);
	mem += vnormals.size() * sizeof(Vec3f);
	mem += this->rt.totalWidth * this->rt.totalHeight * sizeof(cl_float);

	std::cout << "Requesting " << mem << " Bytes of memory." << std::endl;

	this->facesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,	faces.size() * sizeof(cl_uint));
	this->nodesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,	nodes.size() * sizeof(cl_uint));
	this->aabbsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,	aabbs.size() * sizeof(Vec3f));
	this->verticesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,	vertices.size() * sizeof(Vec3f));
	this->vnormalsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,	vnormals.size() * sizeof(Vec3f));
	this->imageBuffer = cl::Image2D(context, CL_MEM_WRITE_ONLY,	cl::ImageFormat(CL_INTENSITY, CL_FLOAT), this->rt.totalWidth, this->rt.totalHeight, 0, NULL, NULL);
// 	this->imageBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY,	this->rt.totalWidth * this->rt.totalHeight * sizeof(cl_uchar)); // TODO: INFO: This is READ_WRITE, not WRITE_ONLY!

	this->queue = cl::CommandQueue(context, device);

	// Write data to GPU
	check(this->queue.enqueueWriteBuffer(this->facesBuffer, CL_TRUE, 0,	faces.size() * sizeof(cl_uint),		&(faces[0])));
	check(this->queue.enqueueWriteBuffer(this->nodesBuffer, CL_TRUE, 0,	nodes.size() * sizeof(cl_uint),		&(nodes[0])));
	check(this->queue.enqueueWriteBuffer(this->aabbsBuffer, CL_TRUE, 0,	aabbs.size() * sizeof(Vec3f),		&(aabbs[0])));
	check(this->queue.enqueueWriteBuffer(this->verticesBuffer, CL_TRUE, 0,	vertices.size() * sizeof(Vec3f),	&(vertices[0])));
	check(this->queue.enqueueWriteBuffer(this->vnormalsBuffer, CL_TRUE, 0,	vnormals.size() * sizeof(Vec3f),	&(vnormals[0])));
	check(this->queue.finish());
}

bool OpenCLHost::operator()() {
	cl::KernelFunctor intersect(cl::Kernel(this->program, "intersect"), this->queue, cl::NullRange, cl::NDRange(this->rt.totalWidth, this->rt.totalHeight), cl::NullRange);
	intersect(this->facesBuffer, this->nodesBuffer, this->aabbsBuffer, this->verticesBuffer, this->vnormalsBuffer, this->imageBuffer);
	check(intersect.getError());
	check(this->queue.finish());

	return true; // TODO
}

void OpenCLHost::loadMem(cl_float * image) {
	// set image grey (debugging)
	for (std::size_t i = 0; i < this->rt.totalWidth * this->rt.totalHeight; ++i) {
		image[i] = 0.7f;
	}

	cl::size_t<3> origin;
	origin.push_back(0);
	origin.push_back(0);
	origin.push_back(0);
	cl::size_t<3> region;
	region.push_back(this->rt.totalWidth);
	region.push_back(this->rt.totalHeight);
	region.push_back(1);
	check(this->queue.enqueueReadImage(this->imageBuffer, CL_TRUE, origin, region, 0, 0, image, NULL, NULL));
	check(this->queue.finish());
}

int OpenCLHost::deviceTypePriority(const cl_device_type &type) {
	switch (type) {
	case CL_DEVICE_TYPE_ACCELERATOR:
		return 2;

	case CL_DEVICE_TYPE_GPU:
		return 1;

	case CL_DEVICE_TYPE_CPU:
		return 0;
	}

	return -1;
}
// TODO: not tested (because I only have one device...)
bool OpenCLHost::devicePriority(const cl::Device a, const cl::Device b) {
	cl_device_type aType = a.getInfo<CL_DEVICE_TYPE>();
	cl_device_type bType = b.getInfo<CL_DEVICE_TYPE>();

	if (!a.getInfo<CL_DEVICE_AVAILABLE>())
		return false;

	return deviceTypePriority(aType) > deviceTypePriority(bType);
}