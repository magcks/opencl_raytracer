#pragma once
#include <cmath>
#include <string>
#include <sstream>
#include "bvh.h"
class RayTracer {
	public:
		// Structure with basic options for raytracer
		// - width : image witdh
		// - focalLength      : focal length of virtual camera
		// - nSuperSamples  : Number of Samples !per pixel!
		//                      Runtime basically explodes if you turn this too high
		// - smoothShading    : switch to turn on/off smooth shading
		// - ambientOcclusion : switch to turn on/off ambient occlusion
		// - aoMaxDistance    : Ambient Occlusion distance
		//                      Should be about 10% of the max scene dimension
		// - aoNumSamples     : Number of samples for each ambient occlusion
		//                      evaluation
		enum Method { AO_METHOD_PERFECT, AO_METHOD_RANDOM };
		struct Options {
			unsigned int width;
			unsigned int height;
			float focalLength;
			unsigned int nSuperSamples;
			bool shading;
			bool ambientOcclusion;
			float aoMaxDistance;
			unsigned int aoNumSamples;
			Method aoMethod;
			int aoAlphaMin;
			int aoAlphaMax;
			BVH::Method bvhMethod;
		};
		RayTracer(Options _opts) :
			opts(_opts),
			totalWidth(_opts.width * (unsigned int) sqrt(_opts.nSuperSamples)),
			totalHeight(_opts.height * (unsigned int) sqrt(_opts.nSuperSamples)) {
		}
		void resize(float * tmp, unsigned char * image);
		Options opts;
		unsigned int totalWidth;
		unsigned int totalHeight;
};
