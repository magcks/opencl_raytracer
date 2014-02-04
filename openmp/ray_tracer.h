#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <cmath>
#include <string>
#include <sstream>

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

	class Options {
	public:
		unsigned int width;
		unsigned int height;
		float focalLength;
		unsigned int nSuperSamples;
		bool shading;
		bool ambientOcclusion;
		float aoMaxDistance;
		unsigned int aoNumSamples;
		char aoMethod;
		int aoAlphaMin;
		int aoAlphaMax;

		std::string getMashineReadable() {
			std::stringstream ss;
			ss << "WIDTH " << this->width << std::endl;
			ss << "HEIGHT " << this->height << std::endl;
			ss << "FOCALLENGTH " << this->focalLength << std::endl;
			ss << "NSUPERSAMPLES " << this->nSuperSamples << std::endl;
			ss << "SHADING " << this->shading << std::endl;
			ss << "AO " << this->ambientOcclusion << std::endl;
			ss << "AO_MAXDISTANCE " << this->aoMaxDistance << std::endl;
			ss << "AO_NUMSAMPLES " << this->aoNumSamples << std::endl;
			ss << "AO_METHOD " << (unsigned int) this->aoMethod << std::endl;
			ss << "AO_ALPHA_MIN " << this->aoAlphaMin << std::endl;
			ss << "AO_ALPHA_MAX " << this->aoAlphaMax;

			return ss.str();
		}
	};
	static const char AO_METHOD_PERFECT = 0;
	static const char AO_METHOD_RANDOM = 1;

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

#endif // RAY_TRACER_H

