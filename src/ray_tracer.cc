#include "ray_tracer.h"
void RayTracer::resize(float *tmp, unsigned char *image) {
	unsigned int n(sqrt(options.nSuperSamples));
	for (std::size_t y = 0; y < options.height; ++y) {
		for (std::size_t x = 0; x < options.width; ++x) {
			float total = 0;
			for (std::size_t ssY = 0; ssY < n; ++ssY) {
				for (std::size_t ssX = 0; ssX < n; ++ssX) {
					total += (float) tmp[((y * n + ssY) * totalWidth + (x * n + ssX))];
				}
			}
			image[y * options.width + x] = (total / (n * n)) * 255;
		}
	}
}