#pragma once
#include <CL/cl.h>
#include "vec3.h"
#include "aabb.h"
#include "ray.h"
#include "mesh.h"
// Representation of a mesh/ray intersection.
//
// - mesh    : mesh that was intersected
// - faceID  : ID of the triangle that was intersected
// - bary    : barycentric coordinates of the intersection
// - position: The position of the intersection point
// - distance: The distance from the ray origin to the intersection point.
//     This field is also used to test whether an intersection happened.
//     In case of failure, it is set to 0.0f.
struct Intersection {
	Mesh const *mesh;
	unsigned int faceID;
	Vec3f bary;
	Vec3f position;
	float distance;
};
// Representation of a triangle which contains an intersection test
// and a few convenience functions.
#pragma pack(push, 1)
class Triangle {
	public:
		Triangle(void);
		Triangle(Mesh const *mesh, unsigned int faceID);
		Vec3f getCentroid(void) const;
		Vec3f getNormalVector(void) const;
		Vec3f getAABBMin(void) const;
		Vec3f getAABBMax(void) const;
		AABB getAABB(void) const;
		cl_uint getFaceID() const;
		Vec3f const &operator[](int index) const;
		Vec3f &operator[](int index);
	private:
		Mesh const *mesh;
		unsigned int faceID;
};
inline Triangle::Triangle(void)
	: mesh(NULL) {}
inline Triangle::Triangle(Mesh const *mesh, unsigned int faceID)
	: mesh(mesh), faceID(faceID) {}
inline Vec3f const &Triangle::operator[](int index) const {
	return this->mesh->vertices[this->mesh->faces[this->faceID * 3 + index]];
}
#pragma pack(pop)