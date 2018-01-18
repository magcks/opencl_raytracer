#include "ray_tracer.h"
void RayTracer::resize(float *tmp, unsigned char *image) {
	unsigned int n(sqrt(this->options.nSuperSamples));
	for (std::size_t y = 0; y < this->options.height; ++y) {
		for (std::size_t x = 0; x < this->options.width; ++x) {
			float total = 0;
			for (std::size_t ssY = 0; ssY < n; ++ssY) {
				for (std::size_t ssX = 0; ssX < n; ++ssX) {
					total += (float) tmp[((y * n + ssY) * this->totalWidth + (x * n + ssX))];
				}
			}
			image[y * this->options.width + x] = (total / (n * n)) * 255;
		}
	}
}