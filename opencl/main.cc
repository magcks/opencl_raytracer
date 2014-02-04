#include <fstream>
#include <vector>
#include <limits>
#include <iostream>
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

#include <iterator>
#include <string>

#define _USE_MATH_DEFINES

// Main routine.
int main(int argc, char **argv) {
	// Check arguments.
	if (argc != 3) {
		std::cerr << Color::red << "Syntax: " << argv[0] << " <IN_MESH.OFF> <OUT_IMAGE.PGM>" << Color::reset << std::endl;
		return 1;
	}

	// Render options for output image.
	RayTracer::Options opts;
	opts.width = 600;
	opts.height = 600;
	opts.focalLength = 1.0f;
	opts.nSuperSamples = 4;
	opts.shading = true;
	opts.ambientOcclusion = true;
	opts.aoMaxDistance = 0.2f;
	opts.aoNumSamples = 3;
	opts.aoMethod = RayTracer::AO_METHOD_PERFECT; // IMPORTANT INFO: You've enabled 'Perfect AO hemispheres'. You have entered a circle count of x. This will result in a huge amount of rays. Note that the Perfect AO Hemisphere will generate much better pictures without noise with less rays and time than you would need using randomized hemispheres. See README.
	opts.aoAlphaMin = 4; // degrees!!!
	opts.aoAlphaMax = 90; // you shouldn't change this

	std::string inMesh(argv[1]);
	std::string outImage(argv[2]);

	// Read input mesh.
	std::cout << Color::blue << "<- " << Color::red << "BVH SECTION" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Reading input mesh..." << std::endl;
	Mesh mesh;
	load_off_mesh(inMesh, &mesh);
	compute_vertex_normals(&mesh);
	std::cout << Color::blue << "- " << Color::yellow << "Vertices: " << Color::red << mesh.vertices.size() << Color::yellow << std::endl;
	std::cout << Color::blue << "- " << Color::yellow << "Triangles: " << Color::red << (mesh.faces.size() / 3) << Color::reset << std::endl;

	RayTracer rt(opts);

	if (opts.aoMethod == RayTracer::AO_METHOD_PERFECT) {
		std::size_t rays = 0;
		float degrees = M_PI / 180;
		for (uint currentCircle = 0; currentCircle < opts.aoNumSamples; ++currentCircle) {
			float const stepAngleRad = (opts.aoAlphaMax * degrees) / opts.aoNumSamples; // the angle of each step

			float const angleRad = (stepAngleRad * currentCircle) + (opts.aoAlphaMin * degrees); // the "horizontal" angle

			rays += (std::size_t) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);
		}

		std::cout << Color::red << "IMPORTANT INFO: You've enabled 'Perfect AO hemispheres'. You have entered a circle count of " << opts.aoNumSamples << ". This will result in " << rays << " rays. Note that the Perfect AO Hemisphere will generate much better pictures without noise with less rays and time than you would need using randomized hemispheres." << Color::reset << std::endl;
	}

	// Build BVH.
	std::cout << Color::yellow << "Building BVH..." << Color::reset << std::flush;
	BVH bvh(BVH::METHOD_CUT_LONGEST_AXIS);
	{
		Timer timer;
		bvh.buildBVH(mesh);
		std::cout << "took " << Color::green << timer.get_elapsed() << "ms." << Color::reset << std::endl;
	}

	std::cout << std::endl;
        std::cout << Color::blue << "<- " << Color::red << "DEVICE SECTION" << Color::blue << " ->" << std::endl;

	OpenCLHost::printInfo();

	std::cout << Color::yellow << "Loading OpenCL kernel...\n" << Color::reset;

	std::size_t totalTime = 0;

	OpenCLHost host(rt);

	// Build the kernel
	{
		Timer timer;

		// Sort faces along triagnle order
		std::vector<cl_uint> sortedFaces;
		sortedFaces.reserve(mesh.faces.size());
		for (size_t i = 0; i < bvh.triangles.size(); ++i) {
			cl_uint faceID = bvh.triangles[i] * 3;

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
        std::cout << Color::blue << "<- " << Color::red << "RENDERING SECTION" << Color::blue << " ->" << std::endl;
	std::cout << Color::yellow << "Rendering image..." << Color::reset << std::flush;

	// Execute
	{
		Timer timer;
		bool success = host();

		if (!success)
			std::cout << Color::red << " FAILED!" << Color::reset;

		std::size_t elapsed = timer.get_elapsed();
		totalTime += elapsed;
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}

	cl_float * tmp = new cl_float[rt.totalWidth * rt.totalHeight];
	std::cout << std::endl;

	// Load memory
	{
		Timer timer;
		std::cout << Color::yellow << "Loading memory... " << Color::reset;
		host.loadMem(tmp);

		std::size_t elapsed = timer.get_elapsed();
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}

	// Resize
	unsigned char * image = new unsigned char[opts.width * opts.height];
	{
		Timer timer;
		std::cout << Color::yellow << "Resizing image on host... " << Color::reset;

		rt.resize(tmp, image);
		delete tmp;

		std::size_t elapsed = timer.get_elapsed();
		totalTime += elapsed;
		std::cout << " took " << Color::green << elapsed << "ms." << Color::reset << std::endl;
	}

	std::cout << Color::yellow << "Total time (without loading memory and building the BVH): " << Color::green << totalTime << "ms" << Color::reset << std::endl;

// 	std::ofstream infos("./infos.txt");
// 	infos << opts.getMashineReadable() << std::endl;
// 	infos << "TIME " << totalTime << std::endl;
// 	infos.close();

	// Write output image.
	std::ofstream out(outImage.c_str());

	if (!out.good()) {
		std::cerr << Color::red << "Error opening output file!" << Color::reset << std::endl;
		return 1;
	}

	out << "P5 " << opts.width << " " << opts.height << " 255\n";
	out.write(reinterpret_cast<char const *>(image), opts.width * opts.height);
	out.close();

	delete image;

	return 0;
}
