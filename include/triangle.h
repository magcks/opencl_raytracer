#pragma once
#include "vec3.h"
#include "aabb.h"
#include "mesh.h"
// Representation of a triangle which contains an intersection test
// and a few convenience functions.
#pragma pack(push, 1)
class Triangle {
	public:
		Triangle();
		Triangle(const Mesh *mesh, unsigned int faceID);
		Vec3f getCentroid() const;
		Vec3f getNormalVector() const;
		Vec3f getAABBMin() const;
		Vec3f getAABBMax() const;
		AABB getAABB() const;
		uint32_t getFaceID() const;
		Vec3f const &operator[](int index) const;
		Vec3f &operator[](int index);
	private:
		const Mesh *mesh;
		unsigned int faceID;
};
inline Triangle::Triangle()
	: mesh(NULL) {}
inline Triangle::Triangle(const Mesh *mesh, unsigned int faceID)
	: mesh(mesh), faceID(faceID) {}
inline const Vec3f &Triangle::operator[](int index) const {
	return mesh->vertices[mesh->faces[faceID * 3 + index]];
}
#pragma pack(pop)