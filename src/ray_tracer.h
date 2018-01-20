#pragma once
#include <cmath>
#include <string>
#include <sstream>
#include "bvh.h"
class RayTracer {
	public:
		// Structure with basic options for raytracer
		// - width            : image witdh
		// - focalLength      : focal length of virtual camera
		// - nSuperSamples    : Number of Samples !per pixel!
		//                      Runtime basically explodes if you turn this too high
		// - smoothShading    : switch to turn on/off smooth shading
		// - ambientOcclusion : switch to turn on/off ambient occlusion
		// - aoMaxDistance    : Ambient Occlusion distance
		//                      Should be about 10% of the max scene dimension
		// - aoNumSamples     : Number of samples for each ambient occlusion
		//                      evaluation
		enum class AmbientOcclusionMethod { UNIFORM, RANDOM };
		struct Options {
			unsigned int width;
			unsigned int height;
			float focalLength;
			unsigned int nSuperSamples;
			bool enableShading;
			bool enableAO;
			float aoMaxDistance;
			unsigned int aoNumSamples;
			AmbientOcclusionMethod aoMethod;
			int aoAlphaMin;
			int aoAlphaMax;
			BVH::Method bvhMethod;
		};
		RayTracer(Options options) :
			options(options),
			totalWidth(options.width * (unsigned int) sqrt(options.nSuperSamples)),
			totalHeight(options.height * (unsigned int) sqrt(options.nSuperSamples)) {
		}
		void resize(float *tmp, unsigned char *image);
		const Options options;
		const unsigned int totalWidth;
		const unsigned int totalHeight;
};