#include "aabb.h"
#include <iostream>

void AABB::merge(AABB const &bb) {
	for (int i = 0; i < 3; ++i) {
		this->min[i] = std::min(this->min[i], bb.min[i]);
		this->max[i] = std::max(this->max[i], bb.max[i]);
	}
}

void AABB::merge(Vec3f const &vec) {
	for (int i = 0; i < 3; ++i) {
		this->min[i] = std::min(this->min[i], vec[i]);
		this->max[i] = std::max(this->max[i], vec[i]);
	}
}

char AABB::getLongestAxis() const {
	Vec3f diff = this->max - this->min;

	if (diff[0] >= diff[1] && diff[0] >= diff[2])
		return 0;

	if (diff[1] >= diff[0] && diff[1] >= diff[2])
		return 1;

	return 2;
}

bool AABB::inside(Vec3f const &point) const {
	for (int i = 0; i < 3; ++i)
		if (point[i] > this->max[i] || point[i] < this->min[i])
			return false;

	return true;
}