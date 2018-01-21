#include "aabb.h"
void AABB::merge(const AABB &bb) {
	for (int i = 0; i < 3; ++i) {
		min[i] = std::min(min[i], bb.min[i]);
		max[i] = std::max(max[i], bb.max[i]);
	}
}
void AABB::merge(const Vec3f &vec) {
	for (int i = 0; i < 3; ++i) {
		min[i] = std::min(min[i], vec[i]);
		max[i] = std::max(max[i], vec[i]);
	}
}
char AABB::getLongestAxis() const {
	Vec3f diff = max - min;
	if (diff[0] >= diff[1] && diff[0] >= diff[2]) {
		return 0;
	}
	if (diff[1] >= diff[0] && diff[1] >= diff[2]) {
		return 1;
	}
	return 2;
}
bool AABB::inside(const Vec3f &point) const {
	for (int i = 0; i < 3; ++i) {
		if (point[i] > max[i] || point[i] < min[i]) {
			return false;
		}
	}
	return true;
}