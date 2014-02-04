#ifndef AABB_H_
#define AABB_H_

#include <limits>

#include "vec3.h"
#include "ray.h"
#include "mesh.h"

#include <CL/cl.h>

// Axis-aligned bounding box.
class AABB {
	public:
		// Creates an inverted bounding box such that min/max operations work.
		AABB(void);
		// Creates and initializes a bounding box.
		AABB(Vec3f const &mi, Vec3f const &ma) : min(mi), max(ma) {}


		// Merge the AABB with another AABB.
		void merge(AABB const &aabb);
		// Merge the AABB with another vector.
		void merge(Vec3f const &vec);
		// Returns the longest axis, 0: x-axis, 1: y-axis, 2: z-axis.
		char getLongestAxis(void) const;
		// Returns true iff point is inside the AABB.
		bool inside(Vec3f const &point) const;

		Vec3f min;
		Vec3f max;
};

/* ------------------------ Implementation ------------------------ */

inline AABB::AABB(void)
	: min(Vec3f(std::numeric_limits<float>::max()))
	, max(Vec3f(-std::numeric_limits<float>::max())) {
}

#endif // AABB_H_
