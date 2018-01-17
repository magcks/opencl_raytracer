#include "triangle.h"

Vec3f Triangle::getCentroid() const {
	return ((*this)[0] + (*this)[1] + (*this)[2]) / 3.0f;
}

Vec3f Triangle::getAABBMin() const {
	Vec3f min;
	min[0] = std::min((*this)[0][0], std::min((*this)[1][0], (*this)[2][0]));
	min[1] = std::min((*this)[0][1], std::min((*this)[1][1], (*this)[2][1]));
	min[2] = std::min((*this)[0][2], std::min((*this)[1][2], (*this)[2][2]));
	return min;
}

Vec3f Triangle::getAABBMax() const {
	Vec3f max;
	max[0] = std::max((*this)[0][0], std::max((*this)[1][0], (*this)[2][0]));
	max[1] = std::max((*this)[0][1], std::max((*this)[1][1], (*this)[2][1]));
	max[2] = std::max((*this)[0][2], std::max((*this)[1][2], (*this)[2][2]));
	return max;
}

AABB Triangle::getAABB() const {
	return AABB(this->getAABBMin(), this->getAABBMax());
}

Vec3f Triangle::getNormalVector() const {
	Vec3f n = ((*this)[1] - (*this)[0]).cross((*this)[2] - (*this)[0]);
	return n / n.length();
}

unsigned int Triangle::getFaceID() const {
	return this->faceID;
}