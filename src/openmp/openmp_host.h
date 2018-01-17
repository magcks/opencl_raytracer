
#ifndef OPENCL_HOST_H
#define OPENCL_HOST_H

#include "color.h"
#include "intersect_kernel.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "ray_tracer.h"
#include "info.h"
#include "vec3.h"
#include "mesh.h"

class OpenMPHost {
public:
	// Constructor
	OpenMPHost(RayTracer & _rt) : rt(_rt),
	kernel(_rt.totalWidth, _rt.totalHeight, _rt.opts.focalLength, _rt.opts.nSuperSamples, _rt.opts.shading, _rt.opts.ambientOcclusion, _rt.opts.aoMaxDistance, _rt.opts.aoNumSamples, _rt.opts.aoMethod, _rt.opts.aoAlphaMin, _rt.opts.aoAlphaMax) {
	}


	bool operator()(Mesh &mesh, std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, float * image);


private:
	RayTracer & rt;

	IntersectKernel kernel;
};


#endif