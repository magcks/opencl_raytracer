#pragma once
#include <limits>
#include "vec3.h"
// Axis-aligned bounding box.
class AABB {
	public:
		// Creates an inverted bounding box such that min/max operations work.
		AABB();
		// Creates and initializes a bounding box.
		AABB(const Vec3f &min, const Vec3f &max) : min(min), max(max) {}
		// Merge the AABB with another AABB.
		void merge(const AABB &aabb);
		// Merge the AABB with another vector.
		void merge(const Vec3f &vec);
		// Returns the longest axis, 0: x-axis, 1: y-axis, 2: z-axis.
		int getLongestAxis() const;
		// Returns true iff point is inside the AABB.
		bool inside(const Vec3f &point) const;
		Vec3f min;
		Vec3f max;
};
inline AABB::AABB()
	: min(Vec3f(std::numeric_limits<float>::max()))
	, max(Vec3f(-std::numeric_limits<float>::max())) {}