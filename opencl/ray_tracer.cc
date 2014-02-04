#include "ray_tracer.h"

void RayTracer::resize(cl_float * tmp, unsigned char * image) {
	unsigned int n(sqrt(this->opts.nSuperSamples));
	for (std::size_t y = 0; y < this->opts.height; ++y) {
		for (std::size_t x = 0; x < this->opts.width; ++x) {
			float total = 0;

			for (std::size_t ssY = 0; ssY < n; ++ssY) {
				for (std::size_t ssX = 0; ssX < n; ++ssX) {
					total += (float) tmp[((y * n + ssY) * this->totalWidth + (x * n + ssX))];
				}
			}

			image[y * this->opts.width + x] = (total / (n * n)) * 255;
		}
	}
}