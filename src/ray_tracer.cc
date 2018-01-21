#include "ray_tracer.h"
void RayTracer::resize(float *tmp, unsigned char *image) {
	unsigned int n(sqrt(options.nSuperSamples));
	for (auto y = 0u; y < options.height; ++y) {
		for (auto x = 0u; x < options.width; ++x) {
			float total = 0;
			for (auto ssY = 0u; ssY < n; ++ssY) {
				for (auto ssX = 0u; ssX < n; ++ssX) {
					total += (float) tmp[((y * n + ssY) * totalWidth + (x * n + ssX))];
				}
			}
			image[y * options.width + x] = (total / (n * n)) * 255;
		}
	}
}