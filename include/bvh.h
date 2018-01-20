#pragma once
#include <vector>
#include "mesh.h"
#include "triangle.h"
#include "aabb.h"
// Bounding Volume Hierarchy (BVH) Interface.
class BVH {
	public:
		enum class Method { CUT_LONGEST_AXIS, SURFACE_AREA_HEURISTIC };
		BVH();
		BVH(BVH::Method method);
		~BVH();
		// Constructs the BVH from the given mesh.
		void buildBVH(const Mesh &mesh);
		unsigned int build(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::size_t &ind);
		std::vector<uint32_t> triangles;
		std::vector<uint32_t> nodes;
		std::vector<Vec3f> aabbs;
	private:
		void cutFaces(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesLongestAxis(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesMedianCut(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesSAH(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		BVH::Method method;
};
inline BVH::BVH() : method(BVH::Method::CUT_LONGEST_AXIS) {}
inline BVH::BVH(Method method) : method(method) {}
inline BVH::~BVH() {}