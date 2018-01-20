#include <fstream>
#include <vector>
#include <limits>
#include <iostream>
#include <iterator>
#include <string>
#include <stdlib.h>
#include "timer.h"
#include "mesh.h"
#include "vec3.h"
#include "bvh.h"
#include "triangle.h"
#include "ray_tracer.h"
#include "info.h"
#include "color.h"
#include "opencl_host.h"
#include "args.h"
#define _USE_MATH_DEFINES

struct Options : RayTracer::Options {
	Options(int argc, const char **argv) : RayTracer::Options{ 600, 600, 1.f, 4, true, true, .2f, 3, RayTracer::AmbientOcclusionMethod::UNIFORM, 4, 90, BVH::Method::CUT_LONGEST_AXIS }
	{
		args::parser args(argc, argv, "An OpenCL raytracer that renders triangle meshes in OFF format.");
		const int ARG_IN = args.add_nonopt("INPUT_MESH");
		const int ARG_OUT = args.add_nonopt("OUTPUT_IMAGE");
		args.range(2, 2);
		const int ARG_W = args.add_opt('w', "width", "Specifies the width to use for the output image.");
		const int ARG_H = args.add_opt('h', "height", "Specifies the height to use for the output image.");
		const int ARG_A = args.add_opt('a', "ambient-occlusion-samples", "Specifies the number of samples used for ambient occlusion. If the value `0` is specified, ambient occlusion will be disabled.");
		const int ARG_D = args.add_opt('d', "ambient-occlusion-max-distance", "Specifies the maximum distance that should be allowed for ambient occlusion rays.");
		const int ARG_M = args.add_opt('m', "ambient-occlusion-method", "Specifies the method of ambient occlusion [uniform|random].");
		const int ARG_F = args.add_opt('f', "focal-length", "Specifies the focal length that the camera should use.");
		const int ARG_S = args.add_opt('s', "supersamples", "Specifies the number of supersamples to use.");
		const int ARG_R = args.add_opt('r', "bvh-strategy", "Specifies the strategy of BVH construction (longest|sah).");
		for (int arg = args.next(); arg != args::parser::end; arg = args.next()) {
			if (arg == ARG_IN) in = args.val<std::string>();
			else if (arg == ARG_OUT) out = args.val<std::string>();
			else if (arg == ARG_W) width = args.val<std::size_t>();
			else if (arg == ARG_H) height = args.val<std::size_t>();
			else if (arg == ARG_A) aoNumSamples = args.val<std::size_t>();
			else if (arg == ARG_D) aoMaxDistance = args.val<float>();
			else if (arg == ARG_M) aoMethod = args.map(std::string("uniform"), RayTracer::AmbientOcclusionMethod::UNIFORM, std::string("random"), RayTracer::AmbientOcclusionMethod::RANDOM);
			else if (arg == ARG_F) focalLength = args.val<float>();
			else if (arg == ARG_S) nSuperSamples = args.val<std::size_t>();
			else if (arg == ARG_R) bvhMethod = args.map(std::string("longest"), BVH::Method::CUT_LONGEST_AXIS, std::string("sah"), BVH::Method::SURFACE_AREA_HEURISTIC);
			enableAO = aoNumSamples != 0;
		}
	}

	std::string in, out;
};

int main(int argc, const char **argv) {
	Options options(argc, argv);
	// Read input mesh.
	std::cout << Color::blue << "<- " << Color::red << "BVH section" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Reading input mesh…" << std::endl;
	Mesh mesh;
	load_off_mesh(options.in, &mesh);
	compute_vertex_normals(&mesh);
	std::cout << Color::blue << "- " << Color::yellow << "Vertices: " << Color::red << mesh.vertices.size() << Color::yellow << std::endl;
	std::cout << Color::blue << "- " << Color::yellow << "Triangles: " << Color::red << (mesh.faces.size() / 3) << Color::reset << std::endl;
	RayTracer rt(options);
	if (options.aoMethod == RayTracer::AmbientOcclusionMethod::UNIFORM) {
		std::size_t rays = 0;
		const float degrees = M_PI / 180;
		for (uint currentCircle = 0; currentCircle < options.aoNumSamples; ++currentCircle) {
			// the angle of each step
			const float stepAngleRad = (options.aoAlphaMax * degrees) / options.aoNumSamples;
			// the "horizontal" angle
			const float angleRad = (stepAngleRad * currentCircle) + (options.aoAlphaMin * degrees);
			rays += (std::size_t) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);
		}
		std::cout << Color::red << "IMPORTANT INFO: You've enabled 'Uniform AO hemispheres'. You have entered a circle count of " << options.aoNumSamples << ". This will result in " << rays << " rays. Note that the Uniform AO Hemisphere will generate much better pictures without noise with less rays and time than you would need using randomized hemispheres." << Color::reset << std::endl;
	}
	// Build BVH.
	std::cout << Color::yellow << "Building BVH…" << Color::reset << std::flush;
	BVH bvh(options.bvhMethod);
	{
		Timer timer;
		bvh.buildBVH(mesh);
		std::cout << " took " << Color::green << timer.get_elapsed() << "ms." << Color::reset << std::endl;
	}
	std::cout << std::endl;
	std::cout << Color::blue << "<- " << Color::red << "Device section" << Color::blue << " ->" << std::endl;
	OpenCLHost::printInfo();
	std::cout << Color::yellow << "Loading OpenCL kernel…\n" << Color::reset;
	std::size_t totalTime = 0;
	OpenCLHost host(rt);
	// Build the kernel
	{
		Timer timer;
		// Sort faces along triangle order
		std::vector<uint32_t> sortedFaces;
		sortedFaces.reserve(mesh.faces.size());
		for (size_t i = 0; i < bvh.triangles.size(); ++i) {
			const uint32_t faceID = bvh.triangles[i] * 3;
			sortedFaces.push_back(mesh.faces[faceID]);
			sortedFaces.push_back(mesh.faces[faceID + 1]);
			sortedFaces.push_back(mesh.faces[faceID + 2]);
		}
		mesh.faces.clear();
		bvh.triangles.clear();
		host.upload(sortedFaces, bvh.nodes, bvh.aabbs, mesh.vertices, mesh.vnormals);
		sortedFaces.clear();
		bvh.nodes.clear();
		bvh.aabbs.clear();
		mesh.vertices.clear();
		mesh.vnormals.clear();
		std::size_t elapsed = timer.get_elapsed();
		std::cout << "Building the kernel took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}
	std::cout << std::endl;
	std::cout << Color::blue << "<- " << Color::red << "Rendering section" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Rendering image…" << Color::reset << std::flush;
	// Execute
	{
		Timer timer;
		bool success = host();
		if (!success) {
			std::cout << Color::red << " failed!" << Color::reset;
		}
		std::size_t elapsed = timer.get_elapsed();
		totalTime += elapsed;
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}
	std::vector<float> tmp(rt.totalWidth * rt.totalHeight);
	std::cout << std::endl;
	// Load memory
	{
		Timer timer;
		std::cout << Color::yellow << "Loading memory…" << Color::reset;
		host.download(tmp.data());
		std::size_t elapsed = timer.get_elapsed();
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}
	// Resize
	std::vector<unsigned char> image(options.width * options.height);
	{
		Timer timer;
		std::cout << Color::yellow << "Resizing image on host…" << Color::reset;
		rt.resize(tmp.data(), image.data());
		std::size_t elapsed = timer.get_elapsed();
		totalTime += elapsed;
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}
	std::cout << Color::yellow << "Total time (without loading memory and building the BVH): " << Color::green << totalTime << "ms" << Color::reset << std::endl;
	// Write output image.
	std::ofstream out(options.out.c_str());
	if (!out.good()) {
		std::cerr << Color::red << "Error opening output file!" << Color::reset << std::endl;
		return 1;
	}
	out << "P5 " << options.width << " " << options.height << " 255\n";
	out.write((const char*)image.data(), options.width * options.height);
	out.close();
	return 0;
}