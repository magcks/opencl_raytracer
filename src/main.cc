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
#define _USE_MATH_DEFINES
int main(int argc, const char *argv[]) {
	RayTracer::Options options;
	options.width = 600;
	options.height = 600;
	options.focalLength = 1;
	options.nSuperSamples = 4;
	options.enableShading = true;
	options.enableAO = true;
	options.aoMaxDistance = 0.2f;
	options.aoNumSamples = 3;
	options.aoMethod = /*RayTracer::AO_METHOD_PERFECT*/RayTracer::AO_METHOD_PERFECT;
	options.aoAlphaMin = 4; // degrees!!!
	options.aoAlphaMax = 90; // you shouldn't change this
	options.bvhMethod = BVH::METHOD_CUT_LONGEST_AXIS; // or: BVH::METHOD_SAH
	std::string inMesh(argv[1]);
	std::string outImage(argv[2]);
	// Read input mesh.
	std::cout << Color::blue << "<- " << Color::red << "BVH section" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Reading input mesh…" << std::endl;
	Mesh mesh;
	load_off_mesh(inMesh, &mesh);
	compute_vertex_normals(&mesh);
	std::cout << Color::blue << "- " << Color::yellow << "Vertices: " << Color::red << mesh.vertices.size() << Color::yellow << std::endl;
	std::cout << Color::blue << "- " << Color::yellow << "Triangles: " << Color::red << (mesh.faces.size() / 3) << Color::reset << std::endl;
	RayTracer rt(options);
	if (options.aoMethod == RayTracer::AO_METHOD_PERFECT) {
		std::size_t rays = 0;
		float degrees = M_PI / 180;
		for (uint currentCircle = 0; currentCircle < options.aoNumSamples; ++currentCircle) {
			float const stepAngleRad = (options.aoAlphaMax * degrees) / options.aoNumSamples; // the angle of each step

			float const angleRad = (stepAngleRad * currentCircle) + (options.aoAlphaMin * degrees); // the "horizontal" angle

			rays += (std::size_t) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);
		}

		std::cout << Color::red << "IMPORTANT INFO: You've enabled 'Perfect AO hemispheres'. You have entered a circle count of " << options.aoNumSamples << ". This will result in " << rays << " rays. Note that the Perfect AO Hemisphere will generate much better pictures without noise with less rays and time than you would need using randomized hemispheres." << Color::reset << std::endl;
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
		host.prepare(sortedFaces, bvh.nodes, bvh.aabbs, mesh.vertices, mesh.vnormals);
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
		host.loadMemory(tmp.data());
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
	std::ofstream out(outImage.c_str());
	if (!out.good()) {
		std::cerr << Color::red << "Error opening output file!" << Color::reset << std::endl;
		return 1;
	}
	out << "P5 " << options.width << " " << options.height << " 255\n";
	out.write((const char*)image.data(), options.width * options.height);
	out.close();
	return 0;
}